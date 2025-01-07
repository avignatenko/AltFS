#pragma once

namespace sol
{
class state;
}

#include <functional>
class Runner;

class LuaModuleAPI
{
public:
    virtual ~LuaModuleAPI() = default;
    virtual sol::state& getLua() = 0;
};
