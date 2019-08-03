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
      // ...
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

            caller->run([d, err]
            {
                d.reject(std::string(err.what()));
            });
        }
        else
        {
            m_initialized = true;
            caller->run([d] {   d.resolve(); });
        }


    });

}

promise::Defer LuaEngine::readFromSim(uint32_t offset, uint32_t size, std::byte* data)
{
    return promise::newPromise([this, offset, size, data](promise::Defer& d)
    {
        std::promise<bool> retval;

        run([this, offset, size, data, &retval]
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

                retval.set_value(true);

            }
            else
            {
                m_stats->reportUnknownOffset(offset, size, false);
                retval.set_value(false);
            }

    
        });

        // wait
        bool result = retval.get_future().get();

        if (result) d.resolve(); else d.reject();
    });



}

promise::Defer LuaEngine::writeToSim(uint32_t offset, uint32_t size, const std::byte* data)
{
     std::vector<std::byte> dataVec(data, data + size);

    return newPromiseAsync(this, [this, offset, dataVec](Runner* caller, promise::Defer d)
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

            d.resolve();
        }
        else
        {
            m_stats->reportUnknownOffset(offset, dataVec.size(), true);
            d.reject();
        }
    });


}
