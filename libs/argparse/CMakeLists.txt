add_library(argparse STATIC argparse.c argparse.h)
target_include_directories(argparse PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})