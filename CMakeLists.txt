cmake_minimum_required(VERSION 3.24)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# conan
if(NOT EXISTS "${CMAKE_BINARY_DIR}/conan_provider.cmake")
    message(
        STATUS
        "Downloading conan_provider.cmake from https://github.com/conan-io/cmake-conan"
    )
    file(
        DOWNLOAD
            "https://raw.githubusercontent.com/conan-io/cmake-conan/refs/heads/develop2/conan_provider.cmake"
            "${CMAKE_BINARY_DIR}/conan_provider.cmake"
        TLS_VERIFY ON
    )
endif()

# compiler
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_PROJECT_TOP_LEVEL_INCLUDES ${CMAKE_BINARY_DIR}/conan_provider.cmake)

project("AltFS" LANGUAGES CXX)

# global configuration
enable_testing()

find_package(asio)
find_package(spdlog)
find_package(sol2)
find_package(continuable)

# windows defaults
if(WIN32)
    add_definitions(-D_WIN32_WINNT=0x0A00)
endif()

if(MSVC)
    add_compile_options(/MP)
endif()

# Include sub-projects.

if(WIN32)
    add_subdirectory("LuaEngine")
    add_subdirectory("LuaTest")
    add_subdirectory("AltFS")
endif()

add_subdirectory("promise-cpp")
add_subdirectory("XPlaneUDPClientCpp")
