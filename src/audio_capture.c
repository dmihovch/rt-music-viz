
#include "audio_capture.h"
#include "audio_buffer.h"
#include "pipewire/stream.h"
#include "pipewire/thread-loop.h"
#include <spa/param/audio/format-utils.h>
#include <spa/param/audio/raw.h>
#include <spa/param/audio/raw-types.h>
#include <spa/pod/builder.h>
#include <spa/utils/result.h>
#include <stdio.h>

static void on_process(void* userdata){
	AudioCapture* capture = userdata;
    atomic_fetch_add_explicit(&capture->process_callbacks, 1, memory_order_relaxed);
    struct pw_buffer *pw_buf = pw_stream_dequeue_buffer(capture->stream);
    if (pw_buf == NULL){
		return;
	}
    atomic_fetch_add_explicit(&capture->dequeued_buffers, 1, memory_order_relaxed);

    struct spa_buffer *buf = pw_buf->buffer;

    if (buf->n_datas > 0 &&
        buf->datas[0].data != NULL &&
        buf->datas[0].chunk != NULL) {

        // Sample data starts at a byte offset into the mapped buffer.
        const float *samples = (const float *)((const uint8_t *)buf->datas[0].data + buf->datas[0].chunk->offset);

        uint32_t n_samples = buf->datas[0].chunk->size / sizeof(float);

        if (n_samples > 0) {
            atomic_fetch_add_explicit(&capture->buffers_with_samples, 1,
                                      memory_order_relaxed);
            atomic_fetch_add_explicit(&capture->sample_values_received, n_samples,
                                      memory_order_relaxed);
        }
        audio_buffer_write(capture->buffer, samples, n_samples);
    }

    pw_stream_queue_buffer(capture->stream, pw_buf);
	return;
}

static void on_state_changed(void *userdata, enum pw_stream_state old_state,
                             enum pw_stream_state state, const char *error) {
    AudioCapture *capture = userdata;

    fprintf(capture->log_output, "PipeWire stream state: %s -> %s",
            pw_stream_state_as_string(old_state),
            pw_stream_state_as_string(state));
    if (error != NULL) {
        fprintf(capture->log_output, " (%s)", error);
    }
    fputc('\n', capture->log_output);
}

static void on_param_changed(void *userdata, uint32_t id,
                             const struct spa_pod *param) {
    AudioCapture *capture = userdata;

    if (id != SPA_PARAM_Format) {
        return;
    }
    if (param == NULL) {
        fprintf(capture->log_output, "PipeWire stream format was cleared\n");
        return;
    }

    struct spa_audio_info_raw format;
    int result = spa_format_audio_raw_parse(param, &format);
    if (result < 0) {
        fprintf(capture->log_output,
                "PipeWire could not parse negotiated audio format: %s\n",
                spa_strerror(result));
        return;
    }

    fprintf(capture->log_output,
            "PipeWire negotiated audio format: %s, %u Hz, %u channels\n",
            spa_type_audio_format_to_short_name(format.format),
            format.rate, format.channels);
}

static const struct pw_stream_events stream_events = {
	.version = PW_VERSION_STREAM_EVENTS,
	.state_changed = on_state_changed,
	.param_changed = on_param_changed,
	.process = on_process,
};

int audio_capture_start(AudioCapture *capture, AudioBuffer *buffer,
                        uint32_t sample_rate, uint8_t channels,
                        FILE *log_output, int* argc, char*** argv) {
	capture->buffer = buffer;
	capture->log_output = log_output;
	capture->sample_rate = sample_rate;
	capture->channels = channels;
	atomic_init(&capture->process_callbacks, 0);
	atomic_init(&capture->dequeued_buffers, 0);
	atomic_init(&capture->buffers_with_samples, 0);
	atomic_init(&capture->sample_values_received, 0);
	pw_init(argc, argv);
	capture->thread_loop = pw_thread_loop_new("audio-capture", NULL); //change if we need special props
	if (capture->thread_loop == NULL) {
		fprintf(capture->log_output,
				"PipeWire failed to create the audio thread loop\n");
		pw_deinit();
		return 1;
	}
	pw_thread_loop_lock(capture->thread_loop);
	if(pw_thread_loop_start(capture->thread_loop) < 0){
		fprintf(capture->log_output,
				"PipeWire failed to start the audio thread loop\n");
		pw_thread_loop_unlock(capture->thread_loop);
		pw_thread_loop_destroy(capture->thread_loop);
		pw_deinit();
		return 1;
	}


	struct pw_properties* props = pw_properties_new(
        PW_KEY_MEDIA_TYPE, "Audio",
        PW_KEY_MEDIA_CATEGORY, "Capture",
        PW_KEY_MEDIA_ROLE, "Music",
        NULL
    );
	if (props == NULL) {
		fprintf(capture->log_output,
				"PipeWire failed to create stream properties\n");
		pw_thread_loop_unlock(capture->thread_loop);
		pw_thread_loop_stop(capture->thread_loop);
		pw_thread_loop_destroy(capture->thread_loop);
		pw_deinit();
		return 1;
	}

    /* Capture the default output's monitor. */
    pw_properties_set(props, PW_KEY_STREAM_CAPTURE_SINK, "true");

    capture->stream = pw_stream_new_simple(
        pw_thread_loop_get_loop(capture->thread_loop),
        "audio-capture",
        props,
        &stream_events,
        capture
    );

    if (!capture->stream) {
		fprintf(capture->log_output,
				"PipeWire failed to create the capture stream\n");
        pw_thread_loop_unlock(capture->thread_loop);
        pw_thread_loop_stop(capture->thread_loop);
        pw_thread_loop_destroy(capture->thread_loop);
        pw_deinit();
        return 1;
    }

    uint8_t pod_buffer[1024];
    struct spa_pod_builder builder =
        SPA_POD_BUILDER_INIT(pod_buffer, sizeof(pod_buffer));

    const struct spa_pod *params[1];

    // Pin the negotiated format to the requested rate and channels so the
    // captured layout matches capture->sample_rate and capture->channels.
    params[0] = spa_format_audio_raw_build(
        &builder,
        SPA_PARAM_EnumFormat,
        &SPA_AUDIO_INFO_RAW_INIT(
            .format = SPA_AUDIO_FORMAT_F32,
            .rate = sample_rate,
            .channels = channels
        )
    );

    int res = pw_stream_connect(
        capture->stream,
        PW_DIRECTION_INPUT,
        PW_ID_ANY,
        PW_STREAM_FLAG_AUTOCONNECT |
        PW_STREAM_FLAG_MAP_BUFFERS |
        PW_STREAM_FLAG_RT_PROCESS,
        params,
        1
    );

    pw_thread_loop_unlock(capture->thread_loop);

	if(res < 0){
		fprintf(capture->log_output,
				"PipeWire stream connection request failed: %s (%d)\n",
				spa_strerror(res), res);
		audio_capture_stop(capture);
	} else {
		fprintf(capture->log_output,
				"PipeWire stream connection request accepted; waiting for state and format events\n");
	}
    return res;
}

void audio_capture_stop(AudioCapture *capture) {
	pw_thread_loop_lock(capture->thread_loop);
	pw_stream_destroy(capture->stream);
	pw_thread_loop_unlock(capture->thread_loop);
	pw_thread_loop_stop(capture->thread_loop);
	pw_thread_loop_destroy(capture->thread_loop);
	pw_deinit();
}
