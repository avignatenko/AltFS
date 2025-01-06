#pragma once

#include <promise-cpp/promise.hpp>

#include <asio.hpp>
#include <functional>
#include <thread>

namespace xplaneudpcpp
{

class BeaconListener
{
public:
    struct ServerInfo
    {
        int hostId = 0;
        int version = 0;
        std::string host;
        int port = 0;
    };

    BeaconListener();
    ~BeaconListener();

    /// Return ServerInfo as value
    promise::Defer getXPlaneServerBroadcast();

private:
    asio::io_service io_service;

    std::unique_ptr<std::thread> m_thread;
};

}  // namespace xplaneudpcpp
