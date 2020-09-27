#pragma once

#include "../util/lang_buffer.h"

typedef void(*lang_bytecode_errfn)(const char* file, int lineNumber, const char* line, const char* message);

int lang_bytecode_parse_line(const char* line, lang_buffer* into);
int lang_bytecode_parse_file(const char* path, lang_buffer* into, lang_bytecode_errfn errorfn_or_null);
