#pragma once

#include <map>
#include <memory>
#include <filesystem>
#include <sol/tuple.hpp>

namespace sol { class state;}


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
};
