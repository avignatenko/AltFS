#include "stdafx.h"
#include "LuaXPlane.h"


#include "../XPlaneUDPClientCpp/BeaconListener.h"
#include "../XPlaneUDPClientCpp/UDPClient.h"

#include <spdlog/spdlog.h>
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

LuaXPlane* LuaXPlane::s_instance = nullptr;

LuaXPlane::LuaXPlane(LuaModuleAPI& api) : api_(api)
{
    s_instance = this;
}

LuaXPlane::~LuaXPlane()
{
    s_instance = nullptr;
}


class Dataref
{
public:

    Dataref(const std::string& path, int type, int freq)
        : path_(path)
        , type_(type)
        , freq_(freq)
    {
    }

    float read()
    {
        if (!LuaXPlane::s_instance->xplaneClient_)
            return 0.0;

        if (!subcribed_)
        {
            LuaXPlane::s_instance->xplaneClient_->subscribeDataref(path_, freq_, [this](float value)
            {
                LuaXPlane::s_instance->api_.runOnLuaThread([this, value]
                {
                    value_ = value;
                });
            });

            subcribed_ = true;
        }


        return value_;
    }

    void write(float value)
    {
        if (!LuaXPlane::s_instance->xplaneClient_)
            return;

        LuaXPlane::s_instance->xplaneClient_->writeDataref(path_, value);
    }

private:

    std::string path_;
    int type_;
    int freq_;
    float value_ = 0.0;
    bool subcribed_ = false;
};

void LuaXPlane::init()
{
    api_.runOnLuaThread([this]
    {

        // x-plane

        auto xplane = api_.getLua()["xplane"].get_or_create<sol::table>();

        xplane["types"] = api_.getLua().create_table_with(
            "int", 1,
            "float", 2,
            "intarray", 3,
            "floatarray", 4,
            "string", 5);

        xplane.new_usertype<Dataref>("dataref",
                                     sol::constructors<Dataref(const std::string&, int, int)>(),
                                     "read", &Dataref::read,
                                     "write", &Dataref::write);


        xplane["connect"] = [this](const std::string& host, int port)
        {
            xplaneClient_ = std::make_unique<xplaneudpcpp::UDPClient>(host, port);

            api_.getLua()["connected"]();

            for (auto& callback : xplaneConnectCallback_)
                callback(true);
        };

        xplane["start_autodiscovery"] = [this]()
        {
            // start x-plane discovery
            xplaneDiscoverer_.reset(new xplaneudpcpp::BeaconListener([this](const xplaneudpcpp::BeaconListener::ServerInfo& info)
            {
                api_.runOnLuaThread([this, info] {
                    xplaneClient_ = std::make_unique<xplaneudpcpp::UDPClient>(info.host, info.port);

                    api_.getLua()["connected"]();

                    for (auto& callback : xplaneConnectCallback_)
                        callback(true);
                });

                return true;
            }));
        };
    });
}
