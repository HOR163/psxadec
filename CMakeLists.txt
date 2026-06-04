cmake_minimum_required(VERSION 3.10)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)

# For MSVC to create static executable with less dependecies
cmake_policy(SET CMP0091 NEW)
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

project(psxadec)

add_subdirectory(libs)

file(GLOB SOURCES src/*.c)
include_directories(${PROJECT_SOURCE_DIR}/inc)

add_executable(psxadec main.c ${SOURCES})

target_link_libraries(psxadec PRIVATE argparse adpcm)

if(MSVC)
  target_compile_options(psxadec PRIVATE /W4)
else()
  target_compile_options(psxadec PRIVATE -Wall -Wextra -Wpedantic -Wconversion)
endif()
