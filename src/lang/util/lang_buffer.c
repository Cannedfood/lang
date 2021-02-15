#include "lang_buffer.h"

#include <string.h>
#include <malloc.h>

LANG_UTIL_BUFFER_API
lang_buffer lang_buffer_new() {
	lang_buffer result;
	result.data = 0;
	result.length = 0;
	result.capacity = 0;
	return result;
}
LANG_UTIL_BUFFER_API
void lang_buffer_destroy(lang_buffer* buffer) {
	free(buffer->data);

	buffer->length = 0;
	buffer->capacity = 0;
	buffer->data = 0;
}

LANG_UTIL_BUFFER_API
void lang_buffer_reserve(lang_buffer* buffer, size_t bytes) {
	if(buffer->capacity < buffer->length + bytes) {
		// Initial capacity
		if(buffer->capacity == 0)
			buffer->capacity = 256;

		// Double size until it fits
		while(buffer->capacity < (buffer->length + bytes))
			buffer->capacity *= 2;

		// Alloc!
		buffer->data = realloc(buffer->data, buffer->capacity);
	}
}

LANG_UTIL_BUFFER_API
void lang_buffer_append(lang_buffer* buffer, void const* data, size_t bytes) {
	lang_buffer_reserve(buffer, bytes);
	memcpy(buffer->data + buffer->length, data, bytes);
	buffer->length += bytes;
}
