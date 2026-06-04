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

#define BYTES_PER_ADPCM_SAMPLE 16
#define BITS_PER_WAV_SAMPLE 16

struct timespec ts;
unsigned long long start_time;

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
static int check_for_incomplete_chunk(adpcm_parameters *params, uint32_t data_length)
{
    uint32_t incomplete_chunks;
    float chunk_length_in_seconds;
    uint32_t single_sample_size = params->channels * params->interleave;

    if (single_sample_size == 0)
    {
        return -EINVAL;
    }

    // How many incomplete chunks there are
    incomplete_chunks = (data_length % single_sample_size != 0);

    // Get the total amount of chunks, if not specified by user
    if (params->chunks == 0)
    {
        params->chunks = (uint32_t)(data_length / single_sample_size + incomplete_chunks);
    }

    // If incomplete chunks seen, then print a warning
    if (incomplete_chunks)
    {
        chunk_length_in_seconds = ((float)params->interleave / BYTES_PER_ADPCM_SAMPLE * ADPCM_OUPUT_BUFFER_SIZE_16) /
                                  (float)params->frequency;
        print_warning("Input file has non-complete chunks at the end of the file. These chunks "
                      "will be included in the conversion result, but there might be audio loss "
                      "(with mono the audio will be shorter; with more channels, one channel "
                      "will cut out before others)\n");

        float number_of_samples = (float)data_length / (float)single_sample_size;
        printf(" Maximum length affected: %.5f seconds.\n",
               (1 - (number_of_samples - (uint32_t)number_of_samples)) * chunk_length_in_seconds);
    }
    return 0;
}
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
static inline void interleave_channels(uint8_t channels, uint32_t buffer_length, int16_t *in_buffer,
                                       int16_t *out_buffer)
{
    if (channels == 1)
    {
        memcpy(out_buffer, in_buffer, buffer_length * sizeof(int16_t));
        return;
    }

    for (uint8_t channel = 0; channel < channels; channel++)
    {
        for (uint32_t index = 0; index < buffer_length; index++)
        {
            out_buffer[index * channels + channel] = in_buffer[index + buffer_length * channel];
        }
    }
}

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
int convert_file(adpcm_parameters *params)
{
    wav_header header;

    init_wav_header(&header);

    FILE *finput;
    FILE *foutput;

    finput = fopen(params->input_file, "rb");
    foutput = fopen(params->output_file, "wb");

    if (finput == NULL)
    {
        print_error("Couldn't open input file\n");
        return -ENOENT;
    }
    if (foutput == NULL)
    {
        fclose(finput);
        print_error("Couldn't open output file\n");
        return -ENOENT;
    }

    // Check for incomplete chunks, if found add one to chunks so that all the
    // data that can be read from the file is read.
    fseek(finput, 0, SEEK_END);
    check_for_incomplete_chunk(params, (uint32_t)ftell(finput) - params->offset);
    fseek(finput, (long)params->offset, SEEK_SET);

    // Write header
    header.number_channels = params->channels;
    header.frequency = params->frequency;
    header.bits_per_sample = BITS_PER_WAV_SAMPLE;
    header.byte_per_block = header.number_channels * header.bits_per_sample / 8;
    header.byte_per_sec = header.frequency * header.byte_per_block;
    header.data_size =
        (params->chunks * params->interleave * params->channels) / BITS_PER_WAV_SAMPLE * ADPCM_OUTPUT_BUFFER_SIZE_8;
    header.file_size = header.data_size + sizeof(header) - sizeof(header.riff_identifier);

    fwrite(&header, sizeof(header), 1, foutput);

    // Allocate memory for single chunk for each channel
    uint32_t in_chunk_length = params->interleave * params->channels;
    uint8_t *in_chunk = (uint8_t *)malloc(in_chunk_length);

    // Now create the output, 16 bytes in = 56 bytes out
    uint32_t out_channel_length = params->interleave / BYTES_PER_ADPCM_SAMPLE * ADPCM_OUPUT_BUFFER_SIZE_16;
    uint32_t out_chunk_length = out_channel_length * params->channels * sizeof(int16_t);
    int16_t *out_chunk = (int16_t *)malloc(out_chunk_length);

    // There are better in-place interleaving methods, but none that I could implement (didn't even try)
    int16_t *out_interleaved = (int16_t *)malloc(out_chunk_length);

    // Save the last values (in the beginning have to be 0)
    // Stores data as (int16_t)older, (int16_t)old
    int16_t *old = (int16_t *)calloc(params->channels * 2, sizeof(int16_t));

    for (uint32_t chunk = 0; chunk < params->chunks; chunk++)
    {
        // Read the data
        fread(in_chunk, sizeof(uint8_t), in_chunk_length, finput);

        for (uint32_t channel = 0; channel < params->channels; channel++)
        {
            for (uint32_t sample_index = 0; sample_index < params->interleave / BYTES_PER_ADPCM_SAMPLE; sample_index++)
            {
                uint32_t input_sample_offset = sample_index * BYTES_PER_ADPCM_SAMPLE + channel * params->interleave;
                uint32_t output_sample_offset =
                    sample_index * ADPCM_OUPUT_BUFFER_SIZE_16 + channel * out_channel_length;
                uint32_t old_offset = 2 * channel + 1;
                uint32_t older_offset = old_offset - 1;

                decode_adpcm_block(&in_chunk[input_sample_offset], &out_chunk[output_sample_offset], &old[old_offset],
                                   &old[older_offset]);
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
