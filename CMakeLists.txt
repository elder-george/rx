cmake_minimum_required(VERSION 3.5)

project(rx CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
add_definitions( "/W3 /D_CRT_SECURE_NO_WARNINGS /EHsc /wd4005 /wd4996 /wd4309 /nologo" )

find_package(fmt CONFIG REQUIRED)


add_library(librx STATIC rx.cpp)

find_package(Catch2 REQUIRED)
add_executable(test-rx test_rx.cpp)
target_link_libraries(test-rx Catch2::Catch2 fmt::fmt fmt::fmt-header-only librx)

include(CTest)
include(Catch)
catch_discover_tests(test-rx)
