cmake_minimum_required(VERSION 3.29)
project(http_server)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_TOOLCHAIN_FILE "C:/Users/User/Downloads/vcpkg/scripts/buildsystems/vcpkg.cmake")

SET(CMAKE_INCLUDE_PATH ${CMAKE_INCLUDE_PATH} "C:/Users/User/Downloads/vcpkg/installed/x64-windows/include")
SET(CMAKE_LIBRARY_PATH ${CMAKE_LIBRARY_PATH} "C:/Users/User/Downloads/vcpkg/installed/x64-windows/bin")

add_executable(http_server main.cpp
        Response.cpp
        ResponseTypes.hpp
        ResponseTypes.cpp
        util.hpp
        util.cpp
        ServerStats.hpp
        ServerStats.cpp)

find_package(Boost REQUIRED COMPONENTS system)
target_link_libraries(http_server PRIVATE Boost::system)

if(MINGW)
    target_link_libraries(http_server PRIVATE ws2_32 wsock32)
endif()