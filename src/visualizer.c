#include <raylib.h>
#include <math.h>

#include "visualizer.h"

static float visualizer_find_peak(const float *magnitudes, size_t count) {
	float peak = 0.0f;
	for (size_t i = 0; i < count; ++i) {
		if (isfinite(magnitudes[i]) && magnitudes[i] > peak) {
			peak = magnitudes[i];
		}
	}
	return peak;
}

static int visualizer_bar_height(float magnitude, float peak,
								 int screen_height) {
	return (int)(magnitude / peak * (float)screen_height);
}

void visualizer_draw_bars(const float *magnitudes, size_t count,
						  int screen_width, int screen_height) {
	if (magnitudes == NULL || count == 0) {
		return;
	}

	const float peak = visualizer_find_peak(magnitudes, count);
	if (peak <= 0.0f) {
		return;
	}

	for (size_t i = 0; i < count; ++i) {
		const float magnitude = magnitudes[i];
		if (!isfinite(magnitude) || magnitude <= 0.0f) {
			continue;
		}

		const int x = (int)(i * (size_t)screen_width / count);
		const int next_x = (int)((i + 1) * (size_t)screen_width / count);
		const int bar_width = next_x > x ? next_x - x : 1;
		const int bar_height = visualizer_bar_height(magnitude, peak,
												 screen_height);
		DrawRectangle(x, screen_height - bar_height, bar_width, bar_height, SKYBLUE);
	}
}
