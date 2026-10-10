#ifndef RTMV_VISUALIZER_H
#define RTMV_VISUALIZER_H

#include <stddef.h>

// Draws the magnitude spectrum as bars with raylib.
void visualizer_draw(const float *magnitudes, size_t count);

#endif
