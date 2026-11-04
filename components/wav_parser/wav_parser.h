#ifndef WAV_PARSER_H
#define WAV_PARSER_H

#include <stdint.h>
#include <stdio.h>

typedef struct{
    uint16_t audio_format;
    uint16_t num_channels;

    uint32_t sample_rate;
    uint32_t byte_rate;

    uint16_t block_align;
    uint16_t bits_per_sample;

    uint32_t data_size;
    long data_offset;

}wav_header_t;

typedef enum{
    WAV_OK =0,
    WAV_ERR_INVALID_ARGUMENT,
    WAV_ERR_IO,
    WAV_ERR_INVALID_RIFF,
    WAV_ERR_INVALID_WAVE,
    WAV_ERR_CHUNK_NOT_FOUND,
    WAV_ERR_UNSUPPORTED_FORMAT
}wav_status_t;



wav_status_t wav_parse_header(
    FILE *file,
    wav_header_t *header
);

wav_status_t wav_validate_format(
    const wav_header_t *header
);

wav_status_t wav_validate_format(const wav_header_t *header);

#endif
