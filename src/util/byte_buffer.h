#pragma once

#include <stddef.h>
#include <stdint.h>

struct byte_buffer {
	char*  data;
	size_t length;
	size_t capacity;
};
typedef struct byte_buffer byte_buffer;

byte_buffer bb_new();
void        bb_free(byte_buffer*);

void        bb_reserve(byte_buffer*, size_t bytes);
void        bb_append(byte_buffer*,  void const* data, size_t bytes);

inline static void bb_pushc  (byte_buffer* buffer, char     value) { bb_append(buffer, &value, sizeof(value)); }
inline static void bb_pushi32(byte_buffer* buffer, int32_t  value) { bb_append(buffer, &value, sizeof(value)); }
inline static void bb_pushf32(byte_buffer* buffer, float    value) { bb_append(buffer, &value, sizeof(value)); }
inline static void bb_pushi64(byte_buffer* buffer, uint64_t value) { bb_append(buffer, &value, sizeof(value)); }
inline static void bb_pushf64(byte_buffer* buffer, double   value) { bb_append(buffer, &value, sizeof(value)); }
