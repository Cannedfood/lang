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
void lang_buffer_free(lang_buffer* buffer) {
	buffer->length = 0;
	buffer->capacity = 0;
	free(buffer->data);
}

LANG_UTIL_BUFFER_API
void lang_buffer_reserve(lang_buffer* buffer, size_t bytes) {
	if(buffer->capacity < buffer->length + bytes) {
		if(buffer->capacity) {
			while(buffer->capacity < buffer->length + bytes)
				buffer->capacity *= 2;
		}
		else
			buffer->capacity = 256;

		buffer->data = realloc(buffer->data, buffer->capacity);
	}
}

LANG_UTIL_BUFFER_API
void lang_buffer_append(lang_buffer* buffer,  void const* data, size_t bytes) {
	lang_buffer_reserve(buffer, bytes);
	memcpy(buffer->data + buffer->length, data, bytes);
	buffer->length += bytes;
}
