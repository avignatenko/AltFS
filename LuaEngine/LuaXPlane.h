#pragma once

#include "LuaModule.h"

#include <memory>
namespace xplaneudpcpp { class BeaconListener; class UDPClient;}

class LuaXPlane
{
public:

    LuaXPlane(LuaModuleAPI& api);
    ~LuaXPlane();

    void init();

    using XPlaneConnectCallback = std::function<void(bool)>;

    void addConnectedCallback(XPlaneConnectCallback callback) { xplaneConnectCallback_.push_back(callback); }
    
    bool isConnected() const { return xplaneClient_ != nullptr; }

private:

    // x-plane
    std::unique_ptr<xplaneudpcpp::BeaconListener> xplaneDiscoverer_;
    std::unique_ptr<xplaneudpcpp::UDPClient> xplaneClient_;
    std::vector<XPlaneConnectCallback>  xplaneConnectCallback_;

    LuaModuleAPI& api_;

    static LuaXPlane* s_instance;

    friend class Dataref;
};