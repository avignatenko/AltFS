#pragma once

#include "LuaModule.h"

namespace xplaneudpcpp { class BeaconListener; class UDPClient;}

class LuaXPlane: public LuaModule
{
public:

    LuaXPlane();
    ~LuaXPlane();

    // Inherited via LuaModule
    virtual void init(LuaModuleAPI& api) override;

private:

    // x-plane
    std::unique_ptr<xplaneudpcpp::BeaconListener> m_xplaneDiscoverer;
    std::unique_ptr<xplaneudpcpp::UDPClient> m_xplaneClient;
};