#ifndef RTMV_AUDIO_BUFFER_H
#define RTMV_AUDIO_BUFFER_H

#include <stdatomic.h>
#include <stddef.h>

// Single-producer/single-consumer ring buffer of interleaved float samples.
// The PipeWire capture thread writes; the main thread reads.
typedef struct {
    float *samples;
    size_t capacity;  // power of two, so index wrapping is a mask
    _Atomic size_t windex;
    _Atomic size_t rindex;
} AudioBuffer;

int audio_buffer_init(AudioBuffer *buffer, size_t capacity);
void audio_buffer_destroy(AudioBuffer *buffer);

// Returns the number of samples written, which may be less than count when
// the buffer is full.
void audio_buffer_write(AudioBuffer *buffer, const float *samples, size_t count);

// Returns the number of samples read, which may be less than count when the
// buffer holds fewer.
size_t audio_buffer_read(AudioBuffer *buffer, float *samples, size_t count);

size_t audio_buffer_available(const AudioBuffer *buffer);

#endif
