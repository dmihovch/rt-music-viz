#ifndef RTMV_AUDIO_CAPTURE_H
#define RTMV_AUDIO_CAPTURE_H

#include <stdint.h>

#include "audio_buffer.h"
#include <pipewire/pipewire.h>


// Captures the default sink's monitor stream through the native PipeWire API
// and forwards the samples to an AudioBuffer.
typedef struct {
    struct pw_thread_loop *thread_loop;
    struct pw_stream *stream;
    AudioBuffer *buffer;
    uint32_t sample_rate;
    uint8_t channels;
} AudioCapture;

// Returns 0 on success, non-zero on failure.
int audio_capture_start(AudioCapture *capture, AudioBuffer *buffer,
                        uint32_t sample_rate, uint8_t channels, int* argc, char*** argv);
void audio_capture_stop(AudioCapture *capture);

#endif
