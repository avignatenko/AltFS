#include "stdafx.h"

#include "LuaEngine.h"
#include "LuaModule.h"
#include "OffsetStatsGenerator.h"

#include <sol/sol.hpp>

#include <spdlog/spdlog.h>

#include <future>



LuaEngine::LuaEngine(const std::filesystem::path& scriptPath)
    : m_lua(std::make_unique<sol::state>())
    , m_scriptPath(scriptPath)
    , m_stats(std::make_unique<OffsetStatsGenerator>(scriptPath / "stats.csv"))
{

}

LuaEngine::~LuaEngine()
{
    // run final stuff
    if (m_initialized)
    {
        std::promise<bool> retval;
        bool result = runOnThread([this, &retval]
        {
            if (isDone()) { retval.set_value(false); return; }

            bool result = lua()["shutdown"]();
            retval.set_value(result);
        });
        result = !result || retval.get_future().get();
    }

}

promise::Defer LuaEngine::load()
{
    return newPromiseAsync(this, [this](Runner* caller, promise::Defer d)
    {
        lua().open_libraries(sol::lib::base, sol::lib::package, sol::lib::string);

        // service functions
        lua().set_function("log", [](int level, const std::string log)
        {
            spdlog::debug(log);
        });


        auto result = lua().safe_script_file((m_scriptPath / "script.lua").string());

        if (!result.valid())
        {
            sol::error err = result;
            spdlog::critical("An error occurred: {}", err.what());

            caller->runOnThread([d, err] 
            { 
                d.reject(std::string(err.what())); 
            });
        }
        else
        {
            m_initialized = true;
           caller->runOnThread([d] {   d.resolve(); });
        }
      

    });
   
}

void LuaEngine::readFromSim(uint32_t offset, uint32_t size, void* data)
{
    std::promise<void> retval;

    runOnThread([this, offset, data, size, &retval]
    {
        sol::optional<sol::table> offsetTable = lua()["offsets"][offset];
        if (offsetTable.has_value())
        {
            int type = offsetTable.value()[1];
            switch (type)
            {
            case 11:
            {
                std::string value = offsetTable.value()[2]();
                std::copy(value.begin(), value.end(), (uint8_t*)data);
                break;
            }
            case 1:
            {
                float value = offsetTable.value()[2]();
                *reinterpret_cast<uint8_t*>(data) = value;
                break;
            }
            case 2:
            {
                float value = offsetTable.value()[2]();
                *reinterpret_cast<uint16_t*>(data) = value;
                break;
            }
            case 3:
            {
                float value = offsetTable.value()[2]();
                *reinterpret_cast<uint32_t*>(data) = value;
                break;
            }
            case 6:
            {
                float value = offsetTable.value()[2]();
                *reinterpret_cast<int16_t*>(data) = value;
                break;
            }

            }

        }
        else
        {
            m_stats->reportUnknownOffset(offset, size, false);
        }

        retval.set_value();
    });

    // wait
    retval.get_future().get();

}

void LuaEngine::writeToSim(uint32_t offset, uint32_t size, const void * data)
{
    // copy data
    std::vector<uint8_t> dataVec((uint8_t*)data, (uint8_t*)data + size);

    // put to queue
    runOnThread([this, dataVec, size, offset]
    {
        sol::optional<sol::table> offsetTable = lua()["offsets"][offset];
        if (offsetTable.has_value())
        {
            int type = offsetTable.value()[1];
            switch (type)
            {
            case 1: // uint8
                offsetTable.value()[3](*reinterpret_cast<const uint8_t*>(dataVec.data()));
                break;
            case 2: // uint16
                offsetTable.value()[3](*reinterpret_cast<const uint16_t*>(dataVec.data()));
                break;
            case 6: // sint16
                offsetTable.value()[3](*reinterpret_cast<const int16_t*>(dataVec.data()));
                break;
            }
        }
        else
        {
            m_stats->reportUnknownOffset(offset, size, true);
        }
    });

}

promise::Defer LuaEngine::init()
{
    return newPromiseAsync(this, [this](Runner* caller, promise::Defer d)
    {
        if (isDone()) return;

        // initialize lua script
        bool result = lua()["initialize"]();

        caller->runOnThread([d] {   d.resolve(); });
    });
}
