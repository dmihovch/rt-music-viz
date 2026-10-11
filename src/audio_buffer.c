#include "audio_buffer.h"
#include <endian.h>
#include <stdatomic.h>
#include <stdlib.h>

static inline size_t next_i(size_t i, size_t cap){
	return (i + 1) % cap;
}

int audio_buffer_init(AudioBuffer *buffer, size_t capacity) {
	buffer->samples = (float*)malloc(sizeof(float) * capacity);
	if(!buffer->samples){
		return 1;
	}
	buffer->capacity = capacity;
	atomic_init(&buffer->windex, 0);
	atomic_init(&buffer->rindex, 0);
	return 0;
}

void audio_buffer_destroy(AudioBuffer *buffer) {
	if(buffer->samples){
		free(buffer->samples);
	}
	return;
}

//only to be called by the audio capturer
void audio_buffer_write(AudioBuffer *buffer, const float *samples, size_t count) {

	for(size_t i = 0; i < count; ++i){
		size_t write = atomic_load_explicit(&buffer->windex, memory_order_relaxed);
		size_t next_write = next_i(write, buffer->capacity);
		size_t read = atomic_load_explicit(&buffer->rindex, memory_order_acquire);
		if(next_write == read){
			break;
		}
		buffer->samples[write]	= samples[i];
		atomic_store_explicit(&buffer->windex, next_write, memory_order_release);
	}

    return;
}

//only to be called by main event loop
size_t audio_buffer_read(AudioBuffer *buffer, float *samples, size_t count) {
	size_t samples_read = 0;
	
	for(size_t i = 0; i < count; ++i){
		size_t read = atomic_load_explicit(&buffer->rindex, memory_order_relaxed);
		size_t write = atomic_load_explicit(&buffer->windex, memory_order_acquire);
		if(read == write){
			break;
		}
		samples[i] = buffer->samples[read];
		samples_read++;
		atomic_store_explicit(&buffer->rindex, next_i(read, buffer->capacity), memory_order_release);
	}

    return samples_read;
}

size_t audio_buffer_available(const AudioBuffer *buffer) {
    // TODO: return write_index - read_index.
    (void)buffer;
    return 0;
}
