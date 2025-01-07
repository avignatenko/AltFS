#pragma once

#include "LuaModule.h"

#include <filesystem>

namespace sol
{
class state;
}

class OffsetStatsGenerator;

class LuaEngine : public LuaModuleAPI
{
public:
    LuaEngine(const std::filesystem::path& scriptPath);
    ~LuaEngine();

    // lua module api
    virtual sol::state& getLua() override { return lua(); }

    // own
    void load();

    void readFromSim(uint32_t offset, uint32_t size, std::byte* data);
    void writeToSim(uint32_t offset, uint32_t size, const std::byte* data);

private:
    sol::state& lua() { return *m_lua; }

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
