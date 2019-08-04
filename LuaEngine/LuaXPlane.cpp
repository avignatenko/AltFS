#include "stdafx.h"
#include "LuaXPlane.h"


#include "../XPlaneUDPClientCpp/BeaconListener.h"
#include "../XPlaneUDPClientCpp/UDPClient.h"
#include "../XPlaneUDPClientCpp/ActiveObject.h"

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
        LuaXPlane::s_instance->xplaneClient_->unsubscribeDataref(path_);
    }

    float read()
    {
        if (!LuaXPlane::s_instance->xplaneClient_)
            return 0.0;

        if (!subcribed_)
        {
            LuaXPlane::s_instance->xplaneClient_->subscribeDataref(path_, freq_, [this](float value)
            {
                LuaXPlane::s_instance->api_.getLuaRunner()->run([this, value]
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

promise::Defer LuaXPlane::discover()
{
    return promise::newPromise([this](promise::Defer& p)
    {
        // start x-plane discovery
        xplaneDiscoverer_.reset(new xplaneudpcpp::BeaconListener([this, p, caller = Runner::threadInstance](const xplaneudpcpp::BeaconListener::ServerInfo& info)
        {
            caller->run([p, info]{p.resolve(info);});
            return true;
        }));
    });
}

promise::Defer LuaXPlane::connect(const std::string& address, int port)
{
    return promise::newPromise([this, address, port](promise::Defer& p)
    {
        xplaneClient_ = std::make_unique<xplaneudpcpp::UDPClient>(address, port);
        p.resolve();
    });
}

promise::Defer LuaXPlane::init()
{
    return newPromiseAsync(api_.getLuaRunner(), [this](Runner* caller, promise::Defer d)
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

        caller->run([d]{d.resolve();});

    });
}
