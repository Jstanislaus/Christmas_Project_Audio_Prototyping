#include "wav_parser.h"
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <limits.h>


static uint16_t read_le16(const uint8_t *buffer){
    return (uint16_t)((uint16_t)buffer[0] | ((uint16_t)buffer[1]<<8));
}

static uint32_t read_le32(const uint8_t *buffer){
    return (uint32_t)((uint32_t)buffer[0] | ((uint32_t)buffer[1]<<8) | ((uint32_t)buffer[2] << 16) | ((uint32_t)buffer[3] << 24));
}

static int read_exact(
    FILE *file,
    uint8_t *buffer,
    size_t length
){
    return fread(buffer,1,length,file)==length;
}



wav_status_t wav_parse_header(
    FILE *file,
    wav_header_t *header
){

    if (file == NULL || header == NULL)
{
    return WAV_ERR_INVALID_ARGUMENT;
}

    memset(header, 0, sizeof(*header));

    if (fseek(file, 0, SEEK_SET) != 0)
    return WAV_ERR_IO;

    uint8_t buffer[4];

    if(!read_exact(file,buffer,4)){
        return WAV_ERR_IO;
    }

    if (buffer[0]!='R'||
        buffer[1]!='I'||
        buffer[2]!='F'||
        buffer[3]!='F'){
            return WAV_ERR_INVALID_RIFF;
        }


    //read riff size
    if(!read_exact(file,buffer,4)){
        return WAV_ERR_IO;
    }

    uint32_t riff_size = read_le32(buffer);
    printf("RIFF size: %lu\n", (unsigned long)riff_size);
    if (riff_size<4)
        return WAV_ERR_INVALID_RIFF;

    if(riff_size>LONG_MAX -8)
        return WAV_ERR_INVALID_RIFF;
    long riff_end = 8L + (long)riff_size;
    printf("RIFF end: %ld\n", riff_end);
    //read wave identifier
    if(!read_exact(file, buffer, 4)){
        return WAV_ERR_IO;
    }

    if (buffer[0]!='W'||
        buffer[1]!='A'||
        buffer[2]!='V'||
        buffer[3]!='E'){
            return WAV_ERR_INVALID_WAVE;
        }



    // find fmt chunk

    int found_fmt=0;
    int found_data=0;

    while(!found_fmt||!found_data){
        long chunk_header_offset = ftell(file);
        if(chunk_header_offset<0)
            return WAV_ERR_IO;

        if(riff_end - chunk_header_offset < 8)
            return WAV_ERR_CHUNK_NOT_FOUND;

        if(!read_exact(file,buffer,4)){
            return WAV_ERR_IO;
        }

        uint8_t size_buffer[4];

        if (!read_exact(file,size_buffer,4)){
            return WAV_ERR_IO;
        }

        uint32_t chunk_size = read_le32(size_buffer);

        long data_offset = ftell(file);
        if (data_offset<0)
            return WAV_ERR_IO;


        long remaining = riff_end - data_offset;

        printf("\nChunk identifier: %.4s\n", buffer);
        printf("Chunk size: %lu\n", (unsigned long)chunk_size);
        printf("Data offset: %ld\n", data_offset);
        printf("Remaining bytes: %ld\n", remaining);
        printf("RIFF end: %ld\n", riff_end);

        if (remaining < 0)
        {
            printf("ERROR: Data offset exceeds RIFF boundary\n");
            return WAV_ERR_INVALID_RIFF;
        }

        printf("Checking chunk boundary: %llu > %llu\n",
            (unsigned long long)chunk_size + (chunk_size % 2),
            (unsigned long long)remaining);

        if ((uint64_t)chunk_size + (chunk_size % 2) > (uint64_t)remaining)
        {
            printf("ERROR: Chunk extends beyond RIFF boundary\n");
            return WAV_ERR_INVALID_RIFF;
        }

        long next_chunk_offset = data_offset + (long)chunk_size + (long)(chunk_size %2);

        if(memcmp(buffer,"fmt ",4)==0){
            if(chunk_size<16){
                return WAV_ERR_UNSUPPORTED_FORMAT;
            }
            uint8_t fmt_buffer[16];

            if(!read_exact(file,fmt_buffer,16)){
                return WAV_ERR_IO;
            }
            
            header->audio_format    = read_le16(&fmt_buffer[0]);
            header->num_channels    = read_le16(&fmt_buffer[2]);
            header->sample_rate     = read_le32(&fmt_buffer[4]);
            header->byte_rate       = read_le32(&fmt_buffer[8]);
            header->block_align     = read_le16(&fmt_buffer[12]);
            header->bits_per_sample = read_le16(&fmt_buffer[14]);

            found_fmt =1;

        }

        else if (memcmp(buffer,"data",4)==0){
            header->data_size = chunk_size;
            header->data_offset = data_offset;

            found_data=1;
        }
        printf("Next chunk offset: %ld\n", next_chunk_offset);
        if (fseek(file, next_chunk_offset, SEEK_SET)!=0){
            printf("ERROR: fseek failed\n");
            return WAV_ERR_IO;
        }
        printf("Successfully moved to offset: %ld\n", ftell(file));

    }

    if (header->block_align == 0)
    return WAV_ERR_INVALID_RIFF;

    if (header->data_size % header->block_align != 0)
        return WAV_ERR_INVALID_RIFF;

    if (fseek(file, header->data_offset, SEEK_SET) != 0)
        return WAV_ERR_IO;

    //find data chunk


    return WAV_OK;
}


wav_status_t wav_validate_format(const wav_header_t *header){
    if(header==NULL)
        return WAV_ERR_INVALID_ARGUMENT;

    if(header->audio_format !=1)
        return WAV_ERR_UNSUPPORTED_FORMAT;

    if(header->num_channels !=1)
        return WAV_ERR_UNSUPPORTED_FORMAT;
    
    if(header->sample_rate != 22050)
        return WAV_ERR_UNSUPPORTED_FORMAT;

    if(header->bits_per_sample != 16)
        return WAV_ERR_UNSUPPORTED_FORMAT;

    if(header->block_align != 2)
        return WAV_ERR_UNSUPPORTED_FORMAT;

    if(header->byte_rate != 44100)
        return WAV_ERR_UNSUPPORTED_FORMAT;

    return WAV_OK;
}