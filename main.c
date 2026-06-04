/**
 * @file main.c
 * @author Hans Oliver Raudvere (hasse2507@gmail.com)
 * @brief CLI tool for converting PS ADPCM to wav
 * @version 1.1.0
 * @date 2026-06-04
 *
 * @copyright Copyright (c) 2026 Hans Oliver Raudvere
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "adpcm.h"
#include "args.h"
#include "print_conv.h"
#include "wav_file.h"

#define SEC_TO_NS(sec) ((sec) * 1000000000)

struct timespec ts;
unsigned long long start_time;

/**
 * Convert 4-bit ADPCM file to wav file
 *
 * @warning parameters in the adpcm_parameters struct won't be validated
 * in this function. Data validation should occur before calling the function.
 * This function also may change values stored in the struct.
 *
 * @param params adpcm_parameters struct with all necessary parameters
 * @retval 0 on success
 * @retval -ENOENT if input or output file could not be opened
 */
int convert_file(adpcm_parameters *params);

/**
 * Check if the number of chunks fit perfectly into the file
 *
 * If the chunks don't fit prefectly inside the given size, warn user about it
 * and increase the number of chunks by one, so that all data is read.
 *
 * @warning this function may change variable values in the given struct
 *
 * @param params pointer to adpcm_parameters struct
 * @param data_length sum of the length of all chunks
 * @retval 0 on success
 * @retval -EINVAL if number of channels or interleav is 0
 */
int check_for_incomplete_chunk(adpcm_parameters *params, long data_length);

/**
 * Interleave given channels
 *
 * Given a buffer that includes n (channels) not interleaved channels saved
 * subsequently (ie |channel1|channel2|channel3|), interleave them into another
 * buffer (out_buffer) without changing the input buffer. The size of each
 * channel is specified in buffer_length
 *
 * Example:
 *   channels = 3
 *   buffer_length = 3
 *   in_buffer = {1, 2, 3, a, b, c, x, y, z}
 *   then out_buffer will become {1, a, x, 2, b, y, 3, c, z}
 *
 * @param channels number of channels inside the in_buffer and number of
 * channels to be interleaved
 * @param buffer_length length of single channel, total buffer size is
 * buffer_length * channels
 * @param in_buffer buffer of `channel` channels, each having a size of
 * `buffer_length`
 * @param out_buffer buffer where interleaved data is saved, has to be the same
 * size as in_buffer
 */
void interleave_channels(int channels, int buffer_length, int16_t *in_buffer, int16_t *out_buffer);

int main(int argc, const char **argv)
{
    start_time = timespec_get(&ts, TIME_UTC) != 0 ? SEC_TO_NS((uint64_t)ts.tv_sec) + (uint64_t)ts.tv_nsec : 0;

    int ret;
    adpcm_parameters *parameters = (adpcm_parameters *)malloc(sizeof(adpcm_parameters));
    ret = read_param(argc, argv, parameters);

    if (ret)
    {
        return ret;
    }

    ret = convert_file(parameters);

    if (timespec_get(&ts, TIME_UTC) != 0)
    {
        printf("Finished in %f ms\n",
               (double)(SEC_TO_NS((uint64_t)ts.tv_sec) + (uint64_t)ts.tv_nsec - start_time) / 1000000);
    }

    return ret;
}

int convert_file(adpcm_parameters *params)
{
    wav_header header;

    init_wav_header(&header);

    FILE *finput;
    FILE *foutput;

    finput = fopen(params->input_file, "rb");
    if (finput == NULL)
    {
        print_error("Couldn't open input file\n");
        return -ENOENT;
    }

    fseek(finput, 0, SEEK_END);
    // Check for incomplete chunks, if found add one to chunks so that all the
    // data that can be read from the file is read.
    check_for_incomplete_chunk(params, ftell(finput) - params->offset);
    fseek(finput, params->offset, SEEK_SET);

    foutput = fopen(params->output_file, "wb");
    if (foutput == NULL)
    {
        fclose(finput);
        print_error("Couldn't open output file\n");
        return -ENOENT;
    }

    // Write header
    header.fmt_chunk_size = 16;
    header.audio_format = 1; // PCM
    header.number_channels = params->channels;
    header.frequency = params->frequency;
    header.bits_per_sample = 16;
    header.byte_per_block = header.number_channels * header.bits_per_sample / 8;
    header.byte_per_sec = header.frequency * header.byte_per_block;
    header.data_size = (params->chunks * params->interleave * params->channels) / 16 * 56;
    header.file_size = header.data_size + sizeof(header) - 4; // 4 = RIFF

    fwrite(&header, sizeof(header), 1, foutput);

    // Allocate memory for single chunk for each channel
    int in_chunk_length = params->interleave * params->channels * sizeof(uint8_t);
    uint8_t *in_chunk = (uint8_t *)malloc(in_chunk_length);

    // Now create the output, 16 bytes in = 56 bytes out
    int out_channel_length = params->interleave / 16 * 28; // Length for single channel (in 16 bit ints)
    int out_chunk_length = out_channel_length * params->channels * sizeof(int16_t);
    int16_t *out_chunk = (int16_t *)malloc(out_chunk_length);

    // There are better in-place interleaving methods, but none that I could implement (didn't even try)
    int16_t *out_interleaved = (int16_t *)malloc(out_chunk_length);

    // Save the last values (in the beginning have to be 0)
    // Stores data as (int16_t)older, (int16_t)old
    int16_t *old = (int16_t *)calloc(params->channels * 2, sizeof(int16_t));

    for (int chunk = 0; chunk < params->chunks; chunk++)
    {
        // Read the data
        fread(in_chunk, sizeof(uint8_t), in_chunk_length, finput);

        for (int channel = 0; channel < params->channels; channel++)
        {
            for (int sample_index = 0; sample_index < params->interleave / 16; sample_index++)
            {
                decode_adpcm_block((uint8_t *)(in_chunk + sample_index * 16 + channel * params->interleave),
                                   (int16_t *)(out_chunk + sample_index * 28 + channel * out_channel_length),
                                   (int16_t *)(old + 2 * channel + 1), (int16_t *)(old + 2 * channel));
            }
        }
        interleave_channels(params->channels, out_channel_length, out_chunk, out_interleaved);
        fwrite(out_interleaved, sizeof(int16_t), out_channel_length * params->channels, foutput);
    }

    free(in_chunk);
    free(out_chunk);
    free(out_interleaved);
    free(old);

    fclose(finput);
    fclose(foutput);

    return 0;
}

int check_for_incomplete_chunk(adpcm_parameters *params, long data_length)
{
    int incomplete_chunks;
    float chunk_length_in_seconds;
    int single_sample_size = params->channels * params->interleave;

    if (single_sample_size == 0)
    {
        return -EINVAL;
    }

    incomplete_chunks = (data_length % single_sample_size != 0);
    if (params->chunks == 0)
    {
        params->chunks = (int)(data_length / single_sample_size + incomplete_chunks);
    }

    if (incomplete_chunks)
    {
        chunk_length_in_seconds = (float)(params->interleave / 16 * 28) / params->frequency;
        print_warning("Input file has non-complete chunks at the end of the file. These chunks "
                      "will be included in the conversion result, but there might be audio loss "
                      "(with mono the audio will be shorter; with more channels, one channel "
                      "will cut out before others)\n");
        printf(" Maximum length affected: %.5f seconds.\n",
               (1 - (float)((float)data_length / single_sample_size - (int)(data_length / single_sample_size))) *
                   chunk_length_in_seconds);
    }
    return 0;
}

inline void interleave_channels(int channels, int buffer_length, int16_t *in_buffer, int16_t *out_buffer)
{
    if (channels == 1)
    {
        memcpy(out_buffer, in_buffer, buffer_length * sizeof(int16_t));
        return;
    }

    for (int channel = 0; channel < channels; channel++)
    {
        for (int index = 0; index < buffer_length; index++)
        {
            memcpy((int16_t *)(out_buffer + index * channels + channel),
                   (int16_t *)(in_buffer + index + buffer_length * channel), sizeof(int16_t));
        }
    }
}
