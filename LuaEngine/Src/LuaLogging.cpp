#include "../Include/LuaEngine/LuaLogging.h"
#include <XPlaneUDPClientCpp/ActiveObject.h>
#include <spdlog/spdlog.h>
#include <sol/sol.hpp>
#include "../Include/LuaEngine/LuaModule.h"
#include "stdafx.h"

LuaLogging::LuaLogging(LuaModuleAPI& api) : api_(api) {}

LuaLogging::~LuaLogging() {}

promise::Defer LuaLogging::init()
{
    return newPromiseAsync(
        api_.getLuaRunner(),
        [this](Runner* caller, promise::Defer d)
        {
            api_.getLua()["loglevel"] = api_.getLua().create_table_with(
                "trace", spdlog::level::trace, "debug", spdlog::level::debug, "info", spdlog::level::info, "warn",
                spdlog::level::warn, "err", spdlog::level::err, "critical", spdlog::level::critical);

            api_.getLua().set_function("log", [](int level, const std::string log)
                                       { spdlog::log(static_cast<spdlog::level::level_enum>(level), log); });

            caller->run([=] { d.resolve(); });
        });
}
