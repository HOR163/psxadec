cmake_minimum_required(VERSION 3.10)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

project(psxadec)

add_subdirectory(libs)

file(GLOB SOURCES src/*.c)
include_directories(${PROJECT_SOURCE_DIR}/inc)

add_executable(psxadec main.c ${SOURCES})

target_link_libraries(psxadec PRIVATE argparse)
