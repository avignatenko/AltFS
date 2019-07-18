#pragma once

#include <map>
#include <memory>
#include <filesystem>

#include "LuaEngine.h"

namespace xplaneudpcpp { class BeaconListener; class UDPClient;}

class FSUIPCEngine
{
public:
    FSUIPCEngine(const std::filesystem::path& scriptPath);
    ~FSUIPCEngine();

    LRESULT processMessage(WPARAM wParam, LPARAM lParam);

private:

    void readFromSim(DWORD offset, DWORD size, void* data);
    void writeToSim(DWORD offset, DWORD size, const void* data);

  
private:

    std::map<ATOM, std::pair<HANDLE, BYTE*>> m_fileMap;

    LuaEngine m_lua;

    // x-plane
    std::mutex m_xplaneClientLock;
    std::unique_ptr<xplaneudpcpp::BeaconListener> m_xplaneDiscoverer;
    std::unique_ptr<xplaneudpcpp::UDPClient> m_xplaneClient;
};
