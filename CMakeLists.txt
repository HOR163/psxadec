cmake_minimum_required(VERSION 3.10)

if (WIN32)
    message("Compiling for Windows")
    set(CMAKE_C_COMPILER C:/msys64/ucrt64/bin/gcc.exe)
    set(CMAKE_CXX_COMPILER C:/msys64/ucrt64/bin/g++.exe)
else()
    message("Compiling for Linux")
    set(CMAKE_C_COMPILER /usr/bin/gcc)
    set(CMAKE_CXX_COMPILER /usr/bin/g++)
endif()

project(psxadec)

file(GLOB SOURCES src/*.c)

include_directories(${PROJECT_SOURCE_DIR}/inc)

add_executable(psxadec main.c ${SOURCES})
