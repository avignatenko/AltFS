#pragma once
#include <map>


class FSUIPCEngine
{
public:
    ~FSUIPCEngine();

    LRESULT processMessage(WPARAM wParam, LPARAM lParam);
private:

    void readFromSim(DWORD offset, DWORD size, void* data);
    void writeToSim(DWORD offset, DWORD size, const void* data);

private:

    std::map<ATOM, std::pair<HANDLE, BYTE*>> m_fileMap;
};
