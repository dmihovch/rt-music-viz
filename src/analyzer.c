#include "analyzer.h"

void analyzer_init(Analyzer *analyzer, size_t fft_size) {
    // TODO: allocate the Hann window and magnitude bins, and prepare any FFT
    // scratch state.
    (void)analyzer;
    (void)fft_size;
}

void analyzer_destroy(Analyzer *analyzer) {
    // TODO: free the window and magnitude bins.
    (void)analyzer;
}

void analyzer_process(Analyzer *analyzer, const float *samples, size_t count) {
    // TODO: window the samples, run the FFT, and write the magnitudes.
    (void)analyzer;
    (void)samples;
    (void)count;
}
