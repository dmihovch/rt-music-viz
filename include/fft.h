#ifndef RTMV_FFT_H
#define RTMV_FFT_H
#define FFT_SIZE 2048
#define NUM_BINS (FFT_SIZE / 2 + 1)
void fft_hanns_init();
void fft_process(float *samples, float *bars, int eq_bars);
#endif
