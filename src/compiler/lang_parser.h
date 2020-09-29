#pragma once

#include "lang_tokens.h"

struct lang_parser {
	void* userpointer;

	void(*pfnError)(void* userpointer, const char* fmt, ...); // The parser only allocates on the stack, so you may longjmp out of it
};
typedef struct lang_parser lang_parser;

void lang_parser_parse(lang_parser* parser, lang_tokenizer* tokens);
