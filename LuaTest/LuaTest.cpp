// LuaTest.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "pch.h"

#include "../LuaEngine/LuaEngine.h"
#include "../LuaEngine/LuaXPlane.h"

#include <filesystem>
#include <iostream>


int main(int argc,char** argv)
{
    std::cout << "Hello World!\n"; 

    std::filesystem::path exePath(argv[0]);

    LuaEngine m_lua(exePath.parent_path() / "lua" / "script.lua");
    LuaXPlane m_xPlaneModule;
    
    // 

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

}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
