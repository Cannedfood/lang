#pragma once

#include "lang_tokens.h"

#include "../config.h"

struct lang_parser {
	void* userpointer;

	// Comments
	void(*pfnComment)(void* userpointer, lang_token const* comment);

	// Expressions
	void(*pfnOperation)(void* userpointer, lang_token const* op);

	// Expressions/Statements
	void(*pfnIndex)(void* userpointer, lang_token const* name);

	// Statements
	void(*pfnAssignMember)(void* userpointer, lang_token const* memberName);
	void(*pfnAssign)(void* userpointer, lang_token const* memberName);

	void(*pfnError)(void* userpointer, const char* fmt, ...); // The parser only allocates on the stack, so you may longjmp out of it
};
typedef struct lang_parser lang_parser;

LANG_PARSER_API
void lang_parser_parse(lang_parser* parser, lang_tokenizer* tokens);
