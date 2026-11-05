#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <dirent.h>
// #include "wav_parser.h"
#include "audio_dsp.h"
#include "esp_log.h"
#include "wav_parser.h"
#include <math.h>
#include "sd_card.h"

#define NUM_SAMPLES 512


#define PI 3.14159265358979323846
#define SAMPLE_RATE 22050.0f
#define FREQUENCY 1000.0f
#define AMPLITUDE 1000.0f

static const char *TAG = "AUDIO_TST";

static int16_t samples[NUM_SAMPLES];
static float window[NUM_SAMPLES];
static float windowed_samples[NUM_SAMPLES];
static float fft_buffer[NUM_SAMPLES * 2];
static float magnitudes[NUM_SAMPLES / 2 + 1];

static void analyse_wav_file(FILE *file){
    wav_header_t header;

    // 1. Parse WAV header

    wav_status_t status = wav_parse_header(file, &header);

    if (status!=WAV_OK){
        ESP_LOGE(TAG,"WAV parsing failed: %d",status);
        return;
    }

    // 2. Validate WAV format
    status = wav_validate_format(&header);

    if(status!=WAV_OK){
        ESP_LOGE(TAG,"Unsupported WAV format: %d", status);
        return;
    }

    ESP_LOGI(TAG,"WAV successfully parsed");
    ESP_LOGI(TAG,"Sample rate: %lu Hz", (unsigned long)header.sample_rate);
    ESP_LOGI(TAG,"Audio data: %lu bytes",(unsigned long)header.data_size);

    if(initialise_fft(NUM_SAMPLES)!=0){
        ESP_LOGE(TAG,"FFT initialisation failed");
        return;
    }

    // 4. Generate Hann window once
    generate_hann_window(window, NUM_SAMPLES);

    size_t total_samples = header.data_size/header.block_align;
    size_t total_frames = total_samples / NUM_SAMPLES;

    ESP_LOGI(TAG,"Total complete FFT frames: %u",(unsigned int)total_frames);

    // 6. Process each frame
    for(size_t frame=0;frame<total_frames;frame++){
        // Read 512 PCM samples
        size_t samples_read = fread(samples, sizeof(int16_t),NUM_SAMPLES,file);

        if(samples_read!=NUM_SAMPLES){
            ESP_LOGE(TAG,"Failed to read audio frame %u",(unsigned int)frame);
            break;
        }
        // Calculate RMS
        float rms = calculate_rms(samples, NUM_SAMPLES);
        // Apply Hann window
        apply_hann_window(samples,window,windowed_samples,NUM_SAMPLES);
        // Convert samples into comples FFT input
        prepare_fft_input(windowed_samples,fft_buffer,NUM_SAMPLES);

        // Perform FFT
        perform_fft(fft_buffer,NUM_SAMPLES);
        calculate_fft_magnitudes(fft_buffer, magnitudes,NUM_SAMPLES);

        // Find strongest frequency bin (excluding DC)

        size_t peak_bin =1;

        for(size_t i=2;i<NUM_SAMPLES/2;i++){
            if(magnitudes[i]>magnitudes[peak_bin]){
                peak_bin=i;
            }
        }
        // Convert bin number into frequency
        float frequency_resolution=(float)header.sample_rate/NUM_SAMPLES;

        float peak_frequency = peak_bin*frequency_resolution;

        if(frame%10==0){
            ESP_LOGI(TAG,"Frame %u | RMS %.2f | Peak 5.2f Hz",(unsigned int)frame,rms,peak_frequency);
        }
    }
    ESP_LOGI(TAG, "WAV analysis complete");
}

void app_main(void)
{

    ESP_LOGI(TAG,"Starting SD card test");
    // 1. Initialise and mount SD card
    esp_err_t ret = sd_card_init();

    if(ret!=ESP_OK){
        ESP_LOGE(TAG,"SD card initialisation failed");
        return;
    }

    // 2. List files on SD card
    DIR *dir = opendir(SD_CARD_MOUNT_POINT);

    if(dir==NULL){
        ESP_LOGE(TAG,"Could not open SD card directory");
        return;
    }

    struct dirent *entry;

    ESP_LOGI(TAG,"Files on SD card:");

    while ((entry=readdir(dir))!=NULL){
        ESP_LOGI(TAG," %s",entry->d_name);
    }

    closedir(dir);


    // 3. Open WAV file
    FILE *file = fopen("/sdcard/TEST_A~1.WAV","rb");

    if(file==NULL){
        ESP_LOGE(TAG,"Could not open WAV file");
        return;
    }

    ESP_LOGI(TAG,"Successfully opened WAV file!");

    // 4. Read the first 12 bytes

    wav_header_t header;

    wav_status_t status = wav_parse_header(file, &header);

    if(status!=WAV_OK){
        ESP_LOGE(TAG,"WAV parsing failed: %d", status);
        fclose(file);
        return;
    }

    ESP_LOGI(TAG,"WAV successfully parsed!");
    ESP_LOGI(TAG,"Sample rate: %lu Hz", (unsigned long)header.sample_rate);
    ESP_LOGI(TAG,"Channels: %u", header.num_channels);
    ESP_LOGI(TAG,"Bits per sample: %u", header.bits_per_sample);
    ESP_LOGI(TAG,"Audio data size: %lu bytes",(unsigned long)header.data_size);
    fclose(file);


}