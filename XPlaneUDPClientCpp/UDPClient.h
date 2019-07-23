#pragma once

#include <boost/asio.hpp>

#include <string>
#include <functional>
#include <thread>

namespace xplaneudpcpp
{
class UDPClient
{
public:

    UDPClient(const std::string& address, int port);
    ~UDPClient();

    void writeDataref(const std::string& dataref, float f);
    void subscribeDataref(const std::string& dataref, double freq, std::function<void(float)> callback);

private:

  
    boost::asio::ip::udp::endpoint m_xplaneDestination;
    boost::asio::io_service io_service;
    boost::asio::ip::udp::socket socket_;
    std::unique_ptr<std::thread> m_thread;

};
}