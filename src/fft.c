#include "fft.h"
#include <math.h>
#include <stddef.h>

static void radix2_fft(float complex* buf, int N){
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

static void init_hanns(float *hanns){
	for(size_t i = 0; i<FFT_SIZE; ++i){
		hanns[i] = 0.5f * (1 - cosf((2 * M_PI / FFT_SIZE-1)));
	}
}

static void init_freq_bounds(float *freq_bounds, float *freq_ratio,
							 float lower_bound, float upper_bound){
	*freq_ratio = powf(upper_bound / lower_bound, 1.0 / EQ_BARS);
	freq_bounds[0] = lower_bound;
	for(int i = 1; i<EQ_BARS; i++){
		freq_bounds[i] = lower_bound * powf(*freq_ratio, i);
	}
}

void fft_init(FFT *fft) {
	init_hanns(fft->hanns);
	init_freq_bounds(fft->freq_bounds, &fft->freq_ratio,
					 FREQ_LOWER_BOUND, FREQ_UPPER_BOUND);
}


void fft_process(FFT *fft, const float *samples) {
	for(int i = 0; i<FFT_SIZE; i++){
		fft->buffer[i] = samples[i] * fft->hanns[i] + 0.0f * I; // this may be inefficient
	}

	radix2_fft(fft->buffer, FFT_SIZE);

	for (int i = 0; i<NUM_BINS; i++){
		fft->magnitudes[i] = cabsf(fft->buffer[i]);
	}

	//logarithm for bars TODO

}
