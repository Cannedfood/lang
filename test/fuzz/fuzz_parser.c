#define LANG_API static
#define LANG_API_DECL static

#include <lang.c>

#include <stddef.h>
#include <stdlib.h>
#include <memory.h>

void void_errors(void* userdata, const char* fmt, ...) {}

int LLVMFuzzerTestOneInput(const char* data, size_t size) {
	lang_tokenizer tokenizer = lang_tokenizer_create(data, size, NULL);

	lang_parser parser = lang_parser_empty();
	parser.pfnError = void_errors;
	lang_parser_parse(&parser, &tokenizer);

	return 0; // Non-zero return values are reserved for future use.
}

