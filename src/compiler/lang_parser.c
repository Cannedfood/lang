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

static inline
void _parserError(const char* expected_message, lang_parser* parser, lang_token* token) {
	parser->pfnError(parser->userpointer,
		"%s:%i:%i: %s, got %s: '%.*s'\n",
		token->file, token->line, token->character,
		expected_message,
		lang_token_names[token->type],
		token->text
	);
}

static
void _parseExpression(lang_parser* parser, lang_tokenizer* tokens) {

}

static
void _parseStatement(lang_parser* parser, lang_tokenizer* tokens) {
	if(tokens->token.type == lang_token_var) {
		// Variable declaration
		lang_token varName = _nextToken(tokens);

		_nextToken(tokens);
		if(tokens->token.type == lang_token_end_stmt) {
			printf("declvar: %.*s\n", tokens->token.length, tokens->token.text);
			return; // Just a declaration appearently
		}
		else if(tokens->token.type == lang_token_assign) {
			printf("declvar: %.*s = \n", tokens->token.length, tokens->token.text);
			_nextToken(tokens);
			_parseExpression(parser, tokens);
		}
		else {
			_parserError("Expected assignment or end of ", parser, &tokens->token);
		}
	}
	else {

	}
}

static
void _parseFunction(lang_parser* parser, lang_tokenizer* tokens) {
	assert(tokens->token.type == lang_token_open_brace);
	printf("Function: (");

	// Argument list
	while(1) {
		_nextToken(tokens);
		if(tokens->token.type == lang_token_name) {
			printf("%.*s,", tokens->token.length, tokens->token.text);

			_nextToken(tokens);
			if(tokens->token.type == lang_token_close_brace) {
				break; // End of argument list
			}
			else if(tokens->token.type == lang_token_comma) {
				continue; // Next argument
			}
			else {
				_parserError(
					"Expected comma followed by an argument name or end of argument list ')'",
					parser, &tokens->token
				);
			}
		}
		else if(tokens->token.type == lang_token_close_brace) {
			break; // End of argument list
		}
		else {
			_parserError("Expected argument name or end of argument list ')'", parser, &tokens->token);
		}
	}

	if(_nextToken(tokens).type != lang_token_open_curly) {
		_parserError("Expected opening curly brace '{' after function signature", parser, &tokens->token);
	}

	printf(") {\n");

	// Function body

	while(1) {
		_nextToken(tokens);
		if(tokens->token.type == lang_token_close_curly) {
			break; // End of function body
		}
		else {
			_parseStatement(parser, tokens);
		}
	}

	printf("}\n");
}

static
void _parseClass(lang_parser* parser, lang_tokenizer* tokens) {
	assert(tokens->token.type == lang_token_class);

	lang_token className = _nextToken(tokens);

	printf("class %.*s {\n", className.length, className.text);

	// _nextToken(lang_tokenizer *tokens)
	if(_nextToken(tokens).type != lang_token_open_curly) {
		_parserError("Expected opening curly brace { at start of class", parser, &tokens->token);
	}

	// TODO: parse class body
	while(1) {
		lang_token memberToken = _nextToken(tokens);
		switch(memberToken.type) {
		case lang_token_name: {
			lang_token thing = _nextToken(tokens);
			if(thing.type == lang_token_end_stmt) {
				printf("memvar: %.*s\n", memberToken.length, memberToken.text);
			}
			// Assign=?
			else if(thing.type == lang_token_open_brace) {
				printf("memfn: %.*s\n", memberToken.length, memberToken.text);
				_parseFunction(parser, tokens);
			}
			else {
				_parserError(
						"Expected end of statement ';' for a member variable or function "
						"start '() {' for a member function",
						parser, &tokens->token);
			}
		} break;
		case lang_token_close_curly:
			goto END_OF_CLASS; // TODO: make this cleaner
		default:
			_parserError("Expected class member name or end of class via '}'", parser, &tokens->token);
		}
	}
	END_OF_CLASS:

	printf("} // class %.*s\n", className.length, className.text);
}

static
void _parseBlock(lang_parser* parser, lang_tokenizer* tokens) {
	switch (tokens->token.type) {
	case lang_token_class:
		_parseClass(parser, tokens);
		break;
	default:
		_parseStatement(parser, tokens);
		break;
	}
}

static
void _defaultError(void* userpointer, const char* fmt, ...) {
	va_list myargs;
	va_start(myargs, fmt);
	vfprintf(stderr, fmt, myargs);
	va_end(myargs);
}

void lang_parser_parse(lang_parser* parser, lang_tokenizer* tokens) {
	if(!parser->pfnError) parser->pfnError = &_defaultError;

	_nextToken(tokens);

	_parseBlock(parser, tokens);
}
