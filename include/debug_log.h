#ifndef RTMV_DEBUG_LOG_H
#define RTMV_DEBUG_LOG_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#include "audio_capture.h"

typedef struct {
    double squared_sample_sum;
    size_t finite_sample_count;
    size_t non_finite_sample_count;
    float minimum_sample;
    float maximum_sample;
    double last_report_time;
} DebugLog;

typedef struct {
    uint32_t process_callbacks;
    uint32_t dequeued_buffers;
    uint32_t buffers_with_samples;
    uint32_t sample_values_received;
    double squared_sample_sum;
    size_t finite_sample_count;
    size_t non_finite_sample_count;
    float minimum_sample;
    float maximum_sample;
} DebugLogReport;

void debug_log_init(DebugLog *log, double current_time);
void debug_log_runtime_versions(FILE *log_output, const char *raylib_version,
                                const char *pipewire_headers_version,
                                const char *pipewire_library_version);
void debug_log_record_samples(DebugLog *log, const float *samples, size_t count);
// Returns true and fills report when the reporting interval has elapsed.
bool debug_log_collect_report(DebugLog *log, AudioCapture *capture,
                              double current_time, DebugLogReport *report);
void debug_log_print_report(FILE *log_output, const DebugLogReport *report);

#endif
