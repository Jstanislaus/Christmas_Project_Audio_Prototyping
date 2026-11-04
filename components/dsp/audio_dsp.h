#ifndef AUDIO_DSP_H
#define AUDIO_DSP_H

#include <stdint.h>
#include <stddef.h>
// #include "esp_err.h"

int initialise_fft(size_t fft_size);

float calculate_rms(const int16_t *samples, size_t num_samples);
void generate_hann_window(float *window, size_t num_samples);

void apply_hann_window(
    const int16_t *samples,
    const float *window,
    float *windowed_samples,
    size_t num_samples
);


void calculate_fft(
    const float *windowed_samples,
    float *fft_output,
    size_t num_samples
);

void prepare_fft_input(
    const float *windowed_samples,
    float *fft_buffer,
    size_t num_samples
);

void perform_fft(float *fft_buffer, size_t fft_size);

void calculate_fft_magnitudes(
    const float *fft_buffer,
    float *magnitudes,
    size_t fft_size
);

#endif