#pragma once

#include "ActiveObject.h"

#include <filesystem>

namespace sol { class state;}

class LuaEngine: public ActiveObject
{
public:
    LuaEngine(const std::filesystem::path& scriptPath);
    ~LuaEngine();

    void readFromSim(DWORD offset, DWORD size, void* data);
    void writeToSim(DWORD offset, DWORD size, const void* data);

private:

    sol::state& lua()
    {
        return *m_lua;
    }

private:

       std::unique_ptr<sol::state> m_lua;
 
};