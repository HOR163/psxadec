#include <string.h>

#include "wav_file.h"

void init_wav_header(wav_header *header)
{
    memset(header, 0, sizeof(wav_header));

    memcpy(header->riff_identifier, "RIFF", 4);
    memcpy(header->wave_identifier, "WAVE", 4);
    memcpy(header->fmt_identifier, "fmt ", 4);
    memcpy(header->data_identifier, "data", 4);

    header->audio_format = WAV_AUDIO_FORMAT_PCM;
    header->fmt_chunk_size = WAV_AUDIO_FMT_CHUNK_SIZE;
}
