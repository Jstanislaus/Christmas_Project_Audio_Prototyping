#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
// #include "wav_parser.h"
#include "audio_dsp.h"
#include "esp_log.h"

#define NUM_SAMPLES 512

#include <math.h>

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

void app_main(void)
{

    ESP_LOGI(TAG, "Starting DSP test");
    // Generate test audio samples

    int ret = initialise_fft(NUM_SAMPLES);

    if(ret!=0){
        ESP_LOGE(TAG, "FFT initialisation failed: %d",ret);
        return;
    }

    ESP_LOGI(TAG, "FFT initialised");

    for (size_t i = 0; i < NUM_SAMPLES; i++)
    {
        samples[i] = (int16_t)(
            AMPLITUDE * sinf(
                2.0f * PI * FREQUENCY * i / SAMPLE_RATE
            )
        );
    }

    // Calculate RMS
    float rms = calculate_rms(samples, NUM_SAMPLES);

    ESP_LOGI(TAG, "RMS: %.2f\n", (double)rms);


    generate_hann_window(window, NUM_SAMPLES);

    apply_hann_window(
    samples,
    window,
    windowed_samples,
    NUM_SAMPLES
    );

    // printf("First coefficient: %.6f\n", window[0]);
    // printf("Middle coefficient: %.6f\n", window[NUM_SAMPLES / 2]);
    // printf("Last coefficient: %.6f\n", window[NUM_SAMPLES - 1]);

    // printf("First windowed sample: %.2f\n", windowed_samples[0]);
    // printf("Middle windowed sample: %.2f\n", windowed_samples[NUM_SAMPLES / 2]);
    // printf("Last windowed sample: %.2f\n", windowed_samples[NUM_SAMPLES - 1]);

    prepare_fft_input(
        windowed_samples,
        fft_buffer,
        NUM_SAMPLES
    );

    // printf("First real: %.2f\n", fft_buffer[0]);
    // printf("First imaginary: %.2f\n", fft_buffer[1]);

    // printf("Second real: %.2f\n", fft_buffer[2]);
    // printf("Second imaginary: %.2f\n", fft_buffer[3]);

    // printf("Third real: %.2f\n", fft_buffer[4]);
    // printf("Third imaginary: %.2f\n", fft_buffer[5]);


    perform_fft(fft_buffer, NUM_SAMPLES);
    calculate_fft_magnitudes(
    fft_buffer,
    magnitudes,
    NUM_SAMPLES
    );

    //9. Find strongest frequency bin
    int max_bin = 0;
    float max_magnitude = 0.0f;

    for(int i=1; i<=NUM_SAMPLES/2;i++){
        if (magnitudes[i]>max_magnitude){
            max_magnitude = magnitudes[i];
            max_bin=i;
        }
    }

    float dominant_frequency = (float)max_bin * SAMPLE_RATE / NUM_SAMPLES;

    ESP_LOGI(TAG, "Dominant bin: %d", max_bin);
    ESP_LOGI(TAG, "Dominant frequency: %.2f Hz", (double)dominant_frequency);
    ESP_LOGI(TAG, "Magnitude: %.2f", (double)max_magnitude);


    for (int i = 0; i <= NUM_SAMPLES / 2; i++)
    {
        float frequency = (float)i * SAMPLE_RATE / NUM_SAMPLES;

        ESP_LOGI(TAG,"Bin %d: %.2f Hz, Magnitude: %.2f\n",
            i, frequency, magnitudes[i]);
    }


}