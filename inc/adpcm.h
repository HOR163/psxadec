#ifndef ADPCM_H
#define ADPCM_H

#include <stdint.h>

#define ADPCM_OUTPUT_BUFFER_SIZE_8 56
#define ADPCM_OUPUT_BUFFER_SIZE_16 28

/**
 * @see https://github.com/vgmstream/vgmstream/blob/7b0c835cacb7717a142a77b3af6d9466b83ce7ee/src/coding/psx_decoder.c#L5C1-L10C72
 */
static const float adpcm_coef[5][2] = {
    {0.0f, 0.0f},           // {   0.0        ,   0.0        },
    {0.9375f, 0.0f},        // {  60.0 / 64.0 ,   0.0        },
    {1.796875f, -0.8125f},  // { 115.0 / 64.0 , -52.0 / 64.0 },
    {1.53125f, -0.859375f}, // {  98.0 / 64.0 , -55.0 / 64.0 },
    {1.90625f, -0.9375f}    // { 122.0 / 64.0 , -60.0 / 64.0 }
};

/**
 * @see https://github.com/vgmstream/vgmstream/blob/7b0c835cacb7717a142a77b3af6d9466b83ce7ee/src/util/reader_get_nibbles.h#L8C1-L8C74
 */
static const int nibble_to_int[] = {0, 1, 2, 3, 4, 5, 6, 7, -8, -7, -6, -5, -4, -3, -2, -1};

/**
 * Get the high nibble (4 MSB bits) of given byte
 *
 * @see https://github.com/vgmstream/vgmstream/blob/7b0c835cacb7717a142a77b3af6d9466b83ce7ee/src/util/reader_get_nibbles.h#L15-L18
 *
 * @param n byte from which the high nibble will be extracted
 * @return high nibble
 */
static inline int get_high_nibble_signed(uint8_t n);

/**
 * Get the low nibble (4 LSB bits) of given byte
 *
 * @see https://github.com/vgmstream/vgmstream/blob/7b0c835cacb7717a142a77b3af6d9466b83ce7ee/src/util/reader_get_nibbles.h#L20-L23
 *
 * @param n byte from which the low nibble will be extracted
 * @return low nibble
 */
static inline int get_low_nibble_signed(uint8_t n);

/**
 * Clamp value to be in bounds of 16 bits
 *
 * @see https://github.com/vgmstream/vgmstream/blob/7b0c835cacb7717a142a77b3af6d9466b83ce7ee/src/coding/libs/relic_lib.c#L418-L422
 *
 * @param n value to be clamped
 * @return clamped value
 */
static inline int16_t clamp16(int n);

/**
 * Decode a 16-byte 4-bit ADPCM block
 *
 * @see https://github.com/vgmstream/vgmstream/blob/7b0c835cacb7717a142a77b3af6d9466b83ce7ee/src/coding/psx_decoder.c#L58C1-L120C2
 *
 * @param frame 16 byte ADPCM block buffer
 * @param outbuf 28 int16_t buffer, where decoded block will be saved
 * @param _old pointer to previous sample value (will be updated inside the
 * function)
 * @param _older pointer to previous previous sample value (will be updated
 * inside the function)
 */
void decode_adpcm_block(uint8_t *frame, int16_t *outbuf, int16_t *_old, int16_t *_older);

#endif