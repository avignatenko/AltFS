#include "../Include/LuaEngine/LuaLogging.h"
#include <spdlog/spdlog.h>
#include <sol/sol.hpp>
#include "../Include/LuaEngine/LuaModule.h"
#include "StdAfx.h"

LuaLogging::LuaLogging(LuaModuleAPI& api) : api_(api) {}

LuaLogging::~LuaLogging() {}

void LuaLogging::init()
{
    api_.getLua()["loglevel"] = api_.getLua().create_table_with(
        "trace", spdlog::level::trace, "debug", spdlog::level::debug, "info", spdlog::level::info, "warn",
        spdlog::level::warn, "err", spdlog::level::err, "critical", spdlog::level::critical);

    api_.getLua().set_function("log", [](int level, const std::string log)
                               { spdlog::log(static_cast<spdlog::level::level_enum>(level), log); });
}
