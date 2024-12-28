cmake_minimum_required(VERSION 3.16)
project(example-server)

set(CMAKE_CXX_STANDARD 20)

add_subdirectory(webServer)

find_package(Boost CONFIG REQUIRED COMPONENTS url)
include_directories(${Boost_INCLUDE_DIR})

add_executable(example-server main.cpp webServer)

target_link_libraries(example-server webServer ${Boost_LIBRARIES})
target_include_directories(example-server PRIVATE ${Boost_INCLUDE_DIRS})
