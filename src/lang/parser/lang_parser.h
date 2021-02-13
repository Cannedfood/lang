#pragma once

#include "lang_tokens.h"

#include "../config.h"

struct lang_parser {
	void* userpointer;

	// Comments
	void(*pfnComment)(void* userpointer, lang_token const* comment);

	void(*pfnError)(void* userpointer, const char* fmt, ...); // The parser only allocates on the stack, so you may longjmp out of it
};
typedef struct lang_parser lang_parser;

LANG_PARSER_API
void lang_parser_parse(lang_parser* parser, lang_tokenizer* tokens);
