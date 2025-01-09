#include "stdafx.h"

#include <XPlaneUDPClientCpp/BeaconListener.h>
#include "FSUIPCEngine.h"

#include <spdlog/spdlog.h>

#pragma pack(push, r1, 1)

typedef struct tagXC_ACTION_READ_HDR_64
{
    DWORD action;
    DWORD offset;
    DWORD size;
    BYTE data[8];  // used only by client
} XC_ACTION_READ_HDR_64;

typedef struct tagXC_ACTION_READ_HDR_32
{
    DWORD action;
    DWORD offset;
    DWORD size;
    BYTE data[4];  // used only by client
} XC_ACTION_READ_HDR_32;

typedef struct tagXC_ACTION_WRITE_HDR
{
    DWORD action;
    DWORD offset;
    DWORD size;
} XC_ACTION_WRITE_HDR;

#pragma pack(pop, r1)

#define XC_ACTION_READ_64 4
#define XC_ACTION_READ 1
#define XC_ACTION_WRITE 2

#define XC_RETURN_FAILURE 0
#define XC_RETURN_SUCCESS 1

FSUIPCEngine::FSUIPCEngine(asio::any_io_executor ex, const std::filesystem::path& scriptPath)
    : m_lua(scriptPath), m_xPlaneModule(m_lua, ex), m_logModule(m_lua)

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

cti::continuable<> FSUIPCEngine::init()
{
    return m_xPlaneModule.discover()
        .then([this](xplaneudpcpp::BeaconListener::ServerInfo info)
              { return m_xPlaneModule.connect(info.host, info.port, 50000); })
        .then(
            [this]
            {
                m_logModule.init();
                m_xPlaneModule.init();
                m_lua.load();
            });
}

LRESULT FSUIPCEngine::processMessage(WPARAM wParam, LPARAM lParam)
{
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
        case XC_ACTION_READ_64:
        {
            spdlog::debug("xc read 64");
            tagXC_ACTION_READ_HDR_64* pHdrR = reinterpret_cast<tagXC_ACTION_READ_HDR_64*>(pdw);

            readFromSim(pHdrR->offset, pHdrR->size, pNext + sizeof(tagXC_ACTION_READ_HDR_64));

            pNext += sizeof(tagXC_ACTION_READ_HDR_64);
            pNext += pHdrR->size;
            pdw = reinterpret_cast<DWORD*>(pNext);
            break;
        }
        case XC_ACTION_READ:
        {
            spdlog::debug("xc read 32");
            tagXC_ACTION_READ_HDR_32* pHdrR = reinterpret_cast<tagXC_ACTION_READ_HDR_32*>(pdw);

            readFromSim(pHdrR->offset, pHdrR->size, pNext + sizeof(tagXC_ACTION_READ_HDR_32));

            pNext += sizeof(tagXC_ACTION_READ_HDR_32);
            pNext += pHdrR->size;
            pdw = reinterpret_cast<DWORD*>(pNext);
            break;
        }
        case XC_ACTION_WRITE:
        {
            spdlog::debug("write");
            XC_ACTION_WRITE_HDR* pHdrW = reinterpret_cast<XC_ACTION_WRITE_HDR*>(pdw);

            writeToSim(pHdrW->offset, pHdrW->size, pNext + sizeof(XC_ACTION_WRITE_HDR));

            pNext += sizeof(XC_ACTION_WRITE_HDR);
            pNext += pHdrW->size;
            pdw = reinterpret_cast<DWORD*>(pNext);
            break;
        }
        default:
        {
            spdlog::debug("Unknown action: {}", *pdw);
            *pdw = 0;
            break;
        }
        }
    }

    return result;
}

void FSUIPCEngine::readFromSim(DWORD offset, DWORD size, void* data)
{
    spdlog::debug("Read request, offset {0:#x}, size {1}", offset, size);

    m_lua.readFromSim(offset, size, static_cast<std::byte*>(data));
}

void FSUIPCEngine::writeToSim(DWORD offset, DWORD size, const void* data)
{
    spdlog::debug("Write request, offset {0:#x}, size {1}", offset, size);

    m_lua.writeToSim(offset, size, static_cast<const std::byte*>(data));
}
