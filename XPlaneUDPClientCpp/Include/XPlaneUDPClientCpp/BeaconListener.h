#pragma once

#include <asio/any_io_executor.hpp>
#include <continuable/continuable.hpp>

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

    static cti::continuable<ServerInfo> getXPlaneServerBroadcast(asio::any_io_executor ex);
};

}  // namespace xplaneudpcpp
