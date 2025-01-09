#pragma once

#include <asio.hpp>
#include <continuable/continuable.hpp>

namespace xplaneudpcpp
{
namespace BeaconListener
{
struct ServerInfo
{
    int hostId = 0;
    int version = 0;
    std::string host;
    int port = 0;
};

cti::continuable<ServerInfo> getXPlaneServerBroadcastAsync(asio::any_io_executor ex);
};  // namespace BeaconListener

}  // namespace xplaneudpcpp
