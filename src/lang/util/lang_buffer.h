#pragma once

#include "../config.h"

#include <stddef.h>
#include <stdint.h>

struct lang_buffer {
	char*  data;
	size_t length;
	size_t capacity;
};
typedef struct lang_buffer lang_buffer;

LANG_UTIL_BUFFER_API lang_buffer lang_buffer_new();
LANG_UTIL_BUFFER_API void        lang_buffer_free(lang_buffer*);

LANG_UTIL_BUFFER_API void        lang_buffer_reserve(lang_buffer*, size_t bytes);
LANG_UTIL_BUFFER_API void        lang_buffer_append(lang_buffer*,  void const* data, size_t bytes);

static inline void lang_buffer_pushc  (lang_buffer* buffer, char     value) { lang_buffer_append(buffer, &value, sizeof(value)); }
static inline void lang_buffer_pushi32(lang_buffer* buffer, int32_t  value) { lang_buffer_append(buffer, &value, sizeof(value)); }
static inline void lang_buffer_pushf32(lang_buffer* buffer, float    value) { lang_buffer_append(buffer, &value, sizeof(value)); }
static inline void lang_buffer_pushi64(lang_buffer* buffer, uint64_t value) { lang_buffer_append(buffer, &value, sizeof(value)); }
static inline void lang_buffer_pushf64(lang_buffer* buffer, double   value) { lang_buffer_append(buffer, &value, sizeof(value)); }
