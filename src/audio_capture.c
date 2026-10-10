
#include "audio_capture.h"
#include "audio_buffer.h"
#include "pipewire/stream.h"
#include "pipewire/thread-loop.h"
#include <spa/param/audio/format-utils.h>
#include <spa/param/audio/raw.h>
#include <spa/pod/builder.h>

static void on_process(void* userdata){
	AudioCapture* capture = userdata;
    struct pw_buffer *pw_buf = pw_stream_dequeue_buffer(capture->stream);
    if (pw_buf == NULL){
		return;
	} 

    struct spa_buffer *buf = pw_buf->buffer;

    if (buf->n_datas > 0 &&
        buf->datas[0].data != NULL &&
        buf->datas[0].chunk != NULL) {

        const float *samples = buf->datas[0].data;

        uint32_t n_samples =
            buf->datas[0].chunk->size / sizeof(float);
		
		audio_buffer_write(capture->buffer, samples, n_samples);
    }

    pw_stream_queue_buffer(capture->stream, pw_buf);
	return;
}

static struct pw_stream_events stream_events = {
	.version = PW_VERSION_STREAM_EVENTS,
	.process = on_process,
};

int audio_capture_start(AudioCapture *capture, AudioBuffer *buffer, uint32_t sample_rate, uint8_t channels, int* argc, char*** argv) {
	pw_init(argc, argv);
	capture->buffer = buffer;
	capture->sample_rate = sample_rate;
	capture->channels = channels;
	capture->thread_loop = pw_thread_loop_new("audio-capture", NULL); //change if we need special props
	pw_thread_loop_lock(capture->thread_loop);
	if(pw_thread_loop_start(capture->thread_loop) < 0){
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

    /* Capture the default output's monitor. */
    pw_properties_set(props, PW_KEY_STREAM_CAPTURE_SINK, "true");

    capture->stream = pw_stream_new_simple(
        pw_thread_loop_get_loop(capture->thread_loop),
        "audio-capture",
        props,
        &stream_events,
        &buffer
    );

    if (!capture->stream) {
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

    params[0] = spa_format_audio_raw_build(
        &builder,
        SPA_PARAM_EnumFormat,
        &SPA_AUDIO_INFO_RAW_INIT(
            .format = SPA_AUDIO_FORMAT_F32
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
		audio_capture_stop(capture);
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
