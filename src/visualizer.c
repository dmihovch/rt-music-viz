#include <raylib.h>
#include <math.h>

#include "visualizer.h"

void visualizer_draw_bars(const float *magnitudes, size_t count) {
	if (magnitudes == NULL || count == 0) {
		return;
	}

	const int screen_width = GetScreenWidth();
	const int screen_height = GetScreenHeight();
	float peak = 0.0f;
	for (size_t i = 0; i < count; ++i) {
		if (isfinite(magnitudes[i]) && magnitudes[i] > peak) {
			peak = magnitudes[i];
		}
	}
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
		const int bar_height = (int)(magnitude / peak * (float)screen_height);
		DrawRectangle(x, screen_height - bar_height, bar_width, bar_height, SKYBLUE);
	}
}
