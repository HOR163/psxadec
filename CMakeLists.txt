cmake_minimum_required(VERSION 3.12)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)

project(psxadec)

add_subdirectory(libs)

file(GLOB SOURCES src/*.c)
include_directories(${PROJECT_SOURCE_DIR}/inc)

add_executable(psxadec main.c ${SOURCES})

target_link_libraries(psxadec PRIVATE argparse adpcm)

if(MINGW OR BOTH_GNU_LIKE)
    add_compile_definitions(_GNU_SOURCE)
endif()

if(MSVC)
  target_compile_options(psxadec PRIVATE /W4)
else()
  target_compile_options(psxadec PRIVATE -Wall -Wextra -Wpedantic)
endif()
