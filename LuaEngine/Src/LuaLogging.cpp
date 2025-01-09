#include "StdAfx.h"

#include "../Include/LuaEngine/LuaLogging.h"
#include "../Include/LuaEngine/LuaModule.h"

#include <XPlaneUDPClientCpp/ActiveObject.h>

#include <spdlog/spdlog.h>
#include <sol/sol.hpp>

LuaLogging::LuaLogging(LuaModuleAPI& api, asio::any_io_executor ex) : api_(api), ex_(ex) {}

LuaLogging::~LuaLogging() {}

cti::continuable<> LuaLogging::init()
{
    return api_
        .runAsync(
            [](sol::state& lua)
            {
                lua["loglevel"] = lua.create_table_with("trace", spdlog::level::trace, "debug", spdlog::level::debug,
                                                        "info", spdlog::level::info, "warn", spdlog::level::warn, "err",
                                                        spdlog::level::err, "critical", spdlog::level::critical);

                lua.set_function("log", [](int level, const std::string log)
                                 { spdlog::log(static_cast<spdlog::level::level_enum>(level), log); });
            })
        .next(postOnAsio(ex_));
    ;
}
