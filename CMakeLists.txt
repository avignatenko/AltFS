cmake_minimum_required (VERSION 3.15)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

project ("AltFS" LANGUAGES CXX)

# global configuration
enable_testing()

# conan
if(NOT EXISTS "${CMAKE_BINARY_DIR}/conan.cmake")
  message(STATUS "Downloading conan.cmake from https://github.com/conan-io/cmake-conan")
  file(DOWNLOAD "https://raw.githubusercontent.com/conan-io/cmake-conan/develop/conan.cmake"
                "${CMAKE_BINARY_DIR}/conan.cmake"
                TLS_VERIFY ON)
endif()

list(APPEND CMAKE_MODULE_PATH ${CMAKE_BINARY_DIR})
list(APPEND CMAKE_PREFIX_PATH ${CMAKE_BINARY_DIR})

include(conan)

set(CMAKE_CONFIGURATION_TYPES Debug Release)

get_property(is_multi_config GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)

if(is_multi_config)
    message(STATUS "Multi config generator detected")
    set(conan_configs_to_create ${CMAKE_CONFIGURATION_TYPES})
    set(conan_cmake_file conanbuildinfo_multi.cmake)
    set(conan_generator cmake_multi)
else()
    message(STATUS "Single config generator detected")
    set(conan_configs_to_create ${CMAKE_BUILD_TYPE})
    set(conan_cmake_file conanbuildinfo.cmake)
    set(conan_generator cmake)
endif()

conan_cmake_configure(REQUIRES 
                        asio/1.18.1
                        spdlog/1.8.5
                        sol2/3.2.3
                      GENERATORS ${conan_generator})

foreach(config ${conan_configs_to_create})
    conan_cmake_autodetect(settings BUILD_TYPE ${config})
    conan_cmake_install(PATH_OR_REFERENCE .
                        BUILD missing
                        SETTINGS ${settings})
endforeach()

include(${CMAKE_BINARY_DIR}/${conan_cmake_file})
conan_basic_setup(TARGETS)

# windows defaults
add_definitions(-D_WIN32_WINNT=0x0A00)

# compiler
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if (MSVC)
    add_compile_options(/MP)  
endif()

# Include sub-projects.
add_subdirectory("promise-cpp")
add_subdirectory("XPlaneUDPClientCpp")
add_subdirectory("LuaEngine")
add_subdirectory("LuaTest")
add_subdirectory("AltFS")
