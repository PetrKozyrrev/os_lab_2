cmake_minimum_required(VERSION 3.10)
project(os_lab_2)

set(CMAKE_CXX_STANDARD 17)
SET(CMAKE_CXX_FLAGS_DEBUG "... -lpthread")
SET(CMAKE_CXX_FLAGS_RELEASE "... -lpthread")

add_executable(app main.cpp bigint.h bigint.cpp)