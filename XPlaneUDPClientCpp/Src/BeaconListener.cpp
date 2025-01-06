#include "StdAfx.h"

#include "../Include/XPlaneUDPClientCpp/BeaconListener.h"

#include <spdlog/spdlog.h>
#include <asio/io_service.hpp>
#include <asio/ip/multicast.hpp>
#include <asio/ip/udp.hpp>

#include <continuable/external/asio.hpp>
#include <utility>

class Receiver : public std::enable_shared_from_this<Receiver>
{
public:
    ~Receiver() {}

    Receiver(asio::any_io_executor& io_service, const asio::ip::address& listen_address,
             const asio::ip::address& multicast_address, const unsigned short multicast_port)
        : socket_(io_service)
    {
        // Create the socket so that multiple may be bound to the same address.
        asio::ip::udp::endpoint listen_endpoint(listen_address, multicast_port);
        socket_.open(listen_endpoint.protocol());
        socket_.set_option(asio::ip::udp::socket::reuse_address(true));
        socket_.bind(listen_endpoint);

        // Join the multicast group.
        socket_.set_option(asio::ip::multicast::join_group(multicast_address));

        spdlog::info("BeaconListener created");
    }

    cti::continuable<xplaneudpcpp::BeaconListener::ServerInfo> receiveAsync()
    {
        spdlog::info("BeaconListener listening started");
        return socket_
            .async_receive_from(asio::buffer(data_.data(), data_.size()), sender_endpoint_, cti::use_continuable)
            .then([self = this->shared_from_this()](size_t bytes_recvd)
                  { return self->handle_receive_from(bytes_recvd); });
    }

private:
#pragma pack(push, r1, 1)

    struct becn_struct
    {
        uint8_t beacon_major_version;  // 1 at the time of X-Plane 10.40
        uint8_t beacon_minor_version;  // 1 at the time of X-Plane 10.40
        int32_t application_host_id;   // 1 for X-Plane, 2 for PlaneMaker
        int32_t version_number;        // 104103 for X-Plane 10.41r3
        uint32_t role;                 // 1 for master, 2 for extern visual, 3 for IOS
        uint16_t port;                 // port number X-Plane is listening on, 49000 by default
        char computer_name[500];       // the hostname of the computer, e.g. �Joe�s Macbook�
    };

#pragma pack(pop, r1)

    xplaneudpcpp::BeaconListener::ServerInfo handle_receive_from(size_t bytes_recvd)
    {
        spdlog::info("Multicast data received");

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

            spdlog::info("Data parsed as X-Plane data: host {}, port: {}", serverInfo.host, serverInfo.port);
            return serverInfo;
        }

        throw std::runtime_error("Invalid data received");
    }

private:
    asio::ip::udp::socket socket_;
    asio::ip::udp::endpoint sender_endpoint_{};
    std::array<char, 1024> data_{};
};

cti::continuable<xplaneudpcpp::BeaconListener::ServerInfo> xplaneudpcpp::BeaconListener::getXPlaneServerBroadcastAsync(
    asio::any_io_executor ex)
{
    const auto ownAddress = "0.0.0.0";
    const auto multicastAddress = "239.255.1.1";
    const unsigned short multicastPort = 49707;

    auto receiver = std::make_shared<Receiver>(ex, asio::ip::address::from_string(ownAddress),
                                               asio::ip::address::from_string(multicastAddress), multicastPort);

    return receiver->receiveAsync();
}
