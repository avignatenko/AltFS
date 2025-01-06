#include "../Include/XPlaneUDPClientCpp/BeaconListener.h"
#include "stdafx.h"

#include <utility>

#include <spdlog/spdlog.h>

class Receiver
{
public:
    ~Receiver() {}

    Receiver(asio::io_service& io_service, const asio::ip::address& listen_address,
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

    void startReceive(std::function<void(const xplaneudpcpp::BeaconListener::ServerInfo&)> callback)
    {
        m_callback = callback;

        socket_.async_receive_from(asio::buffer(data_.data(), max_length), sender_endpoint_,
                                   [this](const std::error_code& error, size_t bytes_recvd)
                                   { handle_receive_from(error, bytes_recvd); });

        spdlog::info("BeaconListener listening started");
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
        char computer_name[500];       // the hostname of the computer, e.g. “Joe’s Macbook”
    };

#pragma pack(pop, r1)

    void handle_receive_from(const std::error_code& error, size_t bytes_recvd)
    {
        spdlog::info("Multicast data refeived");

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

                spdlog::info("Data parsed as X-Plane data: host {}, port: {}", serverInfo.host, serverInfo.port);
                spdlog::info("Running callback...");

                m_callback(serverInfo);

                spdlog::info("Callback returned {}", found);
            }
        }
    }

private:
    asio::ip::udp::socket socket_;
    asio::ip::udp::endpoint sender_endpoint_{};
    enum
    {
        max_length = 1024
    };
    std::array<char, max_length> data_{};
    std::function<void(const xplaneudpcpp::BeaconListener::ServerInfo&)> m_callback;
};

xplaneudpcpp::BeaconListener::BeaconListener()
{
    m_thread = std::make_unique<std::thread>(
        [this]()
        {
            asio::io_service::work w(io_service);
            io_service.run();
        });
}

promise::Defer xplaneudpcpp::BeaconListener::getXPlaneServerBroadcast()
{
    return promise::newPromise(
        [this](promise::Defer d)
        {
            io_service.dispatch(
                [this, d]
                {
                    const auto ownAddress = "0.0.0.0";
                    const auto multicastAddress = "239.255.1.1";
                    const unsigned short multicastPort = 49707;

                    auto receiver =
                        std::make_shared<Receiver>(io_service, asio::ip::address::from_string(ownAddress),
                                                   asio::ip::address::from_string(multicastAddress), multicastPort);

                    receiver->startReceive(
                        [d, receiver](const ServerInfo& info) mutable
                        {
                            d.resolve(info);

                            // have to call this, other receiver will keep a ref to itself
                            receiver.reset();
                        });
                });
        });
}

xplaneudpcpp::BeaconListener::~BeaconListener()
{
    io_service.stop();
    m_thread->join();
}
