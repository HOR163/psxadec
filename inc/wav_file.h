#ifndef WAV_FILE_H
#define WAV_FILE_H

#include <stdint.h>

#define INIT_WAVE_HEADER(X)                                                                                            \
    wav_header X = {                                                                                                   \
        .riff_identifier = "RIFF", .wave_identifier = "WAVE", .fmt_identifier = "fmt ", .data_identifier = "data"}

typedef struct
{
    const char riff_identifier[4]; // RIFF
    uint32_t file_size;            // the length that follows this (ie total file size - 8)
    const char wave_identifier[4]; // WAVE

    // Data format chunk
    const char fmt_identifier[4]; // fmt_ (_ is space)
    uint32_t fmt_chunk_size;      // following chunk size (here 2 + 2 + 4 + 4 + 2 + 2 = 16)
    uint16_t audio_format;        // 1 - PCM, 3 - IEEE 754 float
    uint16_t number_channels;     // Number of channels
    uint32_t frequency;           // sample rate in hertz
    uint32_t byte_per_sec;        // frequency * byte_per_block
    uint16_t byte_per_block;      // number_channels * bits_per_sample / 8
    uint16_t bits_per_sample;     // bits per sample

    // Data chunk
    const char data_identifier[4]; // data
    uint32_t data_size;            // Size of the following data
} wav_header;

#endif
