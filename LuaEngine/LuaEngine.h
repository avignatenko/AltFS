#pragma once

#include "../XPlaneUDPClientCpp/ActiveObject.h"
#include "LuaModule.h"

#include <filesystem>


namespace sol { class state; }

class OffsetStatsGenerator;

class LuaEngine : public ActiveObject, public LuaModuleAPI
{
public:
    LuaEngine(const std::filesystem::path& scriptPath);
    ~LuaEngine();

    // lua module api
    virtual sol::state& getLua() override { return lua();}
    virtual void runOnLuaThread(std::function<void()> func) override {runOnThread(func);}

    // own
    promise::Defer load();
    promise::Defer init();

    void readFromSim(uint32_t offset, uint32_t size, void* data);
    void writeToSim(uint32_t offset, uint32_t size, const void* data);

private:

    sol::state& lua() { return *m_lua; }

private:

    std::unique_ptr<sol::state> m_lua;
    std::filesystem::path m_scriptPath;
    std::unique_ptr<OffsetStatsGenerator> m_stats;
    bool m_initialized = false;

};