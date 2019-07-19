#pragma once

namespace sol { class state;}

class LuaModuleAPI
{
public:
    virtual ~LuaModuleAPI() = default;
    virtual sol::state& getLua() = 0;
    virtual void runOnLuaThread(std::function<void()> func) = 0;
};

class LuaModule
{
public:

    ~LuaModule() = default;

    virtual void init(LuaModuleAPI& api) = 0;
};