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

void debug_log_runtime_versions(FILE *log_output, const char *raylib_version,
                                const char *pipewire_headers_version,
                                const char *pipewire_library_version) {
    fprintf(log_output, "Compiled with Raylib Version %s\n", raylib_version);
    fprintf(log_output, "Compiled with libpipewire %s\n"
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

bool debug_log_collect_report(DebugLog *log, AudioCapture *capture,
                              double current_time, DebugLogReport *report) {
    if (current_time - log->last_report_time < DEBUG_REPORT_INTERVAL_SECONDS) {
        return false;
    }

    *report = (DebugLogReport){
        .process_callbacks = atomic_exchange_explicit(
            &capture->process_callbacks, 0, memory_order_relaxed),
        .dequeued_buffers = atomic_exchange_explicit(
            &capture->dequeued_buffers, 0, memory_order_relaxed),
        .buffers_with_samples = atomic_exchange_explicit(
            &capture->buffers_with_samples, 0, memory_order_relaxed),
        .sample_values_received = atomic_exchange_explicit(
            &capture->sample_values_received, 0, memory_order_relaxed),
        .squared_sample_sum = log->squared_sample_sum,
        .finite_sample_count = log->finite_sample_count,
        .non_finite_sample_count = log->non_finite_sample_count,
        .minimum_sample = log->minimum_sample,
        .maximum_sample = log->maximum_sample,
    };

    log->squared_sample_sum = 0.0;
    log->finite_sample_count = 0;
    log->non_finite_sample_count = 0;
    log->last_report_time = current_time;
    return true;
}

void debug_log_print_report(FILE *log_output, const DebugLogReport *report) {
    fprintf(log_output,
            "PipeWire activity: callbacks=%u, buffers=%u, nonempty buffers=%u, float values=%u\n",
            report->process_callbacks, report->dequeued_buffers,
            report->buffers_with_samples, report->sample_values_received);
    if (report->finite_sample_count > 0) {
        double rms = sqrt(report->squared_sample_sum /
                          (double)report->finite_sample_count);
        float peak = fmaxf(fabsf(report->minimum_sample),
                           fabsf(report->maximum_sample));
        fprintf(log_output,
                "AudioBuffer levels: finite=%zu, nonfinite=%zu, min=%g, max=%g, peak=%g, RMS=%g\n",
                report->finite_sample_count, report->non_finite_sample_count,
                report->minimum_sample, report->maximum_sample, peak, rms);
    } else {
        fprintf(log_output,
                "AudioBuffer levels: no finite values reached the main loop (nonfinite=%zu)\n",
                report->non_finite_sample_count);
    }
}
