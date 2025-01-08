
// AltFS.cpp : Defines the class behaviors for the application.
//
#include "stdafx.h"

#include "FSUIPCEngine.h"

#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/stdout_sinks.h>
#include <spdlog/spdlog.h>

#include <iostream>

#include <asio.hpp>

#define WIN32_LEAN_AND_MEAN  // Exclude rarely-used stuff from Windows headers
#include <windows.h>

// register our message
static const UINT XC_CALL = RegisterWindowMessage("FsasmLib:IPC");

FSUIPCEngine* s_engine = nullptr;

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
    }

    if (uMsg == XC_CALL)
    {
        if (!s_engine) return 0;
        return s_engine->processMessage(wParam, lParam);
    }

    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

BOOL WINAPI CtrlHandler(DWORD fdwCtrlType)
{
    // todo: add closing
    spdlog::shutdown();

    return FALSE;  // let others work on this
}

INT WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR lpCmdLine, INT nCmdShow)
{
    if (AllocConsole())
    {
        FILE *fpstdin = stdin, *fpstdout = stdout, *fpstderr = stderr;

        freopen_s(&fpstdin, "CONIN$", "r", stdin);
        freopen_s(&fpstdout, "CONOUT$", "w", stdout);
        freopen_s(&fpstderr, "CONOUT$", "w", stderr);

        SetConsoleCtrlHandler(CtrlHandler, TRUE);
        // std::cout << "This is a test of the attached console" << std::endl;
        //  FreeConsole();
    }

    // get filename of the executable
    char path[MAX_PATH + 1];
    GetModuleFileName(NULL, path, MAX_PATH);

    std::filesystem::path exeName = static_cast<LPCSTR>(path);
    std::filesystem::path exePath = exeName.parent_path();

    // init logging

    auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    console_sink->set_level(spdlog::level::info);

    auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>("altfs.log", true);
    file_sink->set_level(spdlog::level::info);

    auto logger = std::make_shared<spdlog::logger>("multi_sink", spdlog::sinks_init_list{console_sink, file_sink});
    logger->set_level(spdlog::level::info);

    spdlog::set_default_logger(logger);

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

    asio::io_context runner;
    auto work = asio::make_work_guard(runner);

    // make sure engine will be deleted before runner loop
    {
        FSUIPCEngine engine(runner, (exePath / "lua").string());

        engine.init()
            .then(
                [&engine]
                {
                    spdlog::info("FSUIPIC engine init done, starting to process messages");
                    s_engine = &engine;
                })
            .fail(
                [](std::exception_ptr e)
                {
                    try
                    {
                        std::rethrow_exception(e);
                    }
                    catch (const std::runtime_error& e)
                    {
                        ::MessageBox(NULL, fmt::format("Fatal error: {}", e.what()).c_str(), "Error", MB_OK);
                    }

                    PostQuitMessage(1);
                });

        // Run the message loop.

        MSG msg = {};
        bool endLoop = false;
        while (!endLoop)
        {
            std::this_thread::sleep_for(std::chrono::microseconds(1));

            runner.poll_one();

            if (PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE))
            {
                if (GetMessage(&msg, NULL, 0, 0))
                {
                    TranslateMessage(&msg);
                    DispatchMessage(&msg);
                }
                else
                {
                    endLoop = true;
                }
            }
        }
        s_engine = nullptr;
    }
    runner.stop();

    spdlog::info("End of run");
    spdlog::shutdown();

    return 0;
}
