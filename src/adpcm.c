#include <stdint.h>

#include "adpcm.h"


static inline int get_high_nibble_signed(uint8_t n)
{
    return nibble_to_int[n >> 4];
}

static inline int get_low_nibble_signed(uint8_t n)
{
    return nibble_to_int[n & 0xF];
}

static inline int16_t clamp16(int n)
{
    if (n > 0x7FFF)
        return 0x7FFF;
    else if (n < -0x8000)
        return -0x8000;
    return n;
}

inline void decode_adpcm_block(uint8_t *frame, int16_t *outbuf, int16_t *_old, int16_t *_older)
{
    uint8_t coef_index = (*(frame) >> 4) & 0xF;
    uint8_t shift_factor = *(frame) & 0xF;

    // uint8_t flag = frame[1];  // Not used (usually for loops)

    if (coef_index > 5)
        coef_index = 4;
    if (shift_factor > 12)
        shift_factor = 9;

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
