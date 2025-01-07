#include "../Include/LuaEngine/LuaXPlane.h"
#include "StdAfx.h"

#include <XPlaneUDPClientCpp/BeaconListener.h>
#include <XPlaneUDPClientCpp/UDPClient.h>

#include <spdlog/spdlog.h>
#include <functional>
#include <sol/state.hpp>

enum class XPlaneType
{
    kInt = 1,
    kFloat = 2,
    kIntarray = 3,
    kFloatarray = 4,
    kString = 5
};

LuaXPlane* LuaXPlane::s_instance = nullptr;

LuaXPlane::LuaXPlane(LuaModuleAPI& api, asio::io_context& ex) : api_(api), ex_(ex)
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
    Dataref(const std::string& path, int type, int freq) : path_(path), type_(type), freq_(freq) {}

    float read()
    {
        if (!LuaXPlane::s_instance->xplaneClient_) return 0.0;

        if (!subcribed_)
        {
            LuaXPlane::s_instance->xplaneClient_->subscribeDataref(path_, freq_,
                                                                   [this](float value) { value_ = value; });

            subcribed_ = true;
        }

        return value_;
    }

    void write(float value)
    {
        if (!LuaXPlane::s_instance->xplaneClient_) return;

        LuaXPlane::s_instance->xplaneClient_->writeDataref(path_, value);
    }

private:
    std::string path_;
    int type_;
    int freq_;
    float value_ = 0.0;
    bool subcribed_ = false;
};

cti::continuable<xplaneudpcpp::BeaconListener::ServerInfo> LuaXPlane::discover()
{
    return xplaneudpcpp::BeaconListener::getXPlaneServerBroadcastAsync(ex_.get_executor());
}

cti::continuable<> LuaXPlane::connect(const std::string& address, int port, int localPort)
{
    xplaneClient_ = std::make_unique<xplaneudpcpp::UDPClient>(ex_, address, port, localPort);
    return xplaneClient_->connect();
}

void LuaXPlane::init()
{
    auto xplane = api_.getLua()["xplane"].get_or_create<sol::table>();

    xplane["types"] =
        api_.getLua().create_table_with("int", 1, "float", 2, "intarray", 3, "floatarray", 4, "string", 5);

    xplane.new_usertype<Dataref>("dataref", sol::constructors<Dataref(const std::string&, int, int)>(), "read",
                                 &Dataref::read, "write", &Dataref::write);
}
