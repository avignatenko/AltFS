#pragma once

#include "LuaModule.h"

#include <XPlaneUDPClientCpp/BeaconListener.h>

#include <asio.hpp>
#include <continuable/continuable.hpp>

#include <memory>

namespace xplaneudpcpp
{
class UDPClientAsync;
}  // namespace xplaneudpcpp

class LuaXPlane
{
public:
    LuaXPlane(LuaModuleAPI& api, asio::any_io_executor ex);
    ~LuaXPlane();

    cti::continuable<xplaneudpcpp::BeaconListener::ServerInfo> discover();
    cti::continuable<> connect(const std::string& address, int port, int localPort);

    cti::continuable<> init();

    bool isConnected() const { return xplaneClient_ != nullptr; }

private:
    // x-plane
    std::unique_ptr<xplaneudpcpp::UDPClientAsync> xplaneClient_;

    LuaModuleAPI& api_;

    static LuaXPlane* s_instance;

    friend class Dataref;

    asio::any_io_executor ex_;
};
