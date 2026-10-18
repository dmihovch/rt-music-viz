#ifndef RTMV_DEBUG_LOG_H
#define RTMV_DEBUG_LOG_H

#include <stddef.h>

#include "audio_capture.h"

typedef struct {
    double squared_sample_sum;
    size_t finite_sample_count;
    size_t non_finite_sample_count;
    float minimum_sample;
    float maximum_sample;
    double last_report_time;
} DebugLog;

void debug_log_init(DebugLog *log, double current_time);
void debug_log_runtime_versions(const char *raylib_version,
                                const char *pipewire_headers_version,
                                const char *pipewire_library_version);
void debug_log_record_samples(DebugLog *log, const float *samples, size_t count);
void debug_log_report_if_due(DebugLog *log, AudioCapture *capture,
                             double current_time);

#endif
