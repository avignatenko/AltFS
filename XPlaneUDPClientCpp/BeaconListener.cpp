#include "stdafx.h"
#include "BeaconListener.h"

#include <boost/bind.hpp>
#include <utility>

namespace
{

class receiver
{
public:

    receiver(boost::asio::io_service& io_service,
             const boost::asio::ip::address& listen_address,
             const boost::asio::ip::address& multicast_address,
             const unsigned short multicast_port,
             xplaneudpcpp::BeaconListener::Callback& callback)
        : socket_(io_service), m_callback(callback)
    {
        // Create the socket so that multiple may be bound to the same address.
        boost::asio::ip::udp::endpoint listen_endpoint(
            listen_address, multicast_port);
        socket_.open(listen_endpoint.protocol());
        socket_.set_option(boost::asio::ip::udp::socket::reuse_address(true));
        socket_.bind(listen_endpoint);

        // Join the multicast group.
        socket_.set_option(
            boost::asio::ip::multicast::join_group(multicast_address));

        socket_.async_receive_from(
            boost::asio::buffer(data_.data(), max_length), sender_endpoint_,
            boost::bind(&receiver::handle_receive_from, this,
                      boost::asio::placeholders::error,
                      boost::asio::placeholders::bytes_transferred));
    }
private:

#pragma pack (push, r1, 1)

    struct becn_struct
    {
        uint8_t beacon_major_version;	// 1 at the time of X-Plane 10.40
        uint8_t beacon_minor_version;	// 1 at the time of X-Plane 10.40
        int32_t application_host_id;		// 1 for X-Plane, 2 for PlaneMaker
        int32_t version_number;		// 104103 for X-Plane 10.41r3
        uint32_t role;					// 1 for master, 2 for extern visual, 3 for IOS
        uint16_t port;				// port number X-Plane is listening on, 49000 by default
        char	computer_name[500];	// the hostname of the computer, e.g. “Joe’s Macbook”
    };

#pragma pack (pop, r1)

    void handle_receive_from(
        const boost::system::error_code& error,
        size_t bytes_recvd)
    {
        if (!error)
        {
            bool found = false;

            std::string prologue(data_.begin(), data_.begin() + 4);
            if (prologue == "BECN")
            {
                becn_struct& becn = reinterpret_cast<becn_struct&>(data_.at(5));
                xplaneudpcpp::BeaconListener::ServerInfo serverInfo;
                serverInfo.hostId = becn.application_host_id;
                serverInfo.version = becn.version_number;
                serverInfo.host = sender_endpoint_.address().to_string();
                serverInfo.port = becn.port;

                found = m_callback(serverInfo);
            } 

            if (!found)
            {
                socket_.async_receive_from(
                    boost::asio::buffer(data_.data(), max_length), sender_endpoint_,
                    boost::bind(&receiver::handle_receive_from, this,
                              boost::asio::placeholders::error,
                              boost::asio::placeholders::bytes_transferred));
            }
        }
    }

private:

    boost::asio::ip::udp::socket socket_;
    boost::asio::ip::udp::endpoint sender_endpoint_{};
    enum { max_length = 1024 };
    std::array<char, max_length> data_{};
    xplaneudpcpp::BeaconListener::Callback& m_callback;
};
}

xplaneudpcpp::BeaconListener::BeaconListener(Callback callback)
    : m_callback(callback)
{
    m_thread = std::make_unique<std::thread>([this]()
    {
        const auto ownAddress = "0.0.0.0";
        const auto multicastAddress = "239.255.1.1";
        const unsigned short multicastPort = 49707;

        receiver r(io_service,
                   boost::asio::ip::address::from_string(ownAddress),
                   boost::asio::ip::address::from_string(multicastAddress),
                   multicastPort, m_callback);

        io_service.run();
    });

}

xplaneudpcpp::BeaconListener::~BeaconListener()
{
    io_service.stop();

    m_thread->join();
}

