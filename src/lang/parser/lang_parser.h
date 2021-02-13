#pragma once

#include "lang_tokens.h"

#include "../config.h"

struct lang_parser {
	void* userpointer;

	// Comments
	void(*pfnComment)(void* userpointer, lang_token const* comment);

	// Operators
	void(*pfnBeginExpression)(lang_token const* where);

	void(*pfnLiteral)(lang_token const* value);
	void(*pfnBinop)(lang_token const* which);
	void(*pfnIndexOperator)(lang_token const* where);
	void(*pfnCallOperator)(lang_token const* where);

	void(*pfnEndExpression)(lang_token const* where);

	// Statement
	void(*pfnBeginFunction)(lang_token const* where);
	void(*pfnAddArgument)(lang_token const* name);
	void(*pfnBeginFunctionBody)(lang_token const* where);
	void(*pfnEndFunctionBody)(lang_token const* where);

	void(*pfnError)(void* userpointer, const char* fmt, ...); // The parser only allocates on the stack, so you may longjmp out of it
};
typedef struct lang_parser lang_parser;

LANG_PARSER_API
lang_parser lang_parser_empty();

LANG_PARSER_API
void lang_parser_parse(lang_parser* parser, lang_tokenizer* tokens);
