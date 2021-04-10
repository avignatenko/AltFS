# CMakeList.txt : CMake project for AlfBFF, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.15)

project(AltFSMain)

# Add source to this project's executable.
add_executable (${PROJECT_NAME})

target_sources(${PROJECT_NAME} 
    PRIVATE 
        "Src/FSUIPCEngine.cpp"
        "Src/FSUIPCEngine.h"
        "Src/resource.h"
        "Src/StdAfx.h"
        "Src/AltFS.cpp"
        "Src/AltFS.h"
        "Src/AltFS.rc"
        "Src/AltFSDlg.cpp"
        "Src/AltFSDlg.h"
       )


  target_link_libraries(${PROJECT_NAME}  PRIVATE promise-cpp LuaEngine XPlaneUDPClientCpp spdlog::spdlog Boost::system)

  target_precompile_headers(${PROJECT_NAME} PRIVATE "Src/StdAfx.h")

  target_compile_definitions(${PROJECT_NAME} PRIVATE _AFXDLL)

