#pragma once

#include "lang_tokens.h"

#include "../config.h"

typedef struct lang_parser lang_parser;
struct lang_parser {
	// -- Comments --
	void(*pfnComment)(lang_parser* p, lang_token const* comment);

	// -- Expressions --
	// subexpressions (parantheses)
	void(*pfnBeginSubexpression)(lang_parser* p, lang_token const* where);
	void(*pfnEndSubexpression)(lang_parser* p, lang_token const* where);
	// operations
	void(*pfnBinop)(lang_parser* p, lang_token const* which);
	void(*pfnBeginCall)(lang_parser* p, lang_token const* where);
	void(*pfnNextCallArgument)(lang_parser* p, lang_token const* where);
	void(*pfnEndCall)(lang_parser* p, lang_token const* where);
	// values
	void(*pfnValue)(lang_parser* p, lang_token const* value);
	// functions
	void(*pfnBeginFunction)(lang_parser* p, lang_token const* where);
	void(*pfnFuncArgument)(lang_parser* p, lang_token const* name);
	void(*pfnEndFunction)(lang_parser* p, lang_token const* where);

	// -- Statements --
	void(*pfnNextStatement)(lang_parser* p);
	// if/else
	void(*pfnIf)(lang_parser* p, lang_token const* where);
	void(*pfnIfBody)(lang_parser* p);
	void(*pfnElse)(lang_parser* p, lang_token const* where);
	void(*pfnEndIf)(lang_parser* p);
	// foreach
	void(*pfnWhile)(lang_parser* p, lang_token const* where);
	void(*pfnFor)(lang_parser* p, lang_token const* where);
	void(*pfnForeach)(lang_parser* p, lang_token const* where);
	void(*pfnForeachVariable)(lang_parser* p, lang_token const* where);
	void(*pfnLoopBody)(lang_parser* p, lang_token const* where);
	void(*pfnEndLoop)(lang_parser* p, lang_token const* where);

	void(*pfnBeginBlock)(lang_parser* p, lang_token const* where);
	void(*pfnEndBlock)  (lang_parser* p, lang_token const* where);
	// declarations
	void(*pfnDeclare)(lang_parser* p, lang_token const* name);
	void(*pfnInitDeclaration)(lang_parser* p);
	// classes
	void(*pfnBeginClass)(lang_parser* p, lang_token const* name);
	void(*pfnEndClass)(lang_parser* p, lang_token const* where);

	void(*pfnError)(lang_parser* p, const char* fmt, ...); // The parser only allocates on the stack, so you may longjmp out of it
};


enum lang_parser_default_flags {
	lang_parser_defaults_print_errors = 1,
};
LANG_PARSER_API
lang_parser lang_parser_defaults(int flags LANG_DEFAULT(0));

LANG_PARSER_API
void lang_parser_parse(lang_parser* parser, lang_tokenizer* tokens);
