#pragma once

#include "LuaModule.h"

#define PM_MULTITHREAD
#include <promise-cpp/promise.hpp>

class LuaLogging
{
public:
    LuaLogging(LuaModuleAPI& api);
    ~LuaLogging();

    void init();

private:
    LuaModuleAPI& api_;
};
