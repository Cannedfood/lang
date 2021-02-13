#include "lang_parser.h"

#include <stdio.h> // vfprintf, stderr
#include <stdlib.h> // exit
#include <assert.h> // assert
#include <stdarg.h> // va_list, va_start, va_end
#include <memory.h> // memset

// #define WRITE(...) printf(__VA_ARGS__)
#define WRITE(...)

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
	while(_lang_is_comment(&tokens->current)) {
		if(parser->pfnComment)
			parser->pfnComment(parser->userpointer, &tokens->current);
		tokens->pfnNextToken(tokens);
	}
	return tokens->current;
}

static inline
lang_token _lang_next_token_expect(lang_parser* parser, lang_tokenizer* tokens, lang_token_type expected, const char* msg) {
	_lang_next_token(parser, tokens);
	if(tokens->current.type != expected) {
		_lang_parser_error(msg, parser, &tokens->current);
	}
	return tokens->current;
}

static void _lang_parse_function_arglist_and_body(lang_parser* parser, lang_tokenizer* tokens);
static void _lang_parse_class_definition(lang_parser* parser, lang_tokenizer* tokens);
static void _lang_parse_expression(lang_parser* parser, lang_tokenizer* tokens, int asStatement);
static void _lang_parse_statement(lang_parser* parser, lang_tokenizer* tokens);

static
void _lang_parse_argument_list(lang_parser* parser, lang_tokenizer* tokens) {
	assert(tokens->current.type == lang_token_open_parenthesis);
	_lang_next_token(parser, tokens);

	if(tokens->current.type != lang_token_close_parenthesis) {
		while(1) {
			if(tokens->current.type != lang_token_name) {
				_lang_parser_error("Expected argument name", parser, &tokens->current);
			}
			WRITE("%.*s, ", tokens->current.length, tokens->current.text);

			_lang_next_token(parser, tokens);
			if(tokens->current.type == lang_token_close_parenthesis)
				break;
			else if(tokens->current.type == lang_token_comma)
				continue;
			else {
				_lang_parser_error("Expected , to contine argument list or ) to end it", parser, &tokens->current);
				break;
			}
		}
	}
	_lang_next_token(parser, tokens);
}

static
void _lang_parse_function_body(lang_parser* parser, lang_tokenizer* tokens) {
	if(tokens->current.type != lang_token_open_curly) {
		_lang_parser_error("Expected start of function body '{'", parser, &tokens->current);
	}

	_lang_next_token(parser, tokens);
	while(tokens->current.type != lang_token_close_curly && tokens->current.type != lang_token_end_of_file) {
		_lang_parse_statement(parser, tokens);
	}
	_lang_next_token(parser, tokens);
}

void _lang_parse_function_arglist_and_body(lang_parser* parser, lang_tokenizer* tokens) {
	WRITE("function(");
		_lang_parse_argument_list(parser, tokens);
	WRITE(") {");
		_lang_parse_function_body(parser, tokens);
	WRITE("}\n");
}

static
void _lang_parse_class_member(lang_parser* parser, lang_tokenizer* tokens) {
	do {
		int isPublic = 0;

		if(tokens->current.type == lang_token_pub) {
			isPublic = 1;
			_lang_next_token(parser, tokens);
		}

		if(tokens->current.type != lang_token_name) {
			_lang_parser_error("Expected member name", parser, &tokens->current);
		}

		WRITE("declmember: %.*s = ", tokens->current.length, tokens->current.text);
		_lang_next_token(parser, tokens);

		if(tokens->current.type == lang_token_open_parenthesis) {
			_lang_parse_function_arglist_and_body(parser, tokens);
		}
		else {
			_lang_parser_error("Expected open brace '(' for function declaration", parser, &tokens->current);
		}

	} while(tokens->current.type != lang_token_close_curly && tokens->current.type != lang_token_end_of_file);
}

static
void _lang_parse_class_definition(lang_parser* parser, lang_tokenizer* tokens) {
	assert(tokens->current.type == lang_token_class);

	lang_token className = _lang_next_token_expect(parser, tokens, lang_token_name, "Expected class name");

	WRITE("class %.*s {\n", className.length, className.text);

	_lang_next_token_expect(parser, tokens, lang_token_open_curly, "Expected opening curly brace in class declaration");

	_lang_next_token(parser, tokens);
	while(tokens->current.type != lang_token_close_curly && tokens->current.type != lang_token_end_of_file) {
		_lang_parse_class_member(parser, tokens);
	}

	_lang_next_token(parser, tokens);

	WRITE("} // class\n");
}

static
void _lang_parse_function_call(lang_parser* parser, lang_tokenizer* tokens) {
	WRITE("(");
	if(_lang_next_token(parser, tokens).type != lang_token_close_parenthesis) {
		while(1) {
			_lang_parse_expression(parser, tokens, 0);
			if(tokens->current.type == lang_token_comma) {
				_lang_next_token(parser, tokens);
				continue;
			}
			else if(tokens->current.type == lang_token_close_parenthesis) {
				break;
			}
			else {
				_lang_parser_error("Expected , (continue call argument list) or ) (end call argument list)", parser, &tokens->current);
				break;
			}
		}
	}
	WRITE(")");
	if(tokens->current.type == lang_token_close_parenthesis)
		_lang_next_token(parser, tokens);
}

static
int _lang_parse_closed_expression(lang_parser* parser, lang_tokenizer* tokens) {
	if(tokens->current.type == lang_token_string_literal) {
		WRITE("string(%.*s)", tokens->current.length, tokens->current.text);
		_lang_next_token(parser, tokens);
		return 1;
	}
	else if(tokens->current.type == lang_token_number) {
		WRITE("number(%.*s)", tokens->current.length, tokens->current.text);
		_lang_next_token(parser, tokens);
		return 1;
	}
	else if(tokens->current.type == lang_token_name) {
		WRITE("var(%.*s)", tokens->current.length, tokens->current.text);
		_lang_next_token(parser, tokens);
		return 1;
	}
	else if(tokens->current.type == lang_token_open_parenthesis) {
		WRITE("(\n");
		_lang_next_token(parser, tokens);
		_lang_parse_expression(parser, tokens, 0);
		_lang_next_token_expect(parser, tokens, lang_token_close_parenthesis, "Expected closing brace ')'");
		WRITE(")\n");
		return 1;
	}
	return 0;
}

static
void _lang_parse_continue_expression(lang_parser* parser, lang_tokenizer* tokens) {
	while(1) {
		switch (tokens->current.type) {
		case lang_token_dot: {
			lang_token name = _lang_next_token_expect(parser, tokens, lang_token_name, "Expected name after '.'");
			_lang_next_token(parser, tokens);
			WRITE(".%.*s", name.length, name.text);
		} break;
		case lang_token_open_bracket: {
			_lang_next_token(parser, tokens);
			WRITE("[");
				_lang_parse_expression(parser, tokens, 0);
			WRITE("]");
			_lang_next_token_expect(parser, tokens, lang_token_close_bracket, "Expected closing brace");
			_lang_next_token(parser, tokens);
		} break;
		case lang_token_open_parenthesis: _lang_parse_function_call(parser, tokens); break;
		case lang_token_assign: WRITE(" = "); _lang_next_token(parser, tokens);
			if(!_lang_parse_closed_expression(parser, tokens)) {
				_lang_parser_error("Expected closed expression", parser, &tokens->current);
			}
		break;
		case lang_token_plus: WRITE(" + "); _lang_next_token(parser, tokens);
			if(!_lang_parse_closed_expression(parser, tokens)) {
				_lang_parser_error("Expected closed expression", parser, &tokens->current);
			}
		break;
		case lang_token_minus: WRITE(" - "); _lang_next_token(parser, tokens);
			if(!_lang_parse_closed_expression(parser, tokens)) {
				_lang_parser_error("Expected closed expression", parser, &tokens->current);
			}
		break;
		case lang_token_mul: WRITE(" * "); _lang_next_token(parser, tokens);
			if(!_lang_parse_closed_expression(parser, tokens)) {
				_lang_parser_error("Expected closed expression", parser, &tokens->current);
			}
		break;
		case lang_token_div: WRITE("/\n"); _lang_next_token(parser, tokens);
			if(!_lang_parse_closed_expression(parser, tokens)) {
				_lang_parser_error("Expected closed expression", parser, &tokens->current);
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
	// WRITE("End expression at %s:%i:%i: '%.*s'\n",
	// 	tokens->current.file, tokens->current.line, tokens->current.character,
	// 	tokens->current.length, tokens->current.text
	// );
}

static
void _lang_parse_variable_declaration(lang_parser* parser, lang_tokenizer* tokens) {
	lang_token varName = _lang_next_token_expect(parser, tokens, lang_token_name, "Expected variable name after 'var' keyword");

	_lang_next_token(parser, tokens);
	if(tokens->current.type == lang_token_end_stmt) {
		WRITE("declvar: %.*s;\n", tokens->current.length, tokens->current.text);
	}
	else if(tokens->current.type == lang_token_assign) {
		WRITE("declvar: %.*s = ", varName.length, varName.text);
		_lang_next_token(parser, tokens);
		_lang_parse_expression(parser, tokens, 0);
		if(tokens->current.type != lang_token_end_stmt) {
			_lang_parser_error("Expected end of statment (';')", parser, &tokens->current);
		}
	}
	else {
		_lang_parser_error("Expected = (assignment) or ; (end of statement)", parser, &tokens->current);
	}

	_lang_next_token(parser, tokens);
}

static
void _lang_parse_statement(lang_parser* parser, lang_tokenizer* tokens) {
	if(tokens->current.type == lang_token_def) {
		WRITE("\n\t");
		_lang_parse_variable_declaration(parser, tokens);
		WRITE(";\n");
	}
	else if(tokens->current.type == lang_token_class)
		_lang_parse_class_definition(parser, tokens);
	else {
		WRITE("\n\t");
		_lang_parse_expression(parser, tokens, 1);
		if(tokens->current.type != lang_token_end_stmt) {
			_lang_parser_error("Expected end of statement ';' after this expression", parser, &tokens->current);
		}
		_lang_next_token(parser, tokens);
		WRITE(";\n");
	}
}

static
void _lang_default_error_fn(void* userpointer, const char* fmt, ...) {
	va_list myargs;
	va_start(myargs, fmt);
	vfprintf(stderr, fmt, myargs);
	va_end(myargs);
}

LANG_PARSER_API
void lang_parser_parse(lang_parser* parser, lang_tokenizer* tokens) {
	if(!parser->pfnError) parser->pfnError = &_lang_default_error_fn;

	_lang_next_token(parser, tokens);

	while(tokens->current.type != lang_token_end_of_file) {
		_lang_parse_statement(parser, tokens);
	}
}

LANG_PARSER_API
lang_parser lang_parser_empty() {
	lang_parser result;
	memset(&result, 0, sizeof(result));
	return result;
}
