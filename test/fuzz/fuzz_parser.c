#define LANG_API static
#define LANG_API_DECL static

#include <lang.c>

#include <stddef.h>
#include <stdlib.h>
#include <memory.h>

int LLVMFuzzerTestOneInput(const char* data, size_t size) {
	lang_tokenizer tokenizer = lang_tokenizer_create(data, size, NULL);

	lang_allocator* alloc = lang_new_allocator();

	lang_ast_parser parser = lang_create_parser_ast(lang_alloc_callbacks_for(alloc), 0);
	lang_parser_parse(&parser.parser, &tokenizer);

	lang_free_allocator(alloc);

	return 0; // Non-zero return values are reserved for future use.
}

