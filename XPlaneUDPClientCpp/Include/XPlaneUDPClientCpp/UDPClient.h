#pragma once

#include <promise-cpp/promise.hpp>

#include <asio.hpp>

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

    UDPClient(const std::string& address, int port, int16_t baseId);
    ~UDPClient();

    promise::Defer connect();

    void writeDataref(const std::string& dataref, float f);
    
    void subscribeDataref(const std::string& dataref, int freq, std::function<void(float)> callback);
    void unsubscribeDataref(const std::string& dataref);

private:

   
   asio::io_service io_;
    
   asio::ip::udp::socket socket_;
   std::unique_ptr<ClientSender> m_clientSender;
   std::unique_ptr<ClientReceiver> m_clientReceiver;
   std::unique_ptr<std::thread> m_thread;
};
}