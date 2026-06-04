add_library(adpcm STATIC adpcm.c adpcm.h)
target_include_directories(adpcm PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})