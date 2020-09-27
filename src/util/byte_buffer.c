#include "byte_buffer.h"

#include <string.h>
#include <malloc.h>

byte_buffer bb_new() {
	byte_buffer result;
	result.data = 0;
	result.length = 0;
	result.capacity = 0;
	return result;
}
void bb_free(byte_buffer* buffer) {
	buffer->length = 0;
	buffer->capacity = 0;
	free(buffer->data);
}

void bb_reserve(byte_buffer* buffer, size_t bytes) {
	if(buffer->capacity < buffer->length + bytes) {
		if(buffer->capacity)
			buffer->capacity *= 2;
		else
			buffer->capacity = 256;

		buffer->data = realloc(buffer->data, buffer->capacity);
	}
}

void bb_append(byte_buffer* buffer,  void const* data, size_t bytes) {
	bb_reserve(buffer, bytes);
	memcpy(buffer->data + buffer->length, data, bytes);
	buffer->length += bytes;
}
