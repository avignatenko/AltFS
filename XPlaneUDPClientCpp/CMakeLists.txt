# CMakeList.txt : CMake project for AlfBFF, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.15)

project(XPlaneUDPClientCpp)

# Add source to this project's executable.
add_library (${PROJECT_NAME})

target_sources(${PROJECT_NAME} PRIVATE
        "Include/XPlaneUDPClientCpp/BeaconListener.h"
        "Include/XPlaneUDPClientCpp/UDPClient.h"
        "Include/XPlaneUDPClientCpp/ActiveObject.h"
        "Src/StdAfx.h"
	    "Src/BeaconListener.cpp"
        "Src/UDPClient.cpp"
        "Src/ActiveObject.cpp")


  target_link_libraries(${PROJECT_NAME} PUBLIC promise-cpp CONAN_PKG::asio PRIVATE CONAN_PKG::spdlog )

  target_include_directories(${PROJECT_NAME} PRIVATE PUBLIC Include)

  target_precompile_headers(${PROJECT_NAME} PRIVATE "Src/StdAfx.h")

