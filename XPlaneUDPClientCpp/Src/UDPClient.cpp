#include "StdAfx.h"

#include "../Include/XPlaneUDPClientCpp/ActiveObject.h"
#include "../Include/XPlaneUDPClientCpp/UDPClient.h"

#include <promise-cpp/asio/timer.hpp>

#include <array>
#include <deque>

#include <spdlog/spdlog.h>

#include <asio/error.hpp>

using namespace asio;
using namespace asio::ip;

namespace promise
{

template <typename RESULT>
inline void setPromise(Defer d, std::error_code err, const char* errorString, const RESULT& result)
{
    if (err)
    {
        spdlog::error("{}: {}", errorString, err.message());
        d.reject(err);
    }
    else
        d.resolve(result);
}

}  // namespace promise

namespace xplaneudpcpp
{

class Client
{
public:
    Client(io_service& io, udp::socket& socket) : socket_(socket), io_(io) {}

    void stop()
    {
        asio::dispatch(io_,
                       [this]
                       {
                           m_stopped = true;
                           socket_.close();
                       });
    }

protected:
    bool m_stopped = false;
    udp::socket& socket_;
    io_service& io_;
};

class ClientSender : public Client
{
public:
    ClientSender(io_service& io, udp::socket& socket, const std::string& addressRemote, int portRemote)
        : endpointRemote_(udp::endpoint(address::from_string(addressRemote), portRemote)), Client(io, socket)
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

    promise::Defer writeDataref(const std::string& dataref, float f)
    {
        return promise::newPromise(
            [&](promise::Defer& d)
            {
                io_.dispatch(
                    [=]
                    {
                        spdlog::debug("X-Plane UDP Client send dataref {}  {}", dataref, f);

                        SendFloatDataref data;
                        strcpy(data.id, "DREF");
                        data.value = f;
                        strcpy(data.refName, dataref.c_str());
                        std::memset(data.refName + dataref.length() + 1, ' ', 500 - dataref.length() - 1);
                        socket_.async_send_to(buffer(&data, sizeof(data)), endpointRemote_,
                                              [d](const std::error_code& error, std::size_t bytes_transferred)
                                              { promise::setPromise(d, error, "writeDataref", bytes_transferred); });
                    });
            });
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

    promise::Defer subscribeDataref(const std::string& dataref, int freq, int num)
    {
        return promise::newPromise(
            [&](promise::Defer d)
            {
                io_.dispatch(
                    [=]
                    {
                        spdlog::debug("X-Plane UDP Sender subscribe dataref {} with freq {} and num {}", dataref, freq,
                                      num);

                        SubscribeDataref data;
                        strcpy(data.id, "RREF");
                        data.freq = freq;
                        data.num = num;

                        strcpy(data.refName, dataref.c_str());
                        std::memset(data.refName + dataref.length() + 1, 0, 400 - dataref.length() - 1);

                        socket_.async_send_to(
                            buffer(&data, sizeof(data)), endpointRemote_,
                            [d](const std::error_code& error, std::size_t bytes_transferred)
                            { promise::setPromise(d, error, "subscribeDataref", bytes_transferred); });
                    });
            });
    }

    promise::Defer unsubscribeDataref(int num, const std::string& dataref)
    {
        return promise::newPromise(
            [=](promise::Defer d)
            {
                io_.dispatch(
                    [=]
                    {
                        spdlog::debug("X-Plane UDP Sender unsubscribe dataref {} ", num);

                        SubscribeDataref data;
                        strcpy(data.id, "RREF");
                        data.freq = 0;
                        data.num = num;

                        strcpy(data.refName, dataref.c_str());
                        std::memset(data.refName + dataref.length() + 1, 0, 400 - dataref.length() - 1);

                        socket_.async_send_to(
                            buffer(&data, sizeof(data)), endpointRemote_,
                            [d](const std::error_code& error, std::size_t bytes_transferred)
                            { promise::setPromise(d, error, "unsubscribeDataref", bytes_transferred); });
                    });
            });
    }

private:
    ip::udp::endpoint endpointRemote_;
};

class ClientReceiver : public Client
{
public:
    ClientReceiver(io_service& io, udp::socket& socket, ClientSender& sender, int16_t baseId)
        : Client(io, socket), sender_(sender), baseId_(baseId)
    {
        spdlog::info("X-Plane UDP Receiver client init");

        doReceive();
    }

    promise::Defer getDatarefNum(const std::string& dataref)
    {
        return promise::newPromise(
            [=](promise::Defer d)
            {
                io_.dispatch(
                    [=]
                    {
                        auto found = std::find_if(datarefs_.begin(), datarefs_.end(),
                                                  [dataref](const auto& elem) { return elem.first == dataref; });
                        if (found == datarefs_.end())
                        {
                            datarefs_.push_back({dataref, nullptr});
                            d.resolve(datarefs_.size() - 1);
                            return;
                        }

                        d.resolve(found - datarefs_.begin());
                        return;
                    });
            });
    }

    void subscribeDataref(const std::string& dataref, int freq, std::function<void(float)> callback)
    {
        spdlog::debug("X-Plane UDP Receiver client subscribe dataref {}", dataref);

        getDatarefNum(dataref).then(
            [=](size_t num)
            {
                sender_.subscribeDataref(dataref, freq, num + baseId_);
                datarefs_[num].second = callback;
            });
    }

    void unsubscribeDataref(const std::string& dataref)
    {
        spdlog::debug("X-Plane UDP Receiver client unsubscribe dataref {}", dataref);

        getDatarefNum(dataref).then(
            [=](int num)
            {
                // send unsubscribe
                sender_.unsubscribeDataref(num + baseId_, dataref);
                datarefs_[num].second = nullptr;
            });
    }

    promise::Defer unsubscribeAll()
    {
        return promise::newPromise(
            [this](promise::Defer d)
            {
                spdlog::debug("X-Plane UDP Receiver client unsubscribe all");

                io_.dispatch(
                    [this, d]
                    {
                        for (size_t i = 0; i < datarefs_.size(); ++i)
                        {
                            // send unsubscribe
                            sender_.unsubscribeDataref(i, datarefs_[i].first);
                        }

                        datarefs_.clear();
                        d.resolve();
                    });
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

    promise::Defer receive()
    {
        return promise::newPromise(
            [this](promise::Defer d)
            {
                socket_.async_receive_from(asio::buffer(message), senderEndpoint,
                                           [=](const std::error_code& error, std::size_t reply_length)
                                           { promise::setPromise(d, error, "doReceive", reply_length); });
            });
    }

    void doReceive()
    {
        receive().then(
            [this](size_t reply_length)
            {
                if (reply_length < 5) return doReceive();

                int port = senderEndpoint.port();
                std::string addr = senderEndpoint.address().to_string();

                // dispatch

                const int numberOfValues = (reply_length - 5) / sizeof(ReceiveDataref::Value);
                for (int i = 0; i < numberOfValues; ++i)
                {
                    const ReceiveDataref::Value& valueData = message->value[i];

                    int correctedNum = valueData.num - baseId_;

                    if (correctedNum < 0 || correctedNum >= (int)datarefs_.size())
                    {
                        // not our message (fixme, how to unsubscribe?)
                        continue;
                    }

                    auto& dataref = datarefs_[correctedNum];

                    if (dataref.second)  // subscribed?
                        dataref.second(valueData.value);
                }

                if (!m_stopped) doReceive();
            });
    }

private:
    std::deque<std::pair<std::string, std::function<void(float)>>> datarefs_;
    ClientSender& sender_;
    ReceiveDataref message[1];
    udp::endpoint senderEndpoint;
    int16_t baseId_;
};

xplaneudpcpp::UDPClient::UDPClient(asio::any_io_executor ex, const std::string& address, int port, int16_t baseId)
    : socket_(io_)
{
    spdlog::info("X-Plane UDP Client created with address {}:{}", address, port);
    const int localPort = 50000;
    spdlog::info("X-Plane UDP Client local port is {}", localPort);

    udp::endpoint sendEndpoint(udp::v4(), localPort);

    socket_.open(sendEndpoint.protocol());
    socket_.bind(sendEndpoint);

    m_clientSender = std::make_unique<ClientSender>(io_, socket_, address, port);
    m_clientReceiver = std::make_unique<ClientReceiver>(io_, socket_, *m_clientSender, baseId);

    m_thread = std::make_unique<std::thread>(
        [this]
        {
            io_.run();
            spdlog::info("UDPClient thread stopped");
        });
}

xplaneudpcpp::UDPClient::~UDPClient()
{
    spdlog::info("X-Plane UDP Client shutdown attempt...");

    m_clientReceiver->unsubscribeAll().then(
        [this]
        {
            m_clientReceiver->stop();
            m_clientSender->stop();
        });

    m_thread->join();

    m_clientReceiver.reset();
    m_clientSender.reset();

    spdlog::info("X-Plane UDP Client shutdown successful");
}

cti::continuable<> UDPClient::connect()
{
    return cti::make_ready_continuable();
}

void xplaneudpcpp::UDPClient::writeDataref(const std::string& dataref, float f)
{
    m_clientSender->writeDataref(dataref, f);
}

void xplaneudpcpp::UDPClient::subscribeDataref(const std::string& dataref, int freq,
                                               std::function<void(float)> callback)
{
    m_clientReceiver->subscribeDataref(dataref, freq, callback);
}

void UDPClient::unsubscribeDataref(const std::string& dataref)
{
    m_clientReceiver->unsubscribeDataref(dataref);
}

}  // namespace xplaneudpcpp
