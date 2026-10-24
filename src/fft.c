#include "fft.h"
#include <stdlib.h>
#include <math.h>
#include <complex.h>
#include <stdio.h>

static float complex fft_buf[FFT_SIZE];
static float hanns[FFT_SIZE];
static float magnitudes[NUM_BINS];


void radix2_fft(float complex* buf, int N){
	int j = 0;
	for(int i = 1; i<N; i++){
		int bit = N / 2;
		while(j & bit){
			j ^= bit;
			bit >>= 1;
		}
		j ^= bit;
		if(i < j){
			float complex tmp = buf[i];
			buf[i] = buf[j];
			buf[j] = tmp;
		}
	}
	
	int size = 2;
	while(size <= N){
		int half = size / 2;
		
		//twiddling my shit
		float angle = -2.0f * M_PI / size;
		float complex w_step = cosf(angle) + sinf(angle) * I;
		for(int start = 0; start<N; start+=size){
			int complex w = 1 + 0 * I;
			for(int k = 0; k<half; k++){
				int i = start+k;
				int j = i + half;
				float complex u = buf[i];
				float complex v = buf[j] * w;
				buf[i] = u + v;
				buf[j] = u - v;
				w *= w_step;
			}
			size *= 2;
		}
	}


}


//TODO implement bar lookup table
void fft_init() {
	for(size_t i = 0; i<FFT_SIZE; ++i){
		hanns[i] = 0.5f * (1 - cosf((2 * M_PI / FFT_SIZE-1)));
	}
}


void fft_process(float *samples, float* bars, int n_bars) {
	for(int i = 0; i<FFT_SIZE; i++){
		samples[i] *= hanns[i];
		fft_buf[i] = samples[i] + 0.0f * I; // this may be inefficient
	}

	radix2_fft(fft_buf, FFT_SIZE);

	for (int i = 0; i<NUM_BINS; i++){
		magnitudes[i] = cabsf(fft_buf[i]);
	}

	//logarithm for bars TODO

}
