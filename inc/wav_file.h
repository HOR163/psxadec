#ifndef WAV_FILE_H
#define WAV_FILE_H

#include <stdint.h>

#if defined(__GNUC__) || defined(__clang__)
#define STRUCT_PACK_START
#define STRUCT_PACK_END __attribute__((__packed__))
#elif defined(_MSC_VER)
#define STRUCT_PACK_START __pragma(pack(push, 1))
#define STRUCT_PACK_END __pragma(pack(pop))
#else
#warning Unknown compiler type, structures may contain padding!
#define STRUCT_PACK_START
#define STRUCT_PACK_END
#endif

STRUCT_PACK_START
typedef struct
{
    char riff_identifier[4]; // RIFF
    uint32_t file_size;      // the length that follows this (ie total file size - 8)
    char wave_identifier[4]; // WAVE

    // Data format chunk
    char fmt_identifier[4];   // fmt_ (_ is space)
    uint32_t fmt_chunk_size;  // following chunk size (here 2 + 2 + 4 + 4 + 2 + 2 = 16)
    uint16_t audio_format;    // 1 - PCM, 3 - IEEE 754 float
    uint16_t number_channels; // Number of channels
    uint32_t frequency;       // sample rate in hertz
    uint32_t byte_per_sec;    // frequency * byte_per_block
    uint16_t byte_per_block;  // number_channels * bits_per_sample / 8
    uint16_t bits_per_sample; // bits per sample

    // Data chunk
    char data_identifier[4]; // data
    uint32_t data_size;      // Size of the following data
} STRUCT_PACK_END wav_header;

void init_wav_header(wav_header *header);

#endif
