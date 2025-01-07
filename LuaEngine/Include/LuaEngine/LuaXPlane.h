#pragma once

#include "LuaModule.h"

#include <XPlaneUDPClientCpp/BeaconListener.h>

#include <asio.hpp>
#include <continuable/continuable.hpp>

#include <memory>

namespace xplaneudpcpp
{
class UDPClient;
}  // namespace xplaneudpcpp

class LuaXPlane
{
public:
    LuaXPlane(LuaModuleAPI& api, asio::io_context& ex);
    ~LuaXPlane();

    cti::continuable<xplaneudpcpp::BeaconListener::ServerInfo> discover();
    cti::continuable<> connect(const std::string& address, int port, int localPort);

    void init();

    bool isConnected() const { return xplaneClient_ != nullptr; }

private:
    // x-plane
    std::unique_ptr<xplaneudpcpp::UDPClient> xplaneClient_;

    LuaModuleAPI& api_;

    static LuaXPlane* s_instance;

    friend class Dataref;

    asio::io_context& ex_;
};
