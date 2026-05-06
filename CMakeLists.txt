cmake_minimum_required(VERSION 4.1)
project(raylib_chess)

set(CMAKE_CXX_STANDARD 20)

find_package(raylib REQUIRED)

add_executable(raylib_chess
        main.cpp
        include/tinyfiledialogs.c)

target_include_directories(raylib_chess PRIVATE include)

target_link_libraries(raylib_chess raylib)