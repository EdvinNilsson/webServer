cmake_minimum_required(VERSION 3.16)
project(webServer)

set(CMAKE_CXX_STANDARD 20)

find_package(Boost 1.83.0 CONFIG REQUIRED COMPONENTS filesystem coroutine regex)
include_directories(${Boost_INCLUDE_DIR})

add_library(webServer webServer.cpp webServer.h routeTypes.cpp routeTypes.h)

target_link_libraries(webServer PUBLIC ${Boost_LIBRARIES})
target_include_directories(webServer PUBLIC ${Boost_INCLUDE_DIRS})
