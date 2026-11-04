#include <stdio.h>
#include "wav_parser.h"

int main(void)
{
    FILE *file = fopen("components/wav_parser/Test_Audio.wav", "rb");

    if (file == NULL)
    {
        printf("Could not open WAV file\n");
        return 1;
    }

    wav_header_t header;

    wav_status_t status = wav_parse_header(file, &header);

    if (status != WAV_OK)
    {
        printf("WAV parsing failed: %d\n", status);
        fclose(file);
        return 1;
    }

    status = wav_validate_format(&header);

    if (status != WAV_OK)
    {
        printf("Unsupported WAV format: %d\n", status);
        fclose(file);
        return 1;
    }

    printf("WAV parsing successful!\n\n");

    printf("Audio format:    %u\n", header.audio_format);
    printf("Channels:        %u\n", header.num_channels);
    printf("Sample rate:     %lu Hz\n", (unsigned long)header.sample_rate);
    printf("Byte rate:       %lu\n", (unsigned long)header.byte_rate);
    printf("Block alignment: %u\n", header.block_align);
    printf("Bit depth:       %u\n", header.bits_per_sample);
    printf("Data size:       %lu bytes\n", (unsigned long)header.data_size);
    printf("Data offset:     %ld\n", header.data_offset);

    fclose(file);

    return 0;
}