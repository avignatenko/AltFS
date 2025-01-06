#include "stdafx.h"

#include "../Include/LuaEngine/LuaEngine.h"
#include "../Include/LuaEngine/LuaModule.h"
#include "../Include/LuaEngine/OffsetStatsGenerator.h"

#include <sol/sol.hpp>

#include <spdlog/spdlog.h>

#include <future>

LuaEngine::LuaEngine(const std::filesystem::path& scriptPath)
    : m_lua(std::make_unique<sol::state>()),
      m_scriptPath(scriptPath),
      m_stats(std::make_unique<OffsetStatsGenerator>(scriptPath / "stats.csv"))
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
    return newPromiseAsync(this,
                           [this](Runner* caller, promise::Defer d)
                           {
                               lua().open_libraries(sol::lib::base, sol::lib::package, sol::lib::string);

                               try
                               {
                                   auto result = lua().safe_script_file((m_scriptPath / "script.lua").string());
                               }
                               catch (const std::exception& ex)
                               {
                                   spdlog::critical("An error occurred: {}", ex.what());
                                   caller->run([d, ex = std::string(ex.what())] { d.reject(ex); });
                               }

                               m_initialized = true;
                               caller->run([d] { d.resolve(); });
                           });
}

promise::Defer LuaEngine::readFromSim(uint32_t offset, uint32_t size, std::byte* data)
{
    return promise::newPromise(
        [this, offset, size, data](promise::Defer& d)
        {
            struct Result
            {
                bool result;
                std::string error;
            };

            std::promise<Result> retval;

            run(
                [this, offset, size, data, &retval]
                {
                    sol::optional<sol::table> offsetTable = lua()["offsets"][offset];
                    if (offsetTable.has_value())
                    {
                        FSUIPCType type = offsetTable.value()[1];
                        sol::protected_function readFunc = offsetTable.value()[2];
                        sol::protected_function_result result = readFunc();
                        if (!result.valid())
                        {
                            sol::error err = result;
                            spdlog::error("An error occurred: {}", err.what());
                            retval.set_value({false, err.what()});
                            return;
                        }

                        static const std::map<FSUIPCType, int> Type2Size = {
                            {FSUIPCType::UInt8, 1},   {FSUIPCType::UInt16, 2}, {FSUIPCType::UInt32, 4},
                            {FSUIPCType::UInt64, 8},  {FSUIPCType::SInt8, 1},  {FSUIPCType::SInt16, 2},
                            {FSUIPCType::SInt32, 4},  {FSUIPCType::SInt64, 8}, {FSUIPCType::Float32, 4},
                            {FSUIPCType::Float64, 8}, {FSUIPCType::Array, -1}};

                        int expectedSize = Type2Size.at(type);

                        if (expectedSize >= 0 && size != expectedSize)
                        {
                            m_stats->reportUnknownOffset(offset, size, false);
                            spdlog::error("An error occurred: wrong offset size {} for offset {}", size, offset);
                            retval.set_value({false, "Wrong offset size"});
                            return;
                        }

                        switch (type)
                        {
                        case FSUIPCType::Array:
                        {
                            std::string value = result.get<std::string>();
                            std::copy(value.begin(), value.end(), (uint8_t*)data);
                            break;
                        }
                        case FSUIPCType::UInt8:
                        {
                            uint8_t value = result.get<uint8_t>();
                            *reinterpret_cast<uint8_t*>(data) = value;
                            break;
                        }
                        case FSUIPCType::UInt16:
                        {
                            uint16_t value = result.get<uint16_t>();
                            *reinterpret_cast<uint16_t*>(data) = value;
                            break;
                        }
                        case FSUIPCType::UInt32:
                        {
                            uint32_t value = result.get<uint32_t>();
                            *reinterpret_cast<uint32_t*>(data) = value;
                            break;
                        }
                        case FSUIPCType::SInt16:
                        {
                            int16_t value = result.get<int16_t>();
                            *reinterpret_cast<int16_t*>(data) = value;
                            break;
                        }
                        case FSUIPCType::SInt32:
                        {
                            int32_t value = result.get<int32_t>();
                            *reinterpret_cast<int32_t*>(data) = value;
                            break;
                        }

                        case FSUIPCType::Float64:
                        {
                            double value = result.get<double>();
                            *reinterpret_cast<double*>(data) = value;
                            break;
                        }
                        default:
                            // fixme, where is better place?
                            spdlog::error("Invalid datatype on read {}/{}: {}", offset, size, static_cast<int>(type));
                            retval.set_value({false, "Invalid datatype"});
                            return;
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

            if (result.result)
                d.resolve();
            else
                d.reject(result.error);
        });
}

promise::Defer LuaEngine::writeToSim(uint32_t offset, uint32_t size, const std::byte* data)
{
    std::vector<std::byte> dataVec(data, data + size);

    return newPromiseAsync(this,
                           [this, offset, dataVec](Runner* caller, promise::Defer d)
                           {
                               sol::optional<sol::table> offsetTable = lua()["offsets"][offset];
                               if (offsetTable.has_value())
                               {
                                   int type = offsetTable.value()[1];
                                   sol::protected_function writeFunc = offsetTable.value()[3];
                                   sol::protected_function_result result;
                                   switch (type)
                                   {
                                   case 1:  // uint8
                                       result = writeFunc(*reinterpret_cast<const uint8_t*>(dataVec.data()));
                                       break;
                                   case 2:  // uint16
                                       result = writeFunc(*reinterpret_cast<const uint16_t*>(dataVec.data()));
                                       break;
                                   case 6:  // sint16
                                       result = writeFunc(*reinterpret_cast<const int16_t*>(dataVec.data()));
                                       break;
                                   }

                                   if (result.valid())
                                   {
                                       caller->run([=] { d.resolve(); });
                                   }
                                   else
                                   {
                                       sol::error err = result;
                                       spdlog::critical("An error occurred: {}", err.what());
                                       caller->run([=, err = std::string(err.what())] { d.reject(err); });
                                   }
                               }
                               else
                               {
                                   m_stats->reportUnknownOffset(offset, dataVec.size(), true);
                                   caller->run([=] { d.reject(); });
                               }
                           });
}
