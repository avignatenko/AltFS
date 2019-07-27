#include "stdafx.h"

#include "UDPClient.h"
#include "ActiveObject.h"

#include <array>
#include <deque>

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
        m_dispatchQueue.put([this, dataref, f]()
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
        m_dispatchQueue.put([this, dataref, freq, num]()
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

private:
    ip::udp::endpoint endpointRemote_;
};

class ClientReceiver : public Client
{
public:
    ClientReceiver(io_service& io, int portThis)
        : Client(io, portThis)
    {
        doReceive();
    }

    int subscribeDataref(const std::string& dataref, std::function<void(float)> callback)
    {
         std::lock_guard<std::mutex> lock(lock_);
         datarefs_.push_back(callback);
         return datarefs_.size() - 1;

        /*
        std::promise<int> retval;
        m_dispatchQueue.put([this, &retval, &dataref, &callback]
        {
            datarefs_.push_back(callback);
            retval.set_value(datarefs_.size() - 1);
        });
        
        return retval.get_future().get();*/
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
        m_dispatchQueue.put([this]
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
                    auto& callback = datarefs_[valueData.num];
                    callback(valueData.value);
                }

                doReceive();
            }
            catch (boost::system::system_error& error)
            {
                throw error;
            }
        });
    }

private:

    std::mutex lock_;
    std::deque<std::function<void(float)>> datarefs_;
};

xplaneudpcpp::UDPClient::UDPClient(const std::string& address, int port)
{

    m_clientReceiver = std::make_unique<ClientReceiver>(io_, 50000);
    m_clientSender = std::make_unique<ClientSender>(io_, 50000, address, port);
}

xplaneudpcpp::UDPClient::~UDPClient()
{
    io_.stop();

    m_clientReceiver.reset();
    m_clientSender.reset();

}

void xplaneudpcpp::UDPClient::writeDataref(const std::string & dataref, float f)
{
    m_clientSender->writeDataref(dataref, f);
}

void xplaneudpcpp::UDPClient::subscribeDataref(const std::string & dataref, int freq, std::function<void(float)> callback)
{
    int num = m_clientReceiver->subscribeDataref(dataref, callback);
    m_clientSender->subscribeDataref(dataref, freq, num);
}

}