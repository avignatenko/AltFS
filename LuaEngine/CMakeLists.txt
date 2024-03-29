﻿# CMakeList.txt : CMake project for AlfBFF, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.15)

project(LuaEngine)

# Add source to this project's executable.
add_library (${PROJECT_NAME})

target_sources(${PROJECT_NAME} PRIVATE
        "Include/LuaEngine/OffsetStatsGenerator.h"
        "Include/LuaEngine/LuaEngine.h"
        "Include/LuaEngine/LuaLogging.h"
        "Include/LuaEngine/LuaModule.h"
        "Include/LuaEngine/LuaXPlane.h"
        "Src/LuaLogging.cpp"
        "Src/LuaXPlane.cpp"
        "Src/OffsetStatsGenerator.cpp"
        "Src/stdafx.h"
        "Src/LuaEngine.cpp")


  target_link_libraries(${PROJECT_NAME}  PRIVATE XPlaneUDPClientCpp promise-cpp CONAN_PKG::spdlog CONAN_PKG::sol2)

  target_include_directories(${PROJECT_NAME} PRIVATE PUBLIC Include)

  target_precompile_headers(${PROJECT_NAME} PRIVATE "Src/StdAfx.h")

  target_compile_definitions(${PROJECT_NAME} PUBLIC SOL_NO_CHECK_NUMBER_PRECISION=1)
