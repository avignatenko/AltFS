
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

struct minimal_io_executor
{
    const unsigned int kCustomMessage = WM_USER + 1;

    asio::execution_context* context_;
    DWORD threadId_;

    bool operator==(const minimal_io_executor& other) const noexcept
    {
        return context_ == other.context_ && threadId_ == other.threadId_;
    }

    bool operator!=(const minimal_io_executor& other) const noexcept { return !(*this == other); }

    asio::execution_context& query(asio::execution::context_t) const noexcept { return *context_; }

    static constexpr asio::execution::blocking_t::never_t query(asio::execution::blocking_t) noexcept
    {
        // This executor always has blocking.never semantics.
        return asio::execution::blocking.never;
    }

    template <class F>
    void execute(F f) const
    {
        auto* t = new fu2::unique_function<void()>([f = std::forward<F>(f)]() mutable { std::move(f)(); });
        ::PostThreadMessage(threadId_, kCustomMessage, (WPARAM)t, 0);
    }
};

namespace asio
{
namespace traits
{

#if !defined(ASIO_HAS_DEDUCED_EXECUTE_MEMBER_TRAIT)

template <typename F>
struct execute_member<minimal_io_executor, F>
{
    static constexpr bool is_valid = true;
    static constexpr bool is_noexcept = true;
    typedef void result_type;
};

#endif  // !defined(BOOST_ASIO_HAS_DEDUCED_EXECUTE_MEMBER_TRAIT)
#if !defined(ASIO_HAS_DEDUCED_EQUALITY_COMPARABLE_TRAIT)

template <>
struct equality_comparable<minimal_io_executor>
{
    static constexpr bool is_valid = true;
    static constexpr bool is_noexcept = true;
};

#endif  // !defined(BOOST_ASIO_HAS_DEDUCED_EQUALITY_COMPARABLE_TRAIT)
#if !defined(ASIO_HAS_DEDUCED_QUERY_MEMBER_TRAIT)

template <>
struct query_member<minimal_io_executor, asio::execution::context_t>
{
    static constexpr bool is_valid = true;
    static constexpr bool is_noexcept = true;
    typedef asio::execution_context& result_type;
};

#endif  // !defined(BOOST_ASIO_HAS_DEDUCED_QUERY_MEMBER_TRAIT)
#if !defined(ASIO_HAS_DEDUCED_QUERY_STATIC_CONSTEXPR_MEMBER_TRAIT)

template <typename Property>
struct query_static_constexpr_member<
    minimal_io_executor, Property,
    typename enable_if<std::is_convertible<Property, asio::execution::blocking_t>::value>::type>
{
    static constexpr bool is_valid = true;
    static constexpr bool is_noexcept = true;
    typedef asio::execution::blocking_t::never_t result_type;
    static constexpr result_type value() noexcept { return result_type(); }
};

#endif  // !defined(BOOST_ASIO_HAS_DEDUCED_QUERY_STATIC_CONSTEXPR_MEMBER_TRAIT)

}  // namespace traits
}  // namespace asio

class WinContext : public asio::execution_context
{
public:
    WinContext() { threadId_ = ::GetCurrentThreadId(); }

    auto get_executor()
    {
        minimal_io_executor e;
        e.context_ = this;
        e.threadId_ = threadId_;
        return e;
    }

    void process(const MSG& msg)
    {
        if (msg.message == WM_USER + 1)
        {
            auto* t = (fu2::unique_function<void()>*)msg.wParam;
            (*t)();
            delete t;
        }
    }

private:
    DWORD threadId_;
};

INT WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR lpCmdLine, INT nCmdShow)
{
    if (AllocConsole())
    {
        FILE *fpstdin = stdin, *fpstdout = stdout, *fpstderr = stderr;

        freopen_s(&fpstdin, "CONIN$", "r", stdin);
        freopen_s(&fpstdout, "CONOUT$", "w", stdout);
        freopen_s(&fpstderr, "CONOUT$", "w", stderr);

        SetConsoleCtrlHandler(CtrlHandler, TRUE);
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

    WinContext runner;

    // make sure engine will be deleted before runner loop
    {
        FSUIPCEngine engine(runner.get_executor(), (exePath / "lua").string());

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

        while (GetMessage(&msg, NULL, 0, 0))
        {
            runner.process(msg);

            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        s_engine = nullptr;
    }

    spdlog::info("End of run");
    spdlog::shutdown();

    return 0;
}
