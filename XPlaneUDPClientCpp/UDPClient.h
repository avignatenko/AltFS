#pragma once

#include "promise.hpp"

#include <boost/asio.hpp>

#include <string>
#include <functional>
#include <thread>
#include <array>


namespace xplaneudpcpp
{

 class ClientReceiver;
   class ClientSender;

class UDPClient
{
public:

    UDPClient(const std::string& address, int port);
    ~UDPClient();

    promise::Defer connect();

    void writeDataref(const std::string& dataref, float f);
    
    void subscribeDataref(const std::string& dataref, int freq, std::function<void(float)> callback);
    void unsubscribeDataref(const std::string& dataref);

private:

   
   boost::asio::io_service io_;
    
   std::unique_ptr<ClientSender> m_clientSender;
   std::unique_ptr<ClientReceiver> m_clientReceiver;
   std::unique_ptr<std::thread> m_thread;
};
}