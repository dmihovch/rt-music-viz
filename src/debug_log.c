#include "debug_log.h"

#include <math.h>
#include <stdio.h>
#include <stdatomic.h>

enum {
    DEBUG_REPORT_INTERVAL_SECONDS = 1,
};

void debug_log_init(DebugLog *log, double current_time) {
    *log = (DebugLog){
        .last_report_time = current_time,
    };
}

void debug_log_runtime_versions(const char *raylib_version,
                                const char *pipewire_headers_version,
                                const char *pipewire_library_version) {
    fprintf(stdout, "Compiled with Raylib Version %s\n", raylib_version);
    fprintf(stdout, "Compiled with libpipewire %s\n"
                    "Linked with libpipewire %s\n",
            pipewire_headers_version, pipewire_library_version);
}

void debug_log_record_samples(DebugLog *log, const float *samples, size_t count) {
    if (samples == NULL) {
        return;
    }

    for (size_t i = 0; i < count; ++i) {
        if (!isfinite(samples[i])) {
            log->non_finite_sample_count++;
            continue;
        }

        if (log->finite_sample_count == 0) {
            log->minimum_sample = samples[i];
            log->maximum_sample = samples[i];
        } else {
            if (samples[i] < log->minimum_sample) {
                log->minimum_sample = samples[i];
            }
            if (samples[i] > log->maximum_sample) {
                log->maximum_sample = samples[i];
            }
        }
        log->squared_sample_sum += (double)samples[i] * samples[i];
        log->finite_sample_count++;
    }
}

void debug_log_report_if_due(DebugLog *log, AudioCapture *capture,
                             double current_time) {
    if (current_time - log->last_report_time < DEBUG_REPORT_INTERVAL_SECONDS) {
        return;
    }

    uint32_t process_callbacks = atomic_exchange_explicit(
        &capture->process_callbacks, 0, memory_order_relaxed);
    uint32_t dequeued_buffers = atomic_exchange_explicit(
        &capture->dequeued_buffers, 0, memory_order_relaxed);
    uint32_t buffers_with_samples = atomic_exchange_explicit(
        &capture->buffers_with_samples, 0, memory_order_relaxed);
    uint32_t sample_values_received = atomic_exchange_explicit(
        &capture->sample_values_received, 0, memory_order_relaxed);

    fprintf(stderr,
            "PipeWire activity: callbacks=%u, buffers=%u, nonempty buffers=%u, float values=%u\n",
            process_callbacks, dequeued_buffers, buffers_with_samples,
            sample_values_received);
    if (log->finite_sample_count > 0) {
        double rms = sqrt(log->squared_sample_sum /
                          (double)log->finite_sample_count);
        float peak = fmaxf(fabsf(log->minimum_sample),
                           fabsf(log->maximum_sample));
        fprintf(stderr,
                "AudioBuffer levels: finite=%zu, nonfinite=%zu, min=%g, max=%g, peak=%g, RMS=%g\n",
                log->finite_sample_count, log->non_finite_sample_count,
                log->minimum_sample, log->maximum_sample, peak, rms);
    } else {
        fprintf(stderr,
                "AudioBuffer levels: no finite values reached the main loop (nonfinite=%zu)\n",
                log->non_finite_sample_count);
    }

    log->squared_sample_sum = 0.0;
    log->finite_sample_count = 0;
    log->non_finite_sample_count = 0;
    log->last_report_time = current_time;
}
