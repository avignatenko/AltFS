// LuaTest.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "pch.h"

#include "../LuaEngine/LuaEngine.h"
#include "../LuaEngine/LuaXPlane.h"

#include <filesystem>
#include <iostream>


class Timer
{
public:

    Timer(std::chrono::milliseconds delay, DispatchQueue& d, std::function<void()> callback)
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
                d_.put([this]{callback_();});
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
    DispatchQueue& d_;
};

int main(int argc, char** argv)
{
    std::filesystem::path exePath(argv[0]);

    LuaEngine m_lua(exePath.parent_path() / "lua" / "script.lua");
    LuaXPlane m_xPlaneModule(m_lua);
    
    // event cycle version

    DispatchQueue d;
    bool finished = false;

    int16_t currentposition = -16383;

    Timer t(std::chrono::milliseconds(10), d, [&]
    {
        // write
         m_lua.writeToSim(0x0BB2, 2, &currentposition);
         m_lua.writeToSim(0x0BB6, 2, &currentposition);

         if (currentposition >= 16383) 
             finished = true;
         else
            ++currentposition;

         // read

         int16_t pos0, pos1;
         m_lua.readFromSim(0x0BB2, 2, &pos0);
         m_lua.readFromSim(0x0BB6, 2, &pos1);
         std::cout << "pos: " << pos0 << " " << pos1 << std::endl;

    });

    m_xPlaneModule.addConnectedCallback([&](bool) {  d.put([&]{ t.start();}); });

    m_xPlaneModule.init();
    m_lua.init();
  
    while (!finished)
        d.take()();

    // sync version

    /*
    std::mutex qlock;
    std::condition_variable m_notConnected;
    bool connected = false;

    m_xPlaneModule.setConnectedCallback([&](bool)
    {
        std::lock_guard<std::mutex> guard(qlock);
        connected = true;
        m_notConnected.notify_one();
    });

    m_lua.addModule(m_xPlaneModule);
    m_lua.init();

    // wait for connection
    std::unique_lock<std::mutex> lock(qlock);
    m_notConnected.wait(lock, [&] { return connected; });

    for (int16_t i = -16383; i <= 16383; ++i)
    {
        m_lua.writeToSim(0x0BB2, 2, &i);
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }
    */
}