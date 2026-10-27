#ifndef RTMV_VISUALIZER_H
#define RTMV_VISUALIZER_H

#include <stddef.h>

// Draws magnitude bars inside an active Raylib drawing frame.
void visualizer_draw_bars(const float *magnitudes, size_t count,
                          int screen_width, int screen_height);

#endif
