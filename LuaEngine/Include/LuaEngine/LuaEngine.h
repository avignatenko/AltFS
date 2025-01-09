#pragma once

#include "LuaModule.h"

#include <XPlaneUDPClientCpp/ActiveObject.h>

#include <asio/executor.hpp>

#include <filesystem>

namespace sol
{
class state;
}

class OffsetStatsGenerator;

class LuaEngine
{
public:
    LuaEngine(const std::filesystem::path& scriptPath);
    ~LuaEngine();

    // own
    void load();

    void readFromSim(uint32_t offset, uint32_t size, std::byte* data);
    void writeToSim(uint32_t offset, uint32_t size, const std::byte* data);

    sol::state& lua() { return *m_lua; }

private:
    enum class FSUIPCType
    {
        UInt8 = 1,
        UInt16 = 2,
        UInt32 = 3,
        UInt64 = 4,
        SInt8 = 5,
        SInt16 = 6,
        SInt32 = 7,
        SInt64 = 8,
        Float32 = 9,
        Float64 = 10,
        Array = 11
    };

private:
    std::unique_ptr<sol::state> m_lua;
    std::filesystem::path m_scriptPath;
    std::unique_ptr<OffsetStatsGenerator> m_stats;
    bool m_initialized = false;
};

class LuaEngineAsync : public LuaModuleAPI, private ActiveObject
{
public:
    LuaEngineAsync(const std::filesystem::path& scriptPath, asio::any_io_executor caller) : ActiveObject(caller)
    {
        ActiveObject::runAsync([this, scriptPath] { engine_ = std::make_unique<LuaEngine>(scriptPath); });
    }

    ~LuaEngineAsync()
    {
        ActiveObject::runAsync([this] { engine_.reset(); });
    }

    cti::continuable<> load()
    {
        return ActiveObject::runAsync([this] { engine_->load(); });
    }

    cti::continuable<> readFromSim(uint32_t offset, uint32_t size, std::byte* data)
    {
        return ActiveObject::runAsync([this, offset, size, data] { engine_->readFromSim(offset, size, data); });
    }
    cti::continuable<> writeToSim(uint32_t offset, uint32_t size, const std::byte* data)
    {
        return ActiveObject::runAsync([this, offset, size, data] { engine_->writeToSim(offset, size, data); });
    }

private:
    // lua module api

    cti::continuable<> runAsync(std::function<void(sol::state&)> f) override
    {
        return cti::async_on([this, f] { f(engine_->lua()); }, asioPostExecutor(context().get_executor()));
    }

private:
    std::unique_ptr<LuaEngine> engine_;
};
