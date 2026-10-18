#include <raylib.h>
#include <stddef.h>
#include "analyzer.h"
#include "audio_buffer.h"
#include "audio_capture.h"
#include "debug_log.h"
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
	FPS = 60,
};

int main(int argc, char** argv) {
		

	debug_log_runtime_versions(RAYLIB_VERSION, pw_get_headers_version(),
						   pw_get_library_version());


	int err = 0;

    AudioBuffer buffer;
    err = audio_buffer_init(&buffer, BUFFER_CAPACITY);
	if(err){
		perror("Failed to init audio buffer");
		return errno;
	}


    AudioCapture capture;
    if (audio_capture_start(&capture, &buffer, SAMPLE_RATE, CHANNELS, &argc, &argv) != 0) { //spawns writer thread with pipewire, allocs capture stuff
        audio_buffer_destroy(&buffer);
		perror("Failed to start PipeWire audio capture");
        return errno;
    }

    Analyzer analyzer = {0};
    analyzer_init(&analyzer, FFT_SIZE);

	InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Real-Time Music Visualizer");
	SetExitKey(KEY_Q);

    float samples[FFT_SIZE];
	DebugLog debug_log;
	debug_log_init(&debug_log, GetTime());

	SetTargetFPS(FPS);
	while (!WindowShouldClose()) {
        size_t count = audio_buffer_read(&buffer, samples, FFT_SIZE);
		debug_log_record_samples(&debug_log, samples, count);

        if (count == FFT_SIZE) {
            analyzer_process(&analyzer, samples, count);
        }
		BeginDrawing();
		ClearBackground(BLACK);
		visualizer_draw_bars(analyzer.magnitudes, analyzer.fft_size / 2);
		EndDrawing();

		debug_log_report_if_due(&debug_log, &capture, GetTime());
    }

    CloseWindow();
    analyzer_destroy(&analyzer);
    audio_capture_stop(&capture); //frees the capture stuff, pw_deinit
    audio_buffer_destroy(&buffer);
    return 0;
}
