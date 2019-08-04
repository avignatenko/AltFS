#include "stdafx.h" 

#include "FSUIPCEngine.h"
#include "../XPlaneUDPClientCpp/BeaconListener.h"

#include <spdlog/spdlog.h>

#pragma pack (push, r1, 1)

typedef struct tagXC_ACTION_READ_HDR
{
    DWORD action;
    DWORD offset;
    DWORD size;
    void* data;
} XC_ACTION_READ_HDR;

typedef struct tagXC_ACTION_WRITE_HDR
{
    DWORD action;
    DWORD offset;
    DWORD size;
} XC_ACTION_WRITE_HDR;


#pragma pack (pop, r1)


#define XC_ACTION_READ				1
#define XC_ACTION_WRITE			2

#define XC_RETURN_FAILURE			0
#define XC_RETURN_SUCCESS			1


FSUIPCEngine::FSUIPCEngine(const std::filesystem::path& scriptPath)
    : m_lua(scriptPath)
    , m_xPlaneModule(m_lua)
    , m_logModule(m_lua)
   
{
    
}

FSUIPCEngine::~FSUIPCEngine()
{
   
    // fixme: when to clear?
    for (auto& elem : m_fileMap)
    {
        UnmapViewOfFile(elem.second.second);
        CloseHandle(elem.second.first);
    }
}

promise::Defer FSUIPCEngine::init()
{
   
    /*
    m_xPlaneModule.discover()
    .then([this](xplaneudpcpp::BeaconListener::ServerInfo& info)
    {
        return m_xPlaneModule.connect(info.host, info.port);
    })*/
    return 
        m_xPlaneModule.connect("192.168.114", 49000)
        .then([&]{m_logModule.init();})
        .then([&]{m_xPlaneModule.init();})
        .then([&]{m_lua.load();});
}

LRESULT FSUIPCEngine::processMessage(WPARAM wParam, LPARAM lParam)
{

    XC_ACTION_READ_HDR* pHdrR = NULL;
    XC_ACTION_WRITE_HDR* pHdrW = NULL;

    ATOM atom = (ATOM)wParam;
    LRESULT result = XC_RETURN_SUCCESS;
    HANDLE hMap;
    BYTE* pView;


    const auto found = m_fileMap.find(atom);
    if (found == m_fileMap.end())
    {
        char szName[MAX_PATH];
        GlobalGetAtomName(atom, szName, MAX_PATH);
        hMap = OpenFileMapping(FILE_MAP_WRITE, FALSE, szName);
        pView = static_cast<BYTE*>(MapViewOfFile(hMap, FILE_MAP_WRITE, 0, 0, 0));
        m_fileMap[atom] = {hMap, pView};
    }
    else
    {
        hMap = found->second.first;
        pView = found->second.second;
    }

    if (!hMap || !pView) return XC_RETURN_FAILURE;

    BYTE* pNext = pView;
    DWORD* pdw = reinterpret_cast<DWORD*>(pView);

    while (*pdw)
    {
        switch (*pdw)
        {
        case XC_ACTION_READ:
            pHdrR = reinterpret_cast<XC_ACTION_READ_HDR*>(pdw);

            readFromSim(pHdrR->offset, pHdrR->size, pNext + sizeof(XC_ACTION_READ_HDR));

            pNext += sizeof(XC_ACTION_READ_HDR);
            pNext += pHdrR->size;
            pdw = reinterpret_cast<DWORD*>(pNext);
            break;

        case XC_ACTION_WRITE:
            pHdrW = reinterpret_cast<XC_ACTION_WRITE_HDR*>(pdw);

            writeToSim(pHdrW->offset, pHdrW->size, pNext + sizeof(XC_ACTION_WRITE_HDR));

            pNext += sizeof(XC_ACTION_WRITE_HDR);
            pNext += pHdrW->size;
            pdw = reinterpret_cast<DWORD*>(pNext);
            break;
        default:
            *pdw = 0;
            break;
        }
    }

    return result;

}

void FSUIPCEngine::readFromSim(DWORD offset, DWORD size, void* data)
{
    spdlog::debug("Read request, offset {0:04#x}, size {1}", offset, size);

    // "FSUIPC" version (return 8.0)
    if (offset == 0x3304 && size == 4)
    {
        *reinterpret_cast<DWORD*>(data) = 0x80000000;
        return;
    }

    // "Sim" version (return P3D)
    if (offset == 0x3308 && size == 4)
    {
        *reinterpret_cast<DWORD*>(data) = 0xFADE000A;
        return;
    }

    m_lua.readFromSim(offset, size, static_cast<std::byte*>(data));

}


promise::Defer FSUIPCEngine::writeToSim(DWORD offset, DWORD size, const void* data)
{
    return promise::newPromise([&](promise::Defer& d)
    {
        spdlog::debug("Write request, offset 0x{0:04#x}, size {1}", offset, size);

        m_lua.writeToSim(offset, size, static_cast<const std::byte*>(data));
    });


}

