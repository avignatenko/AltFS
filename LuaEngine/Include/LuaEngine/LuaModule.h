#pragma once

#include <continuable/continuable.hpp>

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
    // virtual sol::state& getLua() = 0;

    virtual cti::continuable<> runAsync(std::function<void(sol::state&)> f) = 0;
};
