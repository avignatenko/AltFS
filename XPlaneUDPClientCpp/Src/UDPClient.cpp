#include "StdAfx.h"

#include "ObjectPool.h"

#include "../Include/XPlaneUDPClientCpp/UDPClient.h"
#include "../Include/XPlaneUDPClientCpp/Utils.h"

#include <array>
#include <deque>

#include <spdlog/spdlog.h>
#include <asio/error.hpp>
#include <continuable/external/asio.hpp>

using namespace asio;
using namespace asio::ip;

namespace xplaneudpcpp
{

class ClientSender
{
public:
    ClientSender(udp::socket& socket, const std::string& addressRemote, int portRemote)
        : endpointRemote_(udp::endpoint(address::from_string(addressRemote), portRemote)), socket_(socket)
    {
        spdlog::info("X-Plane UDP Sender client init");
    }

#pragma pack(push, r1, 1)
    struct SendFloatDataref
    {
        char id[5];
        float value;
        char refName[500];
    };
#pragma pack(pop, r1)

    cti::continuable<> writeDataref(const std::string& dataref, float f)
    {
        spdlog::debug("X-Plane UDP Client send dataref {}  {}", dataref, f);

        auto data = std::make_shared<SendFloatDataref>();

        strcpy(data->id, "DREF");
        data->value = f;
        strcpy(data->refName, dataref.c_str());
        std::memset(data->refName + dataref.length() + 1, ' ', 500 - dataref.length() - 1);

        return socket_
            .async_send_to(buffer(data.get(), sizeof(SendFloatDataref)), endpointRemote_, cti::use_continuable)
            .then([data](std::size_t written) {});
    }

#pragma pack(push, r1, 1)
    struct SubscribeDataref
    {
        char id[5];
        int freq;
        int num;
        char refName[400];
    };
#pragma pack(pop, r1)

    cti::continuable<> subscribeDataref(const std::string& dataref, int freq, int num)
    {
        spdlog::debug("X-Plane UDP Sender subscribe dataref {} with freq {} and num {}", dataref, freq, num);

        auto data = std::make_shared<SubscribeDataref>();
        strcpy(data->id, "RREF");
        data->freq = freq;
        data->num = num;

        strcpy(data->refName, dataref.c_str());
        std::memset(data->refName + dataref.length() + 1, 0, 400 - dataref.length() - 1);

        return socket_
            .async_send_to(buffer(data.get(), sizeof(SubscribeDataref)), endpointRemote_, cti::use_continuable)
            .then([data](std::size_t bytes_transferred) { spdlog::debug("Subscribed dataref {}", data->num); });
    }

    cti::continuable<> unsubscribeDataref(int num, const std::string& dataref)
    {
        spdlog::debug("X-Plane UDP Sender unsubscribe dataref {} ", num);

        auto data = std::make_shared<SubscribeDataref>();
        strcpy(data->id, "RREF");
        data->freq = 0;
        data->num = num;

        strcpy(data->refName, dataref.c_str());
        std::memset(data->refName + dataref.length() + 1, 0, 400 - dataref.length() - 1);

        return socket_
            .async_send_to(buffer(data.get(), sizeof(SubscribeDataref)), endpointRemote_, cti::use_continuable)
            .then([data](std::size_t bytes_transferred) { spdlog::debug("Unsubscribed dataref {}", data->num); });
    }

private:
    ip::udp::endpoint endpointRemote_;
    udp::socket& socket_;
};

class ClientReceiver : public std::enable_shared_from_this<ClientReceiver>
{
public:
    ClientReceiver(asio::any_io_executor exCallback, udp::socket& socket, std::shared_ptr<ClientSender> sender)
        : exCallback_(exCallback), socket_(socket), sender_(sender)
    {
        spdlog::info("X-Plane UDP Receiver client init");
    }

    void start() { doReceive(); }

    unsigned int hash(const char* str)
    {
        unsigned int h;
        unsigned char* p;

        const int MULTIPLIER = 37;
        h = 0;
        for (p = (unsigned char*)str; *p != '\0'; p++) h = MULTIPLIER * h + *p;
        return h;  // or, h % ARRAY_SIZE;
    }

    int getDatarefNum(const std::string& dataref)
    {
        int hashed = hash(dataref.c_str());
        return hashed;
    }

    cti::continuable<> subscribeDataref(const std::string& dataref, int freq, std::function<void(float)> callback)
    {
        spdlog::debug("X-Plane UDP Receiver client subscribe dataref {}", dataref);

        int num = getDatarefNum(dataref);

        return sender_->unsubscribeDataref(num, dataref)
            .then(
                [self = weak_from_this(), dataref, freq, callback, num]() -> cti::continuable<>
                {
                    if (self.expired()) return cti::make_ready_continuable<>();
                    auto lockedSelf = self.lock();
                    return lockedSelf->sender_->subscribeDataref(dataref, freq, num);
                })
            .then(
                [self = weak_from_this(), callback, num, dataref]
                {
                    if (self.expired()) return;
                    auto lockedSelf = self.lock();
                    lockedSelf->datarefs_[num].first = dataref;
                    lockedSelf->datarefs_[num].second = callback;
                });
    }

    cti::continuable<> unsubscribeDataref(const std::string& dataref)
    {
        spdlog::debug("X-Plane UDP Receiver client unsubscribe dataref {}", dataref);

        size_t num = getDatarefNum(dataref);
        // send unsubscribe
        return sender_->unsubscribeDataref(num, dataref)
            .then(
                [self = weak_from_this(), num]
                {
                    if (self.expired()) return;
                    auto lockedSelf = self.lock();
                    lockedSelf->datarefs_.erase(num);
                });
    }

    cti::continuable<> unsubscribeAll()
    {
        spdlog::debug("X-Plane UDP Receiver client unsubscribe all");
        std::vector<cti::continuable<>> ops;
        for (const auto& [key, value] : datarefs_)
        {
            // send unsubscribe
            ops.push_back(sender_->unsubscribeDataref(key, value.first));
        }

        return cti::when_all(std::move(ops))
            .then(
                [self = weak_from_this()]
                {
                    if (self.expired()) return;
                    auto lockedSelf = self.lock();
                    lockedSelf->datarefs_.clear();
                });
    }

private:
#pragma pack(push, r1, 1)
    struct ReceiveDataref
    {
        char id[5];

        struct Value
        {
            int32_t num;
            float value;
        };

        Value value[100];
    };
#pragma pack(pop, r1)

    cti::continuable<std::size_t, std::shared_ptr<ReceiveDataref>> receive()
    {
        auto data = receiveObjectsPool_.aquire();
        return socket_
            .async_receive_from(asio::buffer(data.get(), sizeof(ReceiveDataref)), senderEndpoint_, cti::use_continuable)
            .then([data](std::size_t reply_length) { return std::make_tuple(reply_length, data); });
    }

    void doReceive()
    {
        receive().then(
            [self = weak_from_this()](std::size_t reply_length, std::shared_ptr<ReceiveDataref> data)
            {
                if (self.expired()) return;
                auto lockedSelf = self.lock();

                if (reply_length < 5) return lockedSelf->doReceive();

                int port = lockedSelf->senderEndpoint_.port();
                std::string addr = lockedSelf->senderEndpoint_.address().to_string();

                // dispatch

                const int numberOfValues = (reply_length - 5) / sizeof(ReceiveDataref::Value);
                for (int i = 0; i < numberOfValues; ++i)
                {
                    const ReceiveDataref::Value& valueData = data->value[i];

                    auto datarefIter = lockedSelf->datarefs_.find(valueData.num);
                    if (datarefIter == lockedSelf->datarefs_.end())
                    {
                        // not our message (fixme, how to unsubscribe?)
                        spdlog::debug("Not our message: {} with value {}", valueData.num, valueData.value);
                        continue;
                    }

                    spdlog::debug("Received: {} with value {}", valueData.num, valueData.value);

                    const auto& datarefCallback = datarefIter->second;
                    if (datarefCallback.second)  // subscribed?
                        datarefCallback.second(valueData.value);
                }

                lockedSelf->receiveObjectsPool_.release(data);
                lockedSelf->doReceive();
            });
    }

private:
    asio::any_io_executor exCallback_;
    udp::socket& socket_;
    std::unordered_map<int, std::pair<std::string, std::function<void(float)>>> datarefs_;
    std::shared_ptr<ClientSender> sender_;
    udp::endpoint senderEndpoint_;
    ObjectPool<ReceiveDataref> receiveObjectsPool_;
};

xplaneudpcpp::UDPClient::UDPClient(asio::io_context& ex, const std::string& address, int port, int localPort)
    : socket_(ex), ex_(ex)
{
    spdlog::info("X-Plane UDP Client created with address {}:{}", address, port);
    spdlog::info("X-Plane UDP Client local port is {}", localPort);

    udp::endpoint sendEndpoint(udp::v4(), localPort);

    socket_.open(sendEndpoint.protocol());
    socket_.bind(sendEndpoint);

    m_clientSender = std::make_shared<ClientSender>(socket_, address, port);
    m_clientReceiver = std::make_shared<ClientReceiver>(ex.get_executor(), socket_, m_clientSender);

    m_clientReceiver->start();
}

xplaneudpcpp::UDPClient::~UDPClient()
{
    spdlog::info("X-Plane UDP Client shutdown attempt...");

    unsubscribeAll().apply(waitOnContext(ex_));

    m_clientReceiver.reset();
    m_clientSender.reset();

    spdlog::info("X-Plane UDP Client shutdown successful");
}

cti::continuable<> xplaneudpcpp::UDPClient::unsubscribeAll()
{
    return m_clientReceiver->unsubscribeAll();
}

cti::continuable<> UDPClient::connect()
{
    return cti::make_ready_continuable();
}

cti::continuable<> xplaneudpcpp::UDPClient::writeDataref(const std::string& dataref, float f)
{
    return m_clientSender->writeDataref(dataref, f);
}

cti::continuable<> xplaneudpcpp::UDPClient::subscribeDataref(const std::string& dataref, int freq,
                                                             std::function<void(float)> callback)
{
    return m_clientReceiver->subscribeDataref(dataref, freq, callback);
}

cti::continuable<> UDPClient::unsubscribeDataref(const std::string& dataref)
{
    return m_clientReceiver->unsubscribeDataref(dataref);
}

}  // namespace xplaneudpcpp
