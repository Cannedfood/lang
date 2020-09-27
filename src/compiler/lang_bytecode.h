#pragma once

#include "../util/byte_buffer.h"

typedef void(*lang_bytecode_errfn)(const char* file, int lineNumber, const char* line, const char* message);

int lang_bytecode_parse_line(const char* line, byte_buffer* into);
int lang_bytecode_parse_file(const char* path, byte_buffer* into, lang_bytecode_errfn errorfn_or_null);
