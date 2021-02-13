#include "print-parser.h"

#include <stdio.h>

static void _printComment(void* _, lang_token const* token) {
	printf("%.*s\n", token->length, token->text);
}

void _beginFunction    (lang_token const* where) { printf("function("); }
void _addArgument      (lang_token const* name) { printf("%.*s,", name->length, name->text); }
void _beginFunctionBody(lang_token const* where) { printf("{\n"); }
void _endFunctionBody  (lang_token const* where) { printf("}\n"); }

lang_parser print_parser_create() {
	lang_parser parser = lang_parser_empty();

	parser.pfnComment = _printComment;

	parser.pfnBeginFunction     = _beginFunction;
	parser.pfnAddArgument       = _addArgument;
	parser.pfnBeginFunctionBody = _beginFunctionBody;
	parser.pfnEndFunctionBody   = _endFunctionBody;

	return parser;
}
