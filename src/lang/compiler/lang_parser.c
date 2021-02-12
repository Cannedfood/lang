#include "lang_parser.h"

#include <stdio.h> // vfprintf, stderr
#include <stdlib.h> // exit
#include <assert.h> // assert
#include <stdarg.h> // va_list, va_start, va_end



static inline
lang_token _lang_next_token(lang_tokenizer* tokens) {
	tokens->pfnNextToken(tokens);
	return tokens->token;
}

static inline
void _lang_parser_error(const char* expected_message, lang_parser* parser, lang_token* token) {
	parser->pfnError(parser->userpointer,
		"%s:%i:%i: %s, got %s: '%.*s'\n",
		token->file, token->line, token->character,
		expected_message,
		lang_token_names[token->type],
		token->length, token->text
	);
}

static void _lang_parse_function(lang_parser* parser, lang_tokenizer* tokens);
static void _lang_parse_class_declaration(lang_parser* parser, lang_tokenizer* tokens);
static void _lang_parse_expression(lang_parser* parser, lang_tokenizer* tokens);
static void _lang_parse_statement(lang_parser* parser, lang_tokenizer* tokens);

void _lang_parse_function(lang_parser* parser, lang_tokenizer* tokens) {
	assert(tokens->token.type == lang_token_open_brace);
	printf("Function: (");

	// Argument list
	while(1) {
		_lang_next_token(tokens);
		if(tokens->token.type == lang_token_name) {
			printf("%.*s,", tokens->token.length, tokens->token.text);

			_lang_next_token(tokens);
			if(tokens->token.type == lang_token_close_brace) {
				break; // End of argument list
			}
			else if(tokens->token.type == lang_token_comma) {
				continue; // Next argument
			}
			else {
				_lang_parser_error(
					"Expected comma followed by an argument name or end of argument list ')'",
					parser, &tokens->token
				);
			}
		}
		else if(tokens->token.type == lang_token_close_brace) {
			break; // End of argument list
		}
		else {
			_lang_parser_error("Expected argument name or end of argument list ')'", parser, &tokens->token);
		}
	}

	if(_lang_next_token(tokens).type != lang_token_open_curly) {
		_lang_parser_error("Expected opening curly brace '{' after function signature", parser, &tokens->token);
	}

	printf(") {\n");

	// Function body

	while(1) {
		_lang_next_token(tokens);
		if(tokens->token.type == lang_token_close_curly) {
			break; // End of function body
		}
		else {
			_lang_parse_statement(parser, tokens);
		}
	}

	printf("}\n");
}

static
void _lang_parse_class_declaration(lang_parser* parser, lang_tokenizer* tokens) {
	assert(tokens->token.type == lang_token_class);

	lang_token className = _lang_next_token(tokens);

	printf("class %.*s {\n", className.length, className.text);

	// _nextToken(lang_tokenizer *tokens)
	if(_lang_next_token(tokens).type != lang_token_open_curly) {
		_lang_parser_error("Expected opening curly brace { at start of class", parser, &tokens->token);
	}

	// TODO: parse class body
	while(1) {
		lang_token memberToken = _lang_next_token(tokens);
		switch(memberToken.type) {
		case lang_token_name: {
			lang_token thing = _lang_next_token(tokens);
			if(thing.type == lang_token_end_stmt) {
				printf("memvar: %.*s\n", memberToken.length, memberToken.text);
			}
			// Assign=?
			else if(thing.type == lang_token_open_brace) {
				printf("memfn: %.*s\n", memberToken.length, memberToken.text);
				_lang_parse_function(parser, tokens);
			}
			else {
				_lang_parser_error(
						"Expected end of statement ';' for a member variable or function "
						"start '() {' for a member function",
						parser, &tokens->token);
			}
		} break;
		case lang_token_close_curly:
			goto END_OF_CLASS; // TODO: make this cleaner
		default:
			_lang_parser_error("Expected class member name or end of class via '}'", parser, &tokens->token);
		}
	}
	END_OF_CLASS:

	printf("} // class %.*s\n", className.length, className.text);
}

static
void _lang_parse_expression(lang_parser* parser, lang_tokenizer* tokens) {
	if(tokens->token.type == lang_token_open_brace) {
		printf("Start subexpression:\n");
		_lang_next_token(tokens);
		if(tokens->token.type == lang_token_close_brace) {
			_lang_parser_error("Expected subexpression, but got (), write an expression in the braces to remove this error", parser, &tokens->token);
		}
		_lang_parse_expression(parser, tokens);
	}
	else if(tokens->token.type == lang_token_close_brace) {
		printf("End subexpression\n");
	}
	else if(tokens->token.type == lang_token_plus) {
		
	}
}

static
void _lang_parse_statement(lang_parser* parser, lang_tokenizer* tokens) {
	if(tokens->token.type == lang_token_var) {
		// Variable declaration
		lang_token varName = _lang_next_token(tokens);

		_lang_next_token(tokens);
		if(tokens->token.type == lang_token_end_stmt) {
			printf("declvar: %.*s\n", tokens->token.length, tokens->token.text);
			return; // Just a declaration appearently
		}
		else if(tokens->token.type == lang_token_assign) {
			printf("declvar: %.*s = \n", tokens->token.length, tokens->token.text);
			printf("Assign to %.*s: \n", tokens->token.length, tokens->token.text);
			_lang_next_token(tokens);
			_lang_parse_expression(parser, tokens);
		}
		else {
			_lang_parser_error("Expected assignment or end of ", parser, &tokens->token);
		}

		if(tokens->token.type != lang_token_end_stmt) {
			_lang_parser_error("Expected end of statement ';'", parser, &tokens->token);
		}
		else {
			_lang_next_token(tokens);
		}
	}
	else if(tokens->token.type == lang_token_name) {
		lang_token varName = tokens->token;
		switch (_lang_next_token(tokens).type) {
			case lang_token_dot: {
				lang_token indexName = _lang_next_token(tokens);
				if(indexName.type == lang_token_name) {
					printf("Index %.*s.%.*s\n", varName.length, varName.text, indexName.length, indexName.text);
				}
				else {
					_lang_parser_error("Expected name after object indexing operator .", parser, &indexName);
				}
			} break;
			case lang_token_assign:
			break;
			default:
				break;
		}
	}
	else if(tokens->token.type == lang_token_class) {
		_lang_parse_class_declaration(parser, tokens);
	}
	else {
		_lang_parser_error("Expected class declaration or variable declaration", parser, &tokens->token);
	}
}

static
void _lang_parse_block_inner(lang_parser* parser, lang_tokenizer* tokens) {
	while(tokens->token.type != lang_token_exit_block) {
		if(tokens->token.type == lang_token_end_of_file) {
			_lang_parser_error("Expected statement or end of block", parser, &tokens->token);
			break;
		}

		_lang_parse_statement(parser, tokens);
		_lang_next_token(tokens);
	}
}

static
void _lang_default_error_fn(void* userpointer, const char* fmt, ...) {
	va_list myargs;
	va_start(myargs, fmt);
	vfprintf(stderr, fmt, myargs);
	va_end(myargs);
}

void lang_parser_parse(lang_parser* parser, lang_tokenizer* tokens) {
	if(!parser->pfnError) parser->pfnError = &_lang_default_error_fn;

	_lang_next_token(tokens);
	_lang_parse_block_inner(parser, tokens);
}
