#pragma once

#include "LuaModule.h"

#include <asio.hpp>

class LuaLogging
{
public:
    LuaLogging(LuaModuleAPI& api, asio::any_io_executor ex);
    ~LuaLogging();

    cti::continuable<> init();

private:
    LuaModuleAPI& api_;
    asio::any_io_executor ex_;
};
