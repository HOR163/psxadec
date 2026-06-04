#ifndef ADPCM_H
#define ADPCM_H

#include <stdint.h>

#define ADPCM_OUTPUT_BUFFER_SIZE_8 56
#define ADPCM_OUPUT_BUFFER_SIZE_16 28

/**
 * Decode a 16-byte 4-bit ADPCM block
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
