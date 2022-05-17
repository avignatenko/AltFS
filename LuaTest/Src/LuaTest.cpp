// LuaTest.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "pch.h"

#include <LuaEngine/LuaEngine.h>
#include <LuaEngine/LuaXPlane.h>
#include <LuaEngine/LuaLogging.h>
#include <LuaEngine/OffsetStatsGenerator.h>

#include <XPlaneUDPClientCpp/ActiveObject.h>
#include <XPlaneUDPClientCpp/BeaconListener.h>

#include <spdlog/spdlog.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_sinks.h>
#include <spdlog/sinks/stdout_color_sinks.h>

#include <filesystem>
#include <iostream>
#include <future>


class Timer
{
public:

    Timer(std::chrono::milliseconds delay, Runner* d, std::function<void()> callback)
        : delay_(delay), callback_(callback), d_(d)
    {

    }

    void start()
    {
        if (!m_finished) stop();

        m_finished = false;

        m_timer = std::make_unique<std::thread>([this]
        {
            while (!m_finished)
            {
                std::this_thread::sleep_for(delay_ );
                d_->run([this]{callback_();});
            }
        });


    }

    void stop()
    {
        m_finished = true;
        m_timer->join();
        m_timer.reset();
    }

    ~Timer()
    {
      stop();
    }

private:
    std::unique_ptr<std::thread> m_timer;
    std::atomic<bool> m_finished = true;

    std::chrono::milliseconds delay_;
    std::function<void()> callback_;
    Runner* d_;
};

class QueueRunner: public Runner
{
public:
    QueueRunner()
    {
        threadInstance = this;
    }
    ~QueueRunner()
    {
        threadInstance = nullptr;
    }

    virtual bool run(std::function<void()> func) 
    {
        m_dispatchQueue.put(func);
        return true;
    }

    void run()
    {
        while (!m_done)
        {
            auto functor = m_dispatchQueue.take();
            if (functor)
                functor();
            else
                m_done = true;
        }

        // dispatch remaing functions
        while (auto functor = m_dispatchQueue.takeNonBlocking())
            functor();
    }

    void stop()
    {
        run(nullptr);
    }
    
private:

    DispatchQueue m_dispatchQueue;
     std::atomic<bool> m_done = false;
};

int main(int argc, char** argv)
{
    bool s_exit = false;
    QueueRunner d;

    std::filesystem::path exePath(argv[0]);

    // Set the default logger to file logger
    spdlog::set_level(spdlog::level::info); // Set global log level to debug
    
    auto logFilename = exePath.parent_path() / "altfs.log";
   // auto file_logger = spdlog::basic_logger_mt("basic_logger", logFilename.string());
    auto console = spdlog::stdout_color_mt("console");   
    spdlog::set_default_logger(console);

    spdlog::info("AltFs started");

    LuaEngine m_lua(exePath.parent_path() / "lua");
    LuaXPlane m_xPlaneModule(m_lua);
    LuaLogging luaLogging(m_lua);
    
    int16_t currentposition = -16383;

    // init with current prcoess  lower 16 bits
    int currentProcessId = GetCurrentProcessId();
    int16_t startId = currentProcessId % 0xFFFF;

    Timer t(std::chrono::milliseconds(10), &d, [&]
    {
        // read
        int32_t data32;
        int16_t data16;
        int8_t data8;
        double dataf;

        m_lua.readFromSim(0x66f8, 8, (std::byte*)&dataf);
        std::cout << dataf << std::endl;
         
        //m_lua.readFromSim(0x07bc, 4, (std::byte*)&data32);
        //m_lua.readFromSim(0x07d0, 4, (std::byte*)&data32);
        //m_lua.readFromSim(0x0840, 2, (std::byte*)&data16);
        //m_lua.readFromSim(0x0bc0, 2, (std::byte*)&data16);
        //m_lua.readFromSim(0x3364, 1, (std::byte*)&data8);
        //m_lua.readFromSim(0x3365, 1, (std::byte*)&data8);
        //std::cout << "pos: " << data16 << " " << std::endl;
	
        // write
         //m_lua.writeToSim(0x0BB2, 2, (std::byte*)&currentposition);

         //m_lua.writeToSim(0x0BB6, 2, (std::byte*)&currentposition);

        //m_lua.writeToSim(0x0CC1, 2, (std::byte*)&currentposition);

         //if (currentposition >= 16383) 
        //     s_exit = true;
        // else
            ++currentposition;

         // read

         //int16_t pos0, pos1;
         //m_lua.readFromSim(0x0BB2, 2, (std::byte*)&pos0);
         //m_lua.readFromSim(0x0BB6, 2, (std::byte*)&pos1);
         //std::cout << "pos: " << pos0 << " " << pos1 << std::endl;
         /*
         double time = 0.0;
          m_lua.readFromSim(0x2ea8, 8, (std::byte*)&time);
         std::cout << (double)time << std::endl;*/
    });


     m_xPlaneModule.discover()
        .then([&](xplaneudpcpp::BeaconListener::ServerInfo& info)
        {
            return m_xPlaneModule.connect(info.host, info.port, startId);
        })
     //m_xPlaneModule.connect("192.168.0.114", 49000)
        .then([&]{ return luaLogging.init();})
        .then([&]{ return m_xPlaneModule.init();})
        .then([&]{ return m_lua.load();})
        .then([&]{d.run([&]{ t.start();}); });

     d.run();

}