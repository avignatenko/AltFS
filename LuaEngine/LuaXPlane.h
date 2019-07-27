#pragma once

#include "LuaModule.h"

#include <memory>
namespace xplaneudpcpp { class BeaconListener; class UDPClient;}

class LuaXPlane: public LuaModule
{
public:

    LuaXPlane();
    ~LuaXPlane();

    // Inherited via LuaModule
    virtual void init(LuaModuleAPI& api) override;

    using XPlaneConnectCallback = std::function<void(bool)>;

    void setConnectedCallback(XPlaneConnectCallback callback) { m_xplaneConnectCallback = callback; }
    bool isConnected() const { return m_xplaneClient != nullptr; }
private:

    // x-plane
    std::unique_ptr<xplaneudpcpp::BeaconListener> m_xplaneDiscoverer;
    std::unique_ptr<xplaneudpcpp::UDPClient> m_xplaneClient;
    XPlaneConnectCallback  m_xplaneConnectCallback;
};