#ifndef RTMV_FFT_H
#define RTMV_FFT_H

#include <complex.h>

#include "config.h"

#define FFT_SIZE 8192
#define NUM_BINS (FFT_SIZE / 2 + 1)

typedef struct {
	float complex buffer[FFT_SIZE];
	float hanns[FFT_SIZE];
	float magnitudes[NUM_BINS];
	float freq_bounds[EQ_BARS];
	float freq_ratio;
} FFT;

void fft_init(FFT *fft);
void fft_process(FFT *fft, const float *samples);
#endif
