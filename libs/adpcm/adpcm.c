#include <stdint.h>

#include "adpcm.h"

const float adpcm_coef[5][2] = {
    {0.0f, 0.0f},           // {   0.0        ,   0.0        },
    {0.9375f, 0.0f},        // {  60.0 / 64.0 ,   0.0        },
    {1.796875f, -0.8125f},  // { 115.0 / 64.0 , -52.0 / 64.0 },
    {1.53125f, -0.859375f}, // {  98.0 / 64.0 , -55.0 / 64.0 },
    {1.90625f, -0.9375f}    // { 122.0 / 64.0 , -60.0 / 64.0 }
};

const int nibble_to_int[] = {0, 1, 2, 3, 4, 5, 6, 7, -8, -7, -6, -5, -4, -3, -2, -1};

/**
 * Get the high nibble (4 MSB bits) of given byte
 *
 * @param n byte from which the high nibble will be extracted
 * @return high nibble
 */
static int get_high_nibble_signed(uint8_t n)
{
    return nibble_to_int[n >> 4];
}

/**
 * Get the low nibble (4 LSB bits) of given byte
 *
 * @param n byte from which the low nibble will be extracted
 * @return low nibble
 */
static int get_low_nibble_signed(uint8_t n)
{
    return nibble_to_int[n & 0xF];
}

/**
 * Clamp value to be in bounds of 16 bits
 *
 * @param n value to be clamped
 * @return clamped value
 */
static int16_t clamp16(int n)
{
    if (n > 0x7FFF)
    {
        return 0x7FFF;
    }
    else if (n < -0x8000)
    {
        return -0x8000;
    }
    return n;
}

void decode_adpcm_block(uint8_t *frame, int16_t *outbuf, int16_t *_old, int16_t *_older)
{
    uint8_t coef_index = (*(frame) >> 4) & 0xF;
    uint8_t shift_factor = *(frame) & 0xF;

    // uint8_t flag = frame[1];  // Not used (usually for loops)

    if (coef_index > 5)
    {
        coef_index = 4;
    }
    if (shift_factor > 12)
    {
        shift_factor = 9;
    }

    shift_factor = 20 - shift_factor;

    for (int index = 0; index < ADPCM_OUPUT_BUFFER_SIZE_16; index++)
    {
        int32_t sample = 0;
        uint8_t nibbles = *(frame + 0x02 + index / 2);

        sample = (index & 1 ? get_high_nibble_signed(nibbles) : get_low_nibble_signed(nibbles)) << shift_factor;
        sample += (adpcm_coef[coef_index][0] * (*_old) + adpcm_coef[coef_index][1] * (*_older)) * 256;
        sample >>= 8;

        *(outbuf + index) = clamp16(sample);

        *(_older) = *(_old);
        *(_old) = sample;
    }
}
