#include <raylib.h>
#include <stddef.h>

#include "analyzer.h"
#include "audio_buffer.h"
#include "audio_capture.h"
#include "visualizer.h"
#include <assert.h>
#include <stdio.h>
#include <errno.h>

enum {
    SAMPLE_RATE = 48000,
    CHANNELS = 2,
    BUFFER_CAPACITY = 1 << 15,  // samples 32K
    FFT_SIZE = 2048,
    WINDOW_WIDTH = 1024,
    WINDOW_HEIGHT = 512,
	FPS = 144,
};

int main(int argc, char** argv) {
		

	fprintf(stdout, "Compiled with Raylib Version %s\n", RAYLIB_VERSION);
	fprintf(stdout, "Compiled with libpipewire %s\n"
				"Linked with libpipewire %s\n",
				pw_get_headers_version(),
				pw_get_library_version());


	int err = 0;

    AudioBuffer buffer;
    err = audio_buffer_init(&buffer, BUFFER_CAPACITY);
	if(err){
		perror("Failed to init audio buffer");
		return errno;
	}


    AudioCapture capture;
    if (audio_capture_start(&capture, &buffer, SAMPLE_RATE, CHANNELS, &argc, &argv) < 0) { //spawns writer thread with pipewire, allocs capture stuff
        audio_buffer_destroy(&buffer);
		perror("Failed to start audio capture");
        return errno;
    }

		//testing start

	while(1){

	size_t total = 0;
    float samples_test[FFT_SIZE];
        size_t count = audio_buffer_read(&buffer, samples_test, FFT_SIZE);
        if (count == FFT_SIZE) {
			for(int i = 0; i< FFT_SIZE; i++){
				printf("sample %zu value %f\n", total, samples_test[i]);
				total++;
			}
        }
	}

		//testing end

    Analyzer analyzer;
    analyzer_init(&analyzer, FFT_SIZE);

    InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Real-Time Music Visualizer");

    float samples[FFT_SIZE];

	SetTargetFPS(FPS);
    while (!WindowShouldClose()) {
        size_t count = audio_buffer_read(&buffer, samples, FFT_SIZE);
        if (count == FFT_SIZE) {
            analyzer_process(&analyzer, samples, count);
        }
        visualizer_draw(analyzer.magnitudes, analyzer.fft_size / 2);
    }

    CloseWindow();
    analyzer_destroy(&analyzer);
    audio_capture_stop(&capture); //frees the capture stuff, pw_deinit
    audio_buffer_destroy(&buffer);
    return 0;
}
