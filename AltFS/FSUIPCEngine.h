#pragma once


#include <sol/tuple.hpp>

#include <map>
#include <memory>
#include <filesystem>


namespace sol { class state;}
namespace xplaneudpcpp { class BeaconListener;}

class FSUIPCEngine
{
public:
    FSUIPCEngine(const std::filesystem::path& scriptPath);
    ~FSUIPCEngine();

    LRESULT processMessage(WPARAM wParam, LPARAM lParam);

private:

    void readFromSim(DWORD offset, DWORD size, void* data);
    void writeToSim(DWORD offset, DWORD size, const void* data);

    sol::state& lua()
    {
        return *m_lua;
    }

private:

    std::unique_ptr<sol::state> m_lua;
    std::map<ATOM, std::pair<HANDLE, BYTE*>> m_fileMap;

    // x-plane
    std::unique_ptr<xplaneudpcpp::BeaconListener> m_xplaneDiscoverer;
};
