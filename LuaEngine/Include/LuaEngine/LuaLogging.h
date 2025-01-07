#pragma once

#include "LuaModule.h"

class LuaLogging
{
public:
    LuaLogging(LuaModuleAPI& api);
    ~LuaLogging();

    void init();

private:
    LuaModuleAPI& api_;
};
