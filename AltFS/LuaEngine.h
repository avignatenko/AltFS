#pragma once

#include "ActiveObject.h"
#include "LuaModule.h"

#include <filesystem>

namespace sol { class state; }


class LuaEngine : public ActiveObject, public LuaModuleAPI
{
public:
    LuaEngine(const std::filesystem::path& scriptPath);
    ~LuaEngine();

    // lua module api
    virtual sol::state& getLua() override { return lua();}
    virtual void runOnLuaThread(std::function<void()> func) override {m_dispatchQueue.put(func);}

    // own
    void addModule(LuaModule& module);
    void init();

    void readFromSim(DWORD offset, DWORD size, void* data);
    void writeToSim(DWORD offset, DWORD size, const void* data);

private:

    sol::state& lua() { return *m_lua; }

private:

    std::unique_ptr<sol::state> m_lua;
    std::filesystem::path m_scriptPath;

};