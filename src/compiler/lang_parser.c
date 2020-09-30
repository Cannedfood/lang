#include "lang_parser.h"

#include <stdio.h> // vfprintf, stderr
#include <stdlib.h> // exit
#include <assert.h> // assert
#include <stdarg.h> // va_list, va_start, va_end

static inline
lang_token _nextToken(lang_tokenizer* tokens) {
	tokens->pfnNextToken(tokens);
	return tokens->token;
}

static
void _parseFunction(lang_parser* parser, lang_tokenizer* tokens) {
	assert(tokens->token.type == lang_token_open_brace);

	
}

static
void _parseClass(lang_parser* parser, lang_tokenizer* tokens) {
	assert(tokens->token.type == lang_token_class);

	lang_token className = _nextToken(tokens);

	printf("class %.*s {\n", className.length, className.text);

	// TODO: parse class body
	while(0) {
		_nextToken(tokens);
		switch(tokens->token.type) {
		case lang_token_name:

			break;
		default:
			parser->pfnError(
				"%s:%i:%i: Class member or end, got %s: '%s'",
				tokens->token.file, tokens->token.line, tokens->token.character,
				lang_token_names[tokens->token.type],
				tokens->token.text
			);
		}
	}

	printf("} // class %.*s\n", className.length, className.text);
}

static
void _parseBlock(lang_parser* parser, lang_tokenizer* tokens) {
	switch (tokens->token.type) {
	case lang_token_class:
		_parseClass(parser, tokens);
	break;
	case lang_token_name:

	break;
	default:
		parser->pfnError(parser->userpointer,
			"%s:%i:%i: Expected statement or declaration, got %s: '%s'",
			tokens->token.file, tokens->token.line, tokens->token.character,
			lang_token_names[tokens->token.type],
			tokens->token.text
		);
		// TODO
		exit(-1);
	}
}

static
void _defaultError(void* userpointer, const char* fmt, ...) {
	va_list args;
	va_start(args, fmt);
	vfprintf(stderr, fmt, args);
	va_end(args);
}

void lang_parser_parse(lang_parser* parser, lang_tokenizer* tokens) {
	if(!parser->pfnError) parser->pfnError = &_defaultError;

	_nextToken(tokens);

	_parseBlock(parser, tokens);
}
