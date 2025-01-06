#pragma once

#include "LuaModule.h"

#include <memory>
namespace xplaneudpcpp
{
class BeaconListener;
class UDPClient;
}  // namespace xplaneudpcpp

#define PM_MULTITHREAD
#include <promise-cpp/promise.hpp>

class LuaXPlane
{
public:
    LuaXPlane(LuaModuleAPI& api);
    ~LuaXPlane();

    promise::Defer discover();
    promise::Defer connect(const std::string& address, int port, int16_t baseId);

    promise::Defer init();

    bool isConnected() const { return xplaneClient_ != nullptr; }

private:
    // x-plane
    std::unique_ptr<xplaneudpcpp::BeaconListener> xplaneDiscoverer_;
    std::unique_ptr<xplaneudpcpp::UDPClient> xplaneClient_;

    LuaModuleAPI& api_;

    static LuaXPlane* s_instance;

    friend class Dataref;
};
