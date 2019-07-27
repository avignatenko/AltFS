#include "stdafx.h"

#include "UDPClient.h"
#include "ActiveObject.h"

#include <array>

using namespace boost::asio;
using namespace boost::asio::ip;

namespace xplaneudpcpp
{

class ClientSender : public ActiveObject
{
public:

    ClientSender(io_service& io, int portThis, const std::string& addressRemote, int portRemote)
        : endpointRemote_(udp::endpoint(address::from_string(addressRemote), portRemote))
        , socket_(io, udp::endpoint(udp::v4(), portThis))
    {
    }

    void writeDataref(const std::string& dataref, float f)
    {
        m_dispatchQueue.put([this]() {
            std::array<int, 500> message;
            socket_.send_to(buffer(message), endpointRemote_);
        });
    }

private:
    udp::socket socket_;
    ip::udp::endpoint endpointRemote_;
};

class ClientReceiver : public ActiveObject
{
public:
    ClientReceiver(io_service& io, int portThis)
        : socket_(io, udp::endpoint(udp::v4(), portThis))
    {
        doReceive();
    }

     void subscribeDataref(const std::string& dataref, double freq, std::function<void(float)> callback)
     {
     }

private:

    void doReceive()
    {
        m_dispatchQueue.put([this]
        {
            try{
                std::array<int, 500> message;
                udp::endpoint senderEndpoint;
                size_t reply_length = socket_.receive_from(boost::asio::buffer(message), senderEndpoint);

                // ... process here
                boost::asio::error::interrupted;
                doReceive();
            }
            catch (boost::system::system_error& error)
            {
             
            }
        });
    }

private:

    ip::udp::socket socket_;
};

xplaneudpcpp::UDPClient::UDPClient(const std::string& address, int port)
{
    
    m_clientReceiver = std::make_unique<ClientReceiver>(io_, 50000);
    m_clientSender = std::make_unique<ClientSender>(io_, 50001, address, port);
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

void xplaneudpcpp::UDPClient::subscribeDataref(const std::string & dataref, double freq, std::function<void(float)> callback)
{
    m_clientReceiver->subscribeDataref(dataref, freq, callback);
}

}