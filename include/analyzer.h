#ifndef RTMV_ANALYZER_H
#define RTMV_ANALYZER_H

#include <stddef.h>

// Computes a magnitude spectrum from a window of samples. The FFT is
// hand-rolled; no external FFT library is used.
typedef struct {
    size_t fft_size;
    float *window;      // Hann window, length fft_size
    float *magnitudes;  // fft_size / 2 bins
} Analyzer;

void analyzer_init(Analyzer *analyzer, size_t fft_size);
void analyzer_destroy(Analyzer *analyzer);

// Fills analyzer->magnitudes from count samples. count must equal fft_size.
void analyzer_process(Analyzer *analyzer, const float *samples, size_t count);

#endif
