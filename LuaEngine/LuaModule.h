#pragma once

namespace sol { class state;}

#include <functional>

class LuaModuleAPI
{
public:
    virtual ~LuaModuleAPI() = default;
    virtual sol::state& getLua() = 0;
    virtual void runOnLuaThread(std::function<void()> func) = 0;
};
