#include "stdafx.h"
#include "LuaXPlane.h"


#include "../XPlaneUDPClientCpp/BeaconListener.h"
#include "../XPlaneUDPClientCpp/UDPClient.h"

#include <sol/state.hpp>
#include <functional>

enum class XPlaneType
{
    kInt = 1,
    kFloat = 2,
    kIntarray = 3,
    kFloatarray = 4,
    kString = 5
};

LuaXPlane::LuaXPlane()
{
}

LuaXPlane::~LuaXPlane()
{
}

void LuaXPlane::init(LuaModuleAPI& api)
{
    // x-plane

    api.getLua()["xpl_dataref_subscribe"] = [this, &api](
        const std::string& dataref,
        XPlaneType type,
        double freq,
        sol::function callback)
    {
        spdlog::debug("read: {}", dataref);

        if (!m_xplaneClient) return;
        m_xplaneClient->subscribeDataref(dataref, freq, [callback, &api](float value)
        {
            api.runOnLuaThread([callback, value]{
                 callback(value);
            });
        });
    };

    api.getLua()["xpl_dataref_write"] = [this](
        const std::string& dataref,
        XPlaneType type,
        sol::lua_value value)
    {
        spdlog::debug("write: {}", dataref);

        if (!m_xplaneClient) return;

        float f = value.as<float>();
        m_xplaneClient->writeDataref(dataref, f);
    };

     // start x-plane discovery
    m_xplaneDiscoverer.reset(new xplaneudpcpp::BeaconListener([this, &api](const xplaneudpcpp::BeaconListener::ServerInfo& info)
    {
        api.runOnLuaThread([this, info]{
             m_xplaneClient = std::make_unique<xplaneudpcpp::UDPClient>(info.host, info.port);
        });

        return true; 
    }));
}
