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

        auto result = lua().safe_script_file((m_scriptPath / "script.lua").string());

        if (!result.valid())
        {
            sol::error err = result;
            spdlog::critical("An error occurred: {}", err.what());

            caller->run([d, err = std::string(err.what())]{ d.reject(err);});
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
        struct Result
        {
            bool result;
            std::string error;
        };

        std::promise<Result> retval;

        run([this, offset, size, data, &retval]
        {      
            sol::optional<sol::table> offsetTable = lua()["offsets"][offset];
            if (offsetTable.has_value())
            {
                int type = offsetTable.value()[1];
                sol::protected_function readFunc = offsetTable.value()[2];
                sol::protected_function_result result = readFunc();
                if (!result.valid())
                {
                    sol::error err = result;
                    spdlog::error("An error occurred: {}", err.what());
                    retval.set_value({false, err.what()});
                    return;
                }

                switch (type)
                {
                case 11:
                {
                    std::string value = result.get<std::string>();
                    std::copy(value.begin(), value.end(), (uint8_t*)data);
                    break;
                }
                case 1:
                {
                    uint8_t value = result.get<uint8_t>();
                    *reinterpret_cast<uint8_t*>(data) = value;
                    break;
                }
                case 2:
                {
                    uint16_t value = result.get<uint16_t>();
                    *reinterpret_cast<uint16_t*>(data) = value;
                    break;
                }
                case 3:
                {
                    uint32_t value = result.get<uint32_t>();
                    *reinterpret_cast<uint32_t*>(data) = value;
                    break;
                }
                case 6:
                {
                    int16_t value = result.get<int16_t>();
                    *reinterpret_cast<int16_t*>(data) = value;
                    break;
                }

                case 10:
                {
                    double value = result.get<double>();
                    *reinterpret_cast<double*>(data) = value;
                    break;
                }
                default:
                    // fixme, where is better place?
                    spdlog::error("Invalid datatype on read {}/{}: {}", offset, size, type);
                }

                retval.set_value({true, ""});

            }
            else
            {
                m_stats->reportUnknownOffset(offset, size, false);
                retval.set_value({false, "Unknown offset"});
            }

  
        });

        // wait
        Result result = retval.get_future().get();

        if (result.result) d.resolve(); else d.reject(result.error);
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
            sol::protected_function writeFunc = offsetTable.value()[3];
            sol::protected_function_result result;
            switch (type)
            {
            case 1: // uint8
                result = writeFunc(*reinterpret_cast<const uint8_t*>(dataVec.data()));
                break;
            case 2: // uint16
                result = writeFunc(*reinterpret_cast<const uint16_t*>(dataVec.data()));
                break;
            case 6: // sint16
                result = writeFunc(*reinterpret_cast<const int16_t*>(dataVec.data()));
                break;
            }

            if (result.valid())
            {
              caller->run([=]{d.resolve();});
            }
            else
            {
              sol::error err = result;
              spdlog::critical("An error occurred: {}", err.what());
              caller->run([=, err = std::string(err.what())]{d.reject(err);});
            }
        }
        else
        {
            m_stats->reportUnknownOffset(offset, dataVec.size(), true);
            caller->run([=]{d.reject();});
        }
    });


}
