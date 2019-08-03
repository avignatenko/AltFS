#include "stdafx.h"

#include "UDPClient.h"
#include "ActiveObject.h"

#include <array>
#include <deque>

#include <spdlog/spdlog.h>

#include <boost/asio/error.hpp>

using namespace boost::asio;
using namespace boost::asio::ip;

namespace xplaneudpcpp
{

class Client : public ActiveObject
{
public:

    Client(io_service& io, int portThis)
        : socket_(io)
    {

        udp::endpoint sendEndpoint(udp::v4(), portThis);

        socket_.open(sendEndpoint.protocol());
        socket_.set_option(boost::asio::ip::udp::socket::reuse_address(true));
        socket_.bind(sendEndpoint);

    }

protected:

    udp::socket socket_;

};

class ClientSender : public Client
{
public:

    ClientSender(io_service& io, int portThis, const std::string& addressRemote, int portRemote)
        : endpointRemote_(udp::endpoint(address::from_string(addressRemote), portRemote))
        , Client(io, portThis)
    {
        spdlog::info("X-Plane UDP Sender client init");
    }

#pragma pack (push, r1, 1)
    struct SendFloatDataref
    {
        char id[5];
        float value;
        char refName[500];
    };
#pragma pack (pop, r1)

    void writeDataref(const std::string& dataref, float f)
    {
        spdlog::debug("X-Plane UDP Client send dataref {}  {}", dataref, f);

        run([this, dataref, f]()
        {
            SendFloatDataref data;
            strcpy(data.id, "DREF");
            data.value = f;
            strcpy(data.refName, dataref.c_str());
            std::memset(data.refName + dataref.length() + 1, ' ', 500 - dataref.length() - 1);
            socket_.send_to(buffer(&data, sizeof(data)), endpointRemote_);
        });
    }

#pragma pack (push, r1, 1)
    struct SubscribeDataref
    {
        char id[5];
        int freq;
        int num;
        char refName[400];
    };
#pragma pack (pop, r1)


    void subscribeDataref(const std::string & dataref, int freq, int num)
    {
        spdlog::debug("X-Plane UDP Sender subscribe dataref {} with freq {} and num {}", dataref, freq, num);

        run([this, dataref, freq, num]()
        {
            SubscribeDataref data;
            strcpy(data.id, "RREF");
            data.freq = freq;
            data.num = num;

            strcpy(data.refName, dataref.c_str());
            std::memset(data.refName + dataref.length() + 1, 0, 400 - dataref.length() - 1);

            socket_.send_to(buffer(&data, sizeof(data)), endpointRemote_);
        });

    }

    void unsubscribeDataref(const std::string& dataref, int num)
    {
        spdlog::debug("X-Plane UDP Sender unsubscribe dataref {} ", dataref);

        run([this, dataref, num]()
        {
            SubscribeDataref data;
            strcpy(data.id, "RREF");
            data.freq = 0;
            data.num = num;

            strcpy(data.refName, dataref.c_str());
            std::memset(data.refName + dataref.length() + 1, 0, 400 - dataref.length() - 1);

            socket_.send_to(buffer(&data, sizeof(data)), endpointRemote_);
        });

    }
private:
    ip::udp::endpoint endpointRemote_;
};

class ClientReceiver : public Client
{
public:
    ClientReceiver(io_service& io, int portThis)
        : Client(io, portThis)
    {
        spdlog::info("X-Plane UDP Receiver client init");
        doReceive();
    }

    int getDatarefNum(const std::string& dataref) 
    { 
        std::lock_guard<std::mutex> lock(lock_);

        auto found = std::find_if(datarefs_.begin(), datarefs_.end(), [&dataref](const auto& elem)
        {
            return elem.first == dataref;
        });
        if (found == datarefs_.end())
        {
            datarefs_.push_back({dataref, nullptr});
            return datarefs_.size() - 1;
        }

        return found - datarefs_.begin();
    }

    void subscribeDataref(const std::string& dataref, std::function<void(float)> callback)
    {
         spdlog::debug("X-Plane UDP Receiver client subscribe dataref {}", dataref);

         int index = getDatarefNum(dataref);

         std::lock_guard<std::mutex> lock(lock_);
         datarefs_[index].second = callback;
    }

     void unsubscribeDataref(const std::string& dataref)
    {
         spdlog::debug("X-Plane UDP Receiver client unsubscribe dataref {}", dataref);

         int index = getDatarefNum(dataref);

         std::lock_guard<std::mutex> lock(lock_);
         datarefs_[index].second = nullptr;
    }
private:

    #pragma pack (push, r1, 1)
    struct ReceiveDataref
    {
        char id[5];

        struct Value
        {
           int num;
           float value;
        };
        
        Value value[100];

    };
#pragma pack (pop, r1)

    void doReceive()
    {
        run([this]
        {
            try
            {
                ReceiveDataref message[1];
                udp::endpoint senderEndpoint;

                size_t reply_length = socket_.receive_from(boost::asio::buffer(message), senderEndpoint);
                if (reply_length < 5) doReceive();

                int port = senderEndpoint.port();
                std::string addr = senderEndpoint.address().to_string();

                // dispatch

                std::lock_guard<std::mutex> lock(lock_); // fixme: how to avoid/

                const int numberOfValues = (reply_length - 5) / sizeof(ReceiveDataref::Value);
                for (int i = 0; i < numberOfValues; ++i)
                {
                    const ReceiveDataref::Value& valueData = message->value[i];
                    if (valueData.num < 0 || valueData.num >= (int)datarefs_.size()) continue; // fixme: what's that?                   
                    auto& dataref = datarefs_[valueData.num];
                    if (dataref.second) // subscribed?
                       dataref.second(valueData.value);
                }

                doReceive();
            }
            catch (boost::system::system_error& error)
            {
                if (error.code() == boost::asio::error::interrupted)
                    return; // just normal termination

                throw error;
            }
        });
    }

private:

    std::mutex lock_;
    std::deque<std::pair<std::string, std::function<void(float)>>> datarefs_;
};

xplaneudpcpp::UDPClient::UDPClient(const std::string& address, int port)
{
    spdlog::info("X-Plane UDP Client created with address {}:{}", address, port);
    const int localPort = 50000;
    spdlog::info("X-Plane UDP Client local port is {}", localPort);

    m_clientReceiver = std::make_unique<ClientReceiver>(io_, localPort);
    m_clientSender = std::make_unique<ClientSender>(io_, localPort, address, port);
}

xplaneudpcpp::UDPClient::~UDPClient()
{
    spdlog::info("X-Plane UDP Client shutdown attempt...");

    io_.stop();

    m_clientReceiver.reset();
    m_clientSender.reset();

    spdlog::info("X-Plane UDP Client shutdown successful");

}

void xplaneudpcpp::UDPClient::writeDataref(const std::string & dataref, float f)
{
    m_clientSender->writeDataref(dataref, f);
}

int UDPClient::getDatarefNum(const std::string & dataref)
{
     return m_clientReceiver->getDatarefNum(dataref);
}

void xplaneudpcpp::UDPClient::subscribeDataref(const std::string & dataref, int freq, std::function<void(float)> callback)
{

    m_clientReceiver->subscribeDataref(dataref, callback);
    m_clientSender->subscribeDataref(dataref, freq, m_clientReceiver->getDatarefNum(dataref));
}

void UDPClient::unsubscribeDataref(const std::string & dataref)
{
    m_clientSender->unsubscribeDataref(dataref, m_clientReceiver->getDatarefNum(dataref));
    m_clientReceiver->unsubscribeDataref(dataref);
}

}