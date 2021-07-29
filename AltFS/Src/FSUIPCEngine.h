#pragma once

#include <filesystem>
#include <map>
#include <memory>

#include <LuaEngine/LuaEngine.h>
#include <LuaEngine/LuaLogging.h>
#include <LuaEngine/LuaXPlane.h>

#define WIN32_LEAN_AND_MEAN  // Exclude rarely-used stuff from Windows headers
#include <windows.h>

class FSUIPCEngine
{
public:
    FSUIPCEngine(const std::filesystem::path& scriptPath);
    ~FSUIPCEngine();

    promise::Defer init();

    LRESULT processMessage(WPARAM wParam, LPARAM lParam);

private:
    void readFromSim(DWORD offset, DWORD size, void* data);
    promise::Defer writeToSim(DWORD offset, DWORD size, const void* data);

private:
    std::map<ATOM, std::pair<HANDLE, BYTE*>> m_fileMap;

    LuaEngine m_lua;
    LuaXPlane m_xPlaneModule;
    LuaLogging m_logModule;
};
