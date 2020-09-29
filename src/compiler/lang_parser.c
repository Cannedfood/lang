#include "lang_parser.h"

#include <stdio.h> // printf
#include <stdlib.h> // exit

static inline
lang_token _nextToken(lang_tokenizer* tokens) {
	lang_token result;
	tokens->pfnNextToken(tokens->userdata, &result);
	return result;
}

static inline
void _parseBlock(lang_tokenizer* tokens) {
	lang_token t = _nextToken(tokens);
	switch (t.type) {
	case lang_token_class: break;
	case lang_token_name: break;
	default:
		// TODO
		printf("%s:%i:%i: Expected statement or declaration, got ", t.file, t.line, t.character);
		exit(-1);
	}
}

void lang_parser_parse(lang_tokenizer* tokens) {
	_parseBlock(tokens);
}
