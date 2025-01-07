# CMakeList.txt : CMake project for AlfBFF, include source and define
# project specific logic here.
#
cmake_minimum_required(VERSION 3.15)

project(AltFSMain)

# Add source to this project's executable.
add_executable(${PROJECT_NAME} WIN32)

target_sources(
    ${PROJECT_NAME}
    PRIVATE
        "Src/FSUIPCEngine.cpp"
        "Src/FSUIPCEngine.h"
        "Src/StdAfx.h"
        "Src/AltFS.cpp"
)

target_link_libraries(
    ${PROJECT_NAME}
    PRIVATE LuaEngine XPlaneUDPClientCpp spdlog::spdlog
)

target_precompile_headers(${PROJECT_NAME} PRIVATE "Src/StdAfx.h")

target_compile_definitions(${PROJECT_NAME} PRIVATE _AFXDLL)

add_custom_command(
    TARGET ${PROJECT_NAME}
    POST_BUILD
    COMMAND
        ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/scripts/
        $<TARGET_FILE_DIR:${PROJECT_NAME}>/lua
)
