# CMakeList.txt : CMake project for AlfBFF, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.15)

project(LuaTest)

# Add source to this project's executable.
add_executable (${PROJECT_NAME})

target_sources(${PROJECT_NAME} 
    PRIVATE 
        "Src/LuaTest.cpp"
	    "Src/Pch.h")


  target_link_libraries(${PROJECT_NAME}  PRIVATE promise-cpp LuaEngine XPlaneUDPClientCpp spdlog::spdlog Boost::boost)

  target_precompile_headers(${PROJECT_NAME} PRIVATE "Src/Pch.h")

