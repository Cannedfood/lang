#pragma once

#include "../config.h"

#include "../util/lang_buffer.h"

typedef void(*lang_bytecode_errfn)(void* userptr, const char* file, int lineNumber, const char* line, const char* message);

LANG_BYTECODE_API int lang_bytecode_parse_line(const char* line, lang_buffer* into);
LANG_BYTECODE_API int lang_bytecode_parse_file(const char* path, lang_buffer* into, lang_bytecode_errfn errorfn_or_null LANG_DEFAULT(nullptr), void* errfn_userptr_or_null LANG_DEFAULT(nullptr));
LANG_BYTECODE_API int lang_bytecode_convert_jumps_to_bytes(const char* bytecode, int length);
