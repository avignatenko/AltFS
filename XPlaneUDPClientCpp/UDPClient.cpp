#include "stdafx.h"

#include "UDPClient.h"

#include <array>

using boost::asio::ip::udp;

xplaneudpcpp::UDPClient::UDPClient(const std::string& address, int port)
    : m_xplaneDestination(
        boost::asio::ip::address::from_string(address), 
        port)
    , socket_(io_service, udp::endpoint(udp::v4(), 12345))
{
    m_thread = std::make_unique<std::thread>([this]()
    {
       
        udp::endpoint remote_endpoint_;
        std::array<char, 500> recv_buffer_;

        socket_.async_receive_from(
            boost::asio::buffer(recv_buffer_),
            remote_endpoint_,
            [this](boost::system::error_code ec, std::size_t bytes_recvd)
        {
            if (!ec && bytes_recvd > 0)
            {
                //do_send(bytes_recvd);
            }
            else
            {
                //do_receive();
            }
        });

        io_service.run();
    });

}

xplaneudpcpp::UDPClient::~UDPClient()
{
    io_service.stop();
    m_thread->join();

}

void xplaneudpcpp::UDPClient::writeDataref(const std::string & dataref, float f)
{
    socket_.async_send_to(boost::asio::buffer(dataref), m_xplaneDestination,
                          [this](boost::system::error_code /*ec*/, std::size_t /*bytes_sent*/)
    {
        //do_receive();
    });
}

void xplaneudpcpp::UDPClient::subscribeDataref(const std::string & dataref, double freq, std::function<void(float)> callback) 
{
}
