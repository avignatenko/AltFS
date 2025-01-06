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

set(CMAKE_PROJECT_TOP_LEVEL_INCLUDES ${CMAKE_BINARY_DIR}/conan_provider.cmake)

# compiler
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

project("AltFS" LANGUAGES CXX)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/Bin)
# global configuration
enable_testing()

find_package(asio)
find_package(spdlog)
find_package(sol2)

# windows defaults
add_definitions(-D_WIN32_WINNT=0x0A00)

if(MSVC)
    add_compile_options(/MP)
endif()

# Include sub-projects.
add_subdirectory("promise-cpp")
add_subdirectory("XPlaneUDPClientCpp")
add_subdirectory("LuaEngine")
add_subdirectory("LuaTest")
add_subdirectory("AltFS")
