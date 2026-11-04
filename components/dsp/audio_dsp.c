#include "audio_dsp.h"
#include <math.h>
#include "esp_dsp.h"
#include "dsps_fft2r.h"
#define PI 3.14159265358979323846

float calculate_rms(const int16_t *samples, size_t num_samples){
    int64_t square_count =0;

    for(size_t i=0;i<num_samples;i++){
        int64_t product = (int64_t)samples[i] * samples[i];
        square_count+=product;
    }
    double mean_square = (double) square_count/(num_samples);
    return sqrt(mean_square);
}

void generate_hann_window(float *window, size_t num_samples){
    for(size_t i=0;i<num_samples;i++){
        window[i] = 0.5f*(1.0f-cos((2.0f*PI*i)/(num_samples -1)));
    }
}

void apply_hann_window(
    const int16_t *samples,
    const float *window,
    float *windowed_samples,
    size_t num_samples
){
    for(size_t i=0;i<num_samples;i++){
        windowed_samples[i]=samples[i]*window[i];
    }
}

void calculate_fft(
    const float *windowed_samples,
    float *fft_output,
    size_t num_samples
){   
}


void prepare_fft_input(
    const float *windowed_samples,
    float *fft_buffer,
    size_t num_samples
){
    for(size_t i=0;i<num_samples;i++){
        fft_buffer[2*i]=windowed_samples[i];
        fft_buffer[(2*i)+1]=0;
    }

}

int initialise_fft(size_t fft_size)
{
    return dsps_fft2r_init_fc32(NULL, fft_size);
}

void perform_fft(float *fft_buffer, size_t fft_size)
{
    dsps_fft2r_fc32(fft_buffer, fft_size);
    dsps_bit_rev_fc32_ansi(fft_buffer, fft_size);
    dsps_cplx2reC_fc32(fft_buffer, fft_size);
}


void calculate_fft_magnitudes(
    const float *fft_buffer,
    float *magnitudes,
    size_t fft_size
)
{
    for (size_t i = 0; i < fft_size / 2; i++)
    {
        float real = fft_buffer[2 * i];
        float imag = fft_buffer[2 * i + 1];

        magnitudes[i] = sqrtf(real * real + imag * imag);
    }
}