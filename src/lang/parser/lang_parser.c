#include "lang_parser.h"

#include <stdio.h> // vfprintf, stderr
#include <stdlib.h> // exit
#include <assert.h> // assert
#include <stdarg.h> // va_list, va_start, va_end

static inline
void _lang_parser_error(const char* expected_message, lang_parser* parser, lang_token* token) {
	char buf[64];
	sprintf(buf, "%s:%i:%i:", token->file, token->line+1, token->character+1);

	parser->pfnError(parser->userpointer,
		"%-20s %s, got %s: '%.*s'\n",
		buf,
		expected_message,
		lang_token_names[token->type],
		token->length, token->text
	);
}

static inline
int _lang_is_comment(lang_token const* token) {
	return
		token->type == lang_token_line_comment ||
		token->type == lang_token_block_comment;
}

static inline
lang_token _lang_next_token(lang_parser* parser, lang_tokenizer* tokens) {
	tokens->pfnNextToken(tokens);
	while(_lang_is_comment(&tokens->token)) {
		if(parser->pfnComment)
			parser->pfnComment(parser->userpointer, &tokens->token);
		tokens->pfnNextToken(tokens);
	}
	return tokens->token;
}

static inline
lang_token _lang_next_token_expect(lang_parser* parser, lang_tokenizer* tokens, lang_token_type expected, const char* msg) {
	_lang_next_token(parser, tokens);
	if(tokens->token.type != expected) {
		_lang_parser_error(msg, parser, &tokens->token);
	}
	return tokens->token;
}

static void _lang_parse_function(lang_parser* parser, lang_tokenizer* tokens);
static void _lang_parse_class_declaration(lang_parser* parser, lang_tokenizer* tokens);
static void _lang_parse_expression(lang_parser* parser, lang_tokenizer* tokens, int asStatement);
static void _lang_parse_statement(lang_parser* parser, lang_tokenizer* tokens);

void _lang_parse_function(lang_parser* parser, lang_tokenizer* tokens) {
	assert(tokens->token.type == lang_token_open_brace);
	printf("Function: (");

	// Argument list
	while(1) {
		_lang_next_token(parser, tokens);
		if(tokens->token.type == lang_token_name) {
			printf("%.*s,", tokens->token.length, tokens->token.text);

			_lang_next_token(parser, tokens);
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

	if(_lang_next_token(parser, tokens).type != lang_token_open_curly) {
		_lang_parser_error("Expected opening curly brace '{' after function signature", parser, &tokens->token);
	}

	printf(") {\n");

	// Function body

	while(1) {
		_lang_next_token(parser, tokens);
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

	lang_token className = _lang_next_token(parser, tokens);

	printf("class %.*s {\n", className.length, className.text);

	// _nextToken(lang_tokenizer *tokens)
	if(_lang_next_token(parser, tokens).type != lang_token_open_curly) {
		_lang_parser_error("Expected opening curly brace { at start of class", parser, &tokens->token);
	}

	// TODO: parse class body
	while(1) {
		lang_token memberToken = _lang_next_token(parser, tokens);
		switch(memberToken.type) {
		case lang_token_name: {
			lang_token thing = _lang_next_token(parser, tokens);
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
void _lang_parse_function_call(lang_parser* parser, lang_tokenizer* tokens) {
	puts("call (");
	if(_lang_next_token(parser, tokens).type != lang_token_close_brace) {
		while(1) {
			_lang_parse_expression(parser, tokens, 0);
			if(tokens->token.type == lang_token_comma) {
				_lang_next_token(parser, tokens);
				continue;
			}
			else if(tokens->token.type == lang_token_close_brace) {
				break;
			}
			else {
				_lang_parser_error("Expected , (continue call argument list) or ) (end call argument list)", parser, &tokens->token);
			}
		}
	}
	puts(")");
	_lang_next_token(parser, tokens);
}

static
int _lang_parse_closed_expression(lang_parser* parser, lang_tokenizer* tokens) {
	if(tokens->token.type == lang_token_string_literal) {
		printf("string literal: %.*s\n", tokens->token.length, tokens->token.text);
		_lang_next_token(parser, tokens);
		return 1;
	}
	else if(tokens->token.type == lang_token_number) {
		printf("number literal: %.*s\n", tokens->token.length, tokens->token.text);
		_lang_next_token(parser, tokens);
		return 1;
	}
	else if(tokens->token.type == lang_token_name) {
		printf("Load variable %.*s\n", tokens->token.length, tokens->token.text);
		_lang_next_token(parser, tokens);
		return 1;
	}
	else if(tokens->token.type == lang_token_open_brace) {
		puts("(");
		_lang_next_token(parser, tokens);
		_lang_parse_expression(parser, tokens, 0);
		_lang_next_token_expect(parser, tokens, lang_token_close_brace, "Expected closing brace ')'");
		puts(")");
		return 1;
	}
	return 0;
}

static
void _lang_parse_continue_expression(lang_parser* parser, lang_tokenizer* tokens) {
	while(1) {
		switch (tokens->token.type) {
		case lang_token_dot: {
			lang_token name = _lang_next_token_expect(parser, tokens, lang_token_name, "Expected name after '.'");
			_lang_next_token(parser, tokens);
			if(tokens->token.type == lang_token_assign) {
				printf(".%.*s member = \n", name.length, name.text);
				_lang_next_token(parser, tokens);
				_lang_parse_expression(parser, tokens, 0);
			}
			else {
				printf(".%.*s\n", name.length, name.text);
			}
		} break;
		case lang_token_open_square: {
			_lang_next_token(parser, tokens);
			puts("[");
				_lang_parse_expression(parser, tokens, 0);
			puts("]");
			_lang_next_token_expect(parser, tokens, lang_token_close_square, "Expected closing brace");
			_lang_next_token(parser, tokens);
		} break;
		case lang_token_open_brace: _lang_parse_function_call(parser, tokens); break;
		case lang_token_plus: puts("+"); _lang_next_token(parser, tokens);
			if(!_lang_parse_closed_expression(parser, tokens)) {
				_lang_parser_error("Expected closed expression", parser, &tokens->token);
			}
		break;
		case lang_token_minus: puts("-"); _lang_next_token(parser, tokens);
			if(!_lang_parse_closed_expression(parser, tokens)) {
				_lang_parser_error("Expected closed expression", parser, &tokens->token);
			}
		break;
		case lang_token_mul: puts("*"); _lang_next_token(parser, tokens);
			if(!_lang_parse_closed_expression(parser, tokens)) {
				_lang_parser_error("Expected closed expression", parser, &tokens->token);
			}
		break;
		case lang_token_div: puts("/"); _lang_next_token(parser, tokens);
			if(!_lang_parse_closed_expression(parser, tokens)) {
				_lang_parser_error("Expected closed expression", parser, &tokens->token);
			}
		break;
		default:
			return;
		}
	}
}

static
void _lang_parse_expression(lang_parser* parser, lang_tokenizer* tokens, int asStatement) {
	_lang_parse_closed_expression(parser, tokens);
	_lang_parse_continue_expression(parser, tokens);
	// printf("End expression at %s:%i:%i: '%.*s'\n",
	// 	tokens->token.file, tokens->token.line, tokens->token.character,
	// 	tokens->token.length, tokens->token.text
	// );
}

static
void _lang_parse_variable_declaration(lang_parser* parser, lang_tokenizer* tokens) {
	lang_token varName = _lang_next_token_expect(parser, tokens, lang_token_name, "Expected variable name after 'var' keyword");

	_lang_next_token(parser, tokens);
	if(tokens->token.type == lang_token_end_stmt) {
		printf("declvar: %.*s\n", tokens->token.length, tokens->token.text);
	}
	else if(tokens->token.type == lang_token_assign) {
		printf("declvar: %.*s = \n", varName.length, varName.text);
		_lang_next_token(parser, tokens);
		_lang_parse_expression(parser, tokens, 0);
		if(tokens->token.type != lang_token_end_stmt) {
			_lang_parser_error("Expected end of statment (';')", parser, &tokens->token);
		}
	}
	else {
		_lang_parser_error("Expected = (assignment) or ; (end of statement)", parser, &tokens->token);
	}

	_lang_next_token(parser, tokens);
}

static
void _lang_parse_statement(lang_parser* parser, lang_tokenizer* tokens) {
	if(tokens->token.type == lang_token_var)
		_lang_parse_variable_declaration(parser, tokens);
	else if(tokens->token.type == lang_token_class)
		_lang_parse_class_declaration(parser, tokens);
	else {
		_lang_parse_expression(parser, tokens, 1);
		if(tokens->token.type != lang_token_end_stmt) {
			_lang_parser_error("Expected end of statement ';' after this expression", parser, &tokens->token);
		}
		_lang_next_token(parser, tokens);
	}
}

static
void _lang_parse_function_body(lang_parser* parser, lang_tokenizer* tokens) {
	while(tokens->token.type != lang_token_exit_block || tokens->token.type == lang_token_end_of_file) {
		if(tokens->token.type == lang_token_end_of_file) {
			_lang_parser_error("Expected statement or end of block", parser, &tokens->token);
			break;
		}

		_lang_parse_statement(parser, tokens);
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

	_lang_next_token(parser, tokens);

	while(tokens->token.type != lang_token_end_of_file) {
		_lang_parse_statement(parser, tokens);
	}
}
