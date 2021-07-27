cmake_minimum_required(VERSION 3.19)
project(game1)

set(CMAKE_CXX_STANDARD 14)
find_package(Curses REQUIRED)
include_directories(${CURSES_INCLUDE_DIR})

add_executable(game1 main.cpp)
target_link_libraries(game1 ${CURSES_LIBRARIES})
