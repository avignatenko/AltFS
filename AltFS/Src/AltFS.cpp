
// AltFS.cpp : Defines the class behaviors for the application.
//
#include "stdafx.h"

#include "FSUIPCEngine.h"

#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/stdout_sinks.h>
#include <spdlog/spdlog.h>

#include <iostream>

#define WIN32_LEAN_AND_MEAN  // Exclude rarely-used stuff from Windows headers
#include <windows.h>

constexpr int WM_CUSTOM_MESSAGE = WM_USER + 1;

// register our message
static const UINT XC_CALL = RegisterWindowMessage("FsasmLib:IPC");

FSUIPCEngine* s_engine = nullptr;

class MFCRunner : public Runner
{
public:
    MFCRunner()
    {
        m_threadId = ::GetCurrentThreadId();
        Runner::threadInstance = this;
    }

    ~MFCRunner() { Runner::threadInstance = nullptr; }

    virtual bool run(std::function<void()> f) override
    {
        std::function<void()>* func = new std::function<void()>(f);
        PostThreadMessage(m_threadId, WM_USER + 1, WPARAM(func), 0);
        return true;
    }

private:
    DWORD m_threadId;
};

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_DESTROY:
    {
        PostQuitMessage(0);
        return 0;
    }
    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);

        FillRect(hdc, &ps.rcPaint, (HBRUSH)(COLOR_WINDOW + 1));

        EndPaint(hwnd, &ps);
        return 0;
    }
    case WM_CUSTOM_MESSAGE:
    {
        auto* func = reinterpret_cast<std::function<void()>*>(wParam);
        (*func)();
        delete func;
        return 0;
    }
    }

    if (uMsg == XC_CALL)
    {
        return s_engine->processMessage(wParam, lParam);
    }

    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

INT WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR lpCmdLine, INT nCmdShow)
{
    if (AllocConsole())
    {
        FILE *fpstdin = stdin, *fpstdout = stdout, *fpstderr = stderr;

        freopen_s(&fpstdin, "CONIN$", "r", stdin);
        freopen_s(&fpstdout, "CONOUT$", "w", stdout);
        freopen_s(&fpstderr, "CONOUT$", "w", stderr);

        std::cout << "This is a test of the attached console" << std::endl;
        // FreeConsole();
    }

    // get filename of the executable
    char path[MAX_PATH + 1];
    GetModuleFileName(NULL, path, MAX_PATH);

    std::filesystem::path exeName = static_cast<LPCSTR>(path);
    std::filesystem::path exePath = exeName.parent_path();

    // Set the default logger to file logger
    spdlog::set_level(spdlog::level::info);  // Set global log level to debug

    // auto logFilename = exePath / "altfs.log";
    // auto file_logger = spdlog::basic_logger_mt("basic_logger", logFilename.string());
    // spdlog::set_default_logger(file_logger);
    auto console = spdlog::stdout_color_mt("console");
    spdlog::set_default_logger(console);

    spdlog::info("AltFs started");

    // Register the window class.
    const char CLASS_NAME[] = "FS98MAIN";

    WNDCLASS wc = {};

    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;

    RegisterClass(&wc);

    // Create the window.

    HWND hwnd = CreateWindowEx(0,                    // Optional window styles.
                               CLASS_NAME,           // Window class
                               "XPWideClient",       // Window text
                               WS_OVERLAPPEDWINDOW,  // Window style

                               // Size and position
                               CW_USEDEFAULT, CW_USEDEFAULT, 300, 200,

                               NULL,       // Parent window
                               NULL,       // Menu
                               hInstance,  // Instance handle
                               NULL        // Additional application data
    );

    if (hwnd == NULL)
    {
        return 0;
    }

    ShowWindow(hwnd, nCmdShow);

    MFCRunner runner;
    FSUIPCEngine engine((exePath / "lua").string());
    s_engine = &engine;

    engine.init().fail(
        [](const std::string& e)
        {
            ::MessageBox(NULL, fmt::format("Fatal error: {}", e).c_str(), "Error", MB_OK);
            PostQuitMessage(1);
        });

    // Run the message loop.

    MSG msg = {};
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    s_engine = nullptr;

    return 0;
}
