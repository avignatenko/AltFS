#pragma once

#include <map>
#include <memory>
#include <filesystem>

namespace lua { class State;}


class FSUIPCEngine
{
public:
    FSUIPCEngine(const std::filesystem::path& scriptPath);
    ~FSUIPCEngine();

    LRESULT processMessage(WPARAM wParam, LPARAM lParam);

private:

    void readFromSim(DWORD offset, DWORD size, void* data);
    void writeToSim(DWORD offset, DWORD size, const void* data);

    lua::State& lua()
    {
        return *m_lua;
    }

private:

    std::unique_ptr<lua::State> m_lua;
    std::map<ATOM, std::pair<HANDLE, BYTE*>> m_fileMap;
};
