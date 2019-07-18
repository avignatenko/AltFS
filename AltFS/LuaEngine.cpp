#include "stdafx.h"
#include "LuaEngine.h"

#define SOL_ALL_SAFETIES_ON 1
#include <sol/sol.hpp>

#include <spdlog/spdlog.h>

#include <future>

enum class XPlaneType
{
    kInt = 1,
    kFloat = 2,
    kIntarray = 3,
    kFloatarray = 4,
    kString = 5
};

LuaEngine::LuaEngine(const std::filesystem::path & scriptPath)
    : m_lua(std::make_unique<sol::state>())
{
    dispatchQueue.put([this, scriptPath]
    {
        lua().open_libraries(sol::lib::base, sol::lib::package, sol::lib::string);

        // service functions
        lua().set_function("log", [](int level, const std::string log)
        {
            spdlog::debug(log);
        });

        // x-plane

        lua().set_function("xpl_dataref_subscribe", [this](
            const std::string& dataref,
            XPlaneType type,
            double freq,
            sol::function callback)
        {
            spdlog::debug(dataref);

            //m_xplaneClient->subscribeDataref(dataref, freq, [callback](float value)
           //{
            //    callback(value);
           // });


        });

        lua().set_function("xpl_dataref_write", [this](
            const std::string& dataref,
            XPlaneType type,
            sol::lua_value value)
        {
            //float f = value.as<float>();
            //m_xplaneClient->writeDataref(dataref, f);
        });


        try
        {
            auto result1 = lua().safe_script_file(scriptPath.string());
        }
        catch (const sol::error& e)
        {
            spdlog::error("an expected error has occurred: {}", e.what());
            throw;
        }


        // initialize lua script
        bool result = lua()["initialize"]();

    });


}

LuaEngine::~LuaEngine()
{
    // run final stuff

     std::promise<bool> retval;

     dispatchQueue.put([this, &retval]
     {
          bool result = lua()["shutdown"]();
          retval.set_value(result);
     });
   
     bool result = retval.get_future().get();

}

void LuaEngine::readFromSim(DWORD offset, DWORD size, void* data)
{
    std::promise<void> retval;

    dispatchQueue.put([this, offset, data, &retval]
    {
        sol::optional<sol::table> offsetTable = lua()["offsets"][offset];
        if (offsetTable.has_value())
        {
            int type = offsetTable.value()["type"];
            switch (type)
            {
            case 11: 
            {
                std::string value = offsetTable.value()["read"]();;
                std::copy(value.begin(), value.end(), (uint8_t*)data);
                break;
            }
            case 1:
            {
                uint8_t value = offsetTable.value()["read"]();
                *reinterpret_cast<uint8_t*>(data) = value;

                break;
            }
            }
        }
        retval.set_value();
    });

    // wait
    retval.get_future().get();

}

void LuaEngine::writeToSim(DWORD offset, DWORD size, const void * data)
{
    // copy data
    std::vector<uint8_t> dataVec((uint8_t*)data, (uint8_t*)data + size);

    // put to queue
    dispatchQueue.put([this, dataVec, offset]
    {
        sol::optional<sol::table> offsetTable = lua()["offsets"][offset];
        if (offsetTable.has_value())
        {
            int type = offsetTable.value()["type"];
            switch (type)
            {
            case 1:
                offsetTable.value()["write"](*reinterpret_cast<const uint8_t*>(dataVec.data()));
                break;
            }
        }
    });

}
