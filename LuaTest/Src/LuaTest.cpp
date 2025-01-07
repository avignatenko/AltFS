// LuaTest.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "Pch.h"

#include <LuaEngine/LuaEngine.h>
#include <LuaEngine/LuaLogging.h>
#include <LuaEngine/LuaXPlane.h>
#include <LuaEngine/OffsetStatsGenerator.h>

#include <XPlaneUDPClientCpp/BeaconListener.h>

#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/stdout_sinks.h>
#include <spdlog/spdlog.h>

#include <asio.hpp>
#include <asio/steady_timer.hpp>

#include <filesystem>
#include <future>
#include <iostream>

class PeriodicTimer
{
public:
    using Duration = typename std::chrono::steady_clock::duration;

    PeriodicTimer(asio::io_context& io, Duration duration) : timer_(io), duration_(duration) {}

    template <typename F>
    void wait(F&& func)
    {
        timer_.expires_after(std::chrono::milliseconds(0));
        timeoutHandler(func);
    }

private:
    template <typename F>
    void timeoutHandler(F&& func)
    {
        timer_.async_wait(
            [this, f = std::move(func)](std::error_code ec)
            {
                f();
                timer_.expires_at(timer_.expiry() + duration_);
                timeoutHandler(std::move(f));
            });
    }

private:
    asio::steady_timer timer_;
    Duration duration_;
};

int main(int argc, char** argv)
{
    bool s_exit = false;
    asio::io_context runner;

    std::filesystem::path exePath(argv[0]);

    // Set the default logger to file logger
    spdlog::set_level(spdlog::level::trace);  // Set global log level to debug

    auto logFilename = exePath.parent_path() / "altfs.log";
    // auto file_logger = spdlog::basic_logger_mt("basic_logger", logFilename.string());
    auto console = spdlog::stdout_color_mt("console");
    spdlog::set_default_logger(console);

    spdlog::info("AltFs started");

    LuaEngine m_lua(exePath.parent_path() / "lua");
    LuaXPlane m_xPlaneModule(m_lua, runner);
    LuaLogging luaLogging(m_lua);

    int16_t currentposition = -16383;

    // init with current prcoess  lower 16 bits
    int currentProcessId = 100;  // GetCurrentProcessId();
    int16_t startId = currentProcessId % 0xFFFF;

    PeriodicTimer t(runner, std::chrono::seconds(2));

    // m_xPlaneModule.discover()
    //   .then([&](xplaneudpcpp::BeaconListener::ServerInfo info)
    //         { return m_xPlaneModule.connect(info.host, info.port, 50000); })
    m_xPlaneModule.connect("192.168.0.114", 49000, 50000)
        .then(
            [&]
            {
                luaLogging.init();
                m_xPlaneModule.init();
                m_lua.load();

                t.wait(
                    [&]
                    {
                        // read
                        int32_t data32;
                        int16_t data16;
                        int8_t data8;
                        double dataf;

                        m_lua.readFromSim(0x023a, 1, (std::byte*)&data8);
                        std::cout << data8 << std::endl;

                        // m_lua.readFromSim(0x07bc, 4, (std::byte*)&data32);
                        // m_lua.readFromSim(0x07d0, 4, (std::byte*)&data32);
                        // m_lua.readFromSim(0x0840, 2, (std::byte*)&data16);
                        // m_lua.readFromSim(0x0bc0, 2, (std::byte*)&data16);
                        // m_lua.readFromSim(0x3364, 1, (std::byte*)&data8);
                        // m_lua.readFromSim(0x3365, 1, (std::byte*)&data8);
                        // std::cout << "pos: " << data16 << " " << std::endl;

                        // write
                        // m_lua.writeToSim(0x0BB2, 2, (std::byte*)&currentposition);

                        // m_lua.writeToSim(0x0BB6, 2, (std::byte*)&currentposition);

                        // m_lua.writeToSim(0x0CC1, 2, (std::byte*)&currentposition);

                        // if (currentposition >= 16383)
                        //     s_exit = true;
                        // else
                        ++currentposition;

                        // read

                        // int16_t pos0, pos1;
                        // m_lua.readFromSim(0x0BB2, 2, (std::byte*)&pos0);
                        // m_lua.readFromSim(0x0BB6, 2, (std::byte*)&pos1);
                        // std::cout << "pos: " << pos0 << " " << pos1 << std::endl;
                        /*
                        double time = 0.0;
                         m_lua.readFromSim(0x2ea8, 8, (std::byte*)&time);
                        std::cout << (double)time << std::endl;*/
                    });
            });

    runner.run_for(std::chrono::seconds(30));
    runner.stop();
}
