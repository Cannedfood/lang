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

	parser->pfnError(parser,
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
			parser->pfnComment(parser, &tokens->current);
		tokens->pfnNextToken(tokens);
	}
	return tokens->current;
}

static inline
lang_token _lang_next_token_quiet(lang_tokenizer* tokens) {
	tokens->pfnNextToken(tokens);
	while(_lang_is_comment(&tokens->current)) {
		tokens->pfnNextToken(tokens);
	}
	return tokens->current;
}

static inline
int _lang_current_token_expect(lang_parser* parser, lang_tokenizer* tokens, lang_token_type expected, const char* msg) {
	if(tokens->current.type != expected) {
		_lang_parser_error(msg, parser, &tokens->current);
	}
	return tokens->current.type == expected;
}

static inline
int _lang_next_token_expect(lang_parser* parser, lang_tokenizer* tokens, lang_token_type expected, const char* msg) {
	_lang_next_token(parser, tokens);
	return _lang_current_token_expect(parser, tokens, expected, msg);
}

static inline
int _lang_skip_token_expect(lang_parser* parser, lang_tokenizer* tokens, lang_token_type expected, const char* msg) {
	if(!_lang_next_token_expect(parser, tokens, expected, msg))
		return 0;
	_lang_next_token(parser, tokens);
	return 1;
}

static void _lang_parse_function(lang_parser* parser, lang_tokenizer* tokens);
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
			parser->pfnFuncArgument(parser, &tokens->current);

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
		parser->pfnNextStatement(parser);
		_lang_parse_statement(parser, tokens);
	}
	_lang_next_token(parser, tokens);
}

void _lang_parse_function(lang_parser* parser, lang_tokenizer* tokens) {
	parser->pfnBeginFunction(parser, &tokens->current);
	WRITE("function(");
		_lang_parse_argument_list(parser, tokens);
	WRITE(") {");
		_lang_parse_function_body(parser, tokens);
	WRITE("}\n");
	parser->pfnEndFunction(parser, &tokens->current);
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
		parser->pfnDeclare(parser, &tokens->current);
		_lang_next_token(parser, tokens);

		if(tokens->current.type == lang_token_open_parenthesis) {
			_lang_parse_function(parser, tokens);
		}
		else {
			_lang_parser_error("Expected open brace '(' for function declaration", parser, &tokens->current);
		}

	} while(tokens->current.type != lang_token_close_curly && tokens->current.type != lang_token_end_of_file);
}

static
void _lang_parse_class_definition(lang_parser* parser, lang_tokenizer* tokens) {
	assert(tokens->current.type == lang_token_class);

	_lang_next_token_expect(parser, tokens, lang_token_name, "Expected class name");

	parser->pfnDeclare(parser, &tokens->current);

	WRITE("class %.*s {\n", className.length, className.text);

	_lang_next_token_expect(parser, tokens, lang_token_open_curly, "Expected opening curly brace in class declaration");
	parser->pfnBeginClass(parser, &tokens->current);

	_lang_next_token(parser, tokens);
	while(tokens->current.type != lang_token_close_curly && tokens->current.type != lang_token_end_of_file) {
		_lang_parse_class_member(parser, tokens);
	}

	parser->pfnEndClass(parser, &tokens->current);
	_lang_next_token(parser, tokens);

	WRITE("} // class\n");
}

static
void _lang_parse_function_call(lang_parser* parser, lang_tokenizer* tokens) {
	parser->pfnBeginCall(parser, &tokens->current);

	WRITE("(");
	if(_lang_next_token(parser, tokens).type != lang_token_close_parenthesis) {
		while(1) {
			parser->pfnNextCallArgument(parser, &tokens->current);
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
	parser->pfnEndCall(parser, &tokens->current);

	WRITE(")");
	if(tokens->current.type == lang_token_close_parenthesis)
		_lang_next_token(parser, tokens);
}

static
int _lang_parse_closed_expression(lang_parser* parser, lang_tokenizer* tokens) {
	if(
		tokens->current.type == lang_token_name ||
		tokens->current.type == lang_token_string_literal ||
		tokens->current.type == lang_token_number ||
		tokens->current.type == lang_token_true ||
		tokens->current.type == lang_token_false ||
		tokens->current.type == lang_token_null)
	{
		parser->pfnValue(parser, &tokens->current);
		_lang_next_token(parser, tokens);
		return 1;
	}
	else if(tokens->current.type == lang_token_open_parenthesis) {
		parser->pfnBeginSubexpression(parser, &tokens->current);
		_lang_next_token(parser, tokens);
		_lang_parse_expression(parser, tokens, 0);
		_lang_skip_token_expect(parser, tokens, lang_token_close_parenthesis, "Expected closing brace ')'");
		parser->pfnEndSubexpression(parser, &tokens->current);
		return 1;
	}
	return 0;
}

static
void _lang_parse_continue_expression(lang_parser* parser, lang_tokenizer* tokens) {
	while(1) {
		switch (tokens->current.type) {
		case lang_token_dot: {
			parser->pfnBinop(parser, &tokens->current);
			_lang_next_token_expect(parser, tokens, lang_token_name, "Expected name after '.'");
			parser->pfnValue(parser, &tokens->current);
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
		// binary operators
		case lang_token_assign:
		case lang_token_plus: case lang_token_minus:
		case lang_token_mul: case lang_token_div:
			parser->pfnBinop(parser, &tokens->current);
			_lang_next_token(parser, tokens);
			_lang_parse_expression(parser, tokens, 0);
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
void _lang_parse_declaration(lang_parser* parser, lang_tokenizer* tokens) {
	_lang_next_token_expect(parser, tokens, lang_token_name, "Expected variable name after 'var' keyword");

	parser->pfnDeclare(parser, &tokens->current);

	_lang_next_token(parser, tokens);
	if(tokens->current.type == lang_token_end_stmt) {
		WRITE("declvar: %.*s;\n", tokens->current.length, tokens->current.text);
	}
	else if(tokens->current.type == lang_token_open_parenthesis) {
		WRITE("declfn: %.*s;\n", tokens->current.length, tokens->current.text);
		_lang_parse_function(parser, tokens);
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
}

static
void _lang_parse_while(lang_parser* parser, lang_tokenizer* tokens) {
	assert(tokens->current.type == lang_token_while);

	parser->pfnWhile(parser, &tokens->current);
	_lang_skip_token_expect(parser, tokens, lang_token_open_parenthesis, "Expected opening paranthesis after while keyword: while(<expression>)");
	_lang_parse_expression(parser, tokens, 0);
	if(_lang_current_token_expect(parser, tokens, lang_token_close_parenthesis, "No closing parenthesis ')' to while condition"))
		_lang_next_token(parser, tokens);
	parser->pfnLoopBody(parser, &tokens->current);
	_lang_parse_statement(parser, tokens);
	parser->pfnEndLoop(parser, &tokens->current);
}

static
int _lang_is_foreach_loop(lang_tokenizer* tokens) {
	lang_token start = tokens->current;

	assert(tokens->current.type == lang_token_for);
	printf("Is foreach loop %s:%i:%i?\n", tokens->current.file, tokens->current.line + 1, tokens->current.character + 1);

	_lang_next_token_quiet(tokens);
	if(tokens->current.type == lang_token_open_parenthesis)
		_lang_next_token_quiet(tokens);

	while(1) {
		if(tokens->current.type != lang_token_name)
			goto REGULAR_FOR_LOOP;

		_lang_next_token_quiet(tokens);
		if(tokens->current.type == lang_token_comma) {
			_lang_next_token_quiet(tokens);
			continue;
		}
		else if(tokens->current.type == lang_token_in)
			goto FOREACH_LOOP;
		else
			goto REGULAR_FOR_LOOP;
	}

	FOREACH_LOOP:
		puts("Yes");
		tokens->pfnRewind(tokens, &start);
		return 1;

	REGULAR_FOR_LOOP:
		puts("No");
		tokens->pfnRewind(tokens, &start);
		return 0;
}

static
void _lang_parse_for(lang_parser* parser, lang_tokenizer* tokens) {
	assert(tokens->current.type == lang_token_for);

	int is_foreach = _lang_is_foreach_loop(tokens);
	lang_token for_keyword = tokens->current;
	_lang_skip_token_expect(parser, tokens, lang_token_open_parenthesis, "Expected for loop parameters to be enclosed in parantheses: for(<stuff...>)");

	if(is_foreach) {
		parser->pfnForeach(parser, &for_keyword);
		while(1) {
			if(tokens->current.type != lang_token_name) {
				_lang_parser_error("Expected for each value name: 'for(<name> /*<- this*/ in <collection>)' or 'for(<name>, <name> /*<- this */ in <collection>)'", parser, &tokens->current);
				// Try to recover from error
				while(tokens->current.type != lang_token_close_parenthesis) {
					if(tokens->current.type == lang_token_end_of_file) {
						_lang_parser_error("Failed recovering from previous error: Expected a closing paranthesis ')' for the for loop somewhere, got end of file", parser, &tokens->current);
						break;
					}
					_lang_next_token(parser, tokens);
				}
				break;
			}
			else {
				parser->pfnForeachVariable(parser, &tokens->current);
			}

			_lang_next_token(parser, tokens);
			if(tokens->current.type == lang_token_comma)
				continue;
			else if(tokens->current.type == lang_token_in)
				break;
		}

		_lang_next_token(parser, tokens);
		_lang_parse_expression(parser, tokens, 0);
		_lang_skip_token_expect(parser, tokens, lang_token_close_parenthesis, "Expected closing paranthesis at end of foreach: for(x in collection) /* <- that */");
	}
	else {
		parser->pfnFor(parser, &for_keyword);
		_lang_parse_statement(parser, tokens);
		_lang_parse_expression(parser, tokens, 0);
		_lang_skip_token_expect(parser, tokens, lang_token_end_stmt, "Expected semicolon after for loop condition: for(<initialization>; <loop condition>; /* <- that */; <each iteration>)");
		_lang_parse_expression(parser, tokens, 0);
		_lang_skip_token_expect(parser, tokens, lang_token_open_parenthesis, "No closing parenthesis ')' to for parameters");
	}

	parser->pfnLoopBody(parser, &tokens->current);
	_lang_parse_statement(parser, tokens);
	parser->pfnEndLoop(parser, &tokens->current);
}

static
void _lang_parse_if_else(lang_parser* parser, lang_tokenizer* tokens) {
	assert(tokens->current.type == lang_token_if);

	parser->pfnIf(parser, &tokens->current);

	// Condition
	_lang_skip_token_expect(parser, tokens, lang_token_open_parenthesis, "Expected opening paranthesis '(' after if");
	_lang_parse_expression(parser, tokens, 0);
	_lang_current_token_expect(parser, tokens, lang_token_close_parenthesis, "Expected closing paranthesis ')' at the end of if condition");
	_lang_next_token(parser, tokens);

	// Body
	parser->pfnIfBody(parser);
	_lang_parse_statement(parser, tokens);

	// else
	if(tokens->current.type == lang_token_else) {
		parser->pfnElse(parser, &tokens->current);
		_lang_next_token(parser, tokens);
		_lang_parse_statement(parser, tokens);
	}
	parser->pfnEndIf(parser);
}

static
void _lang_parse_statement(lang_parser* parser, lang_tokenizer* tokens) {
	if(tokens->current.type == lang_token_def) {
		WRITE("\n\t");
		_lang_parse_declaration(parser, tokens);
		WRITE(";\n");
		if(tokens->current.type != lang_token_end_stmt) {
			_lang_parser_error("Expected end of statement ';' after this expression", parser, &tokens->current);
		}
		_lang_next_token(parser, tokens);
	}
	else if(tokens->current.type == lang_token_if) {
		_lang_parse_if_else(parser, tokens);
	}
	else if(tokens->current.type == lang_token_while) {
		_lang_parse_while(parser, tokens);
	}
	else if(tokens->current.type == lang_token_for) {
		_lang_parse_for(parser, tokens);
	}
	else if(tokens->current.type == lang_token_class) {
		_lang_parse_class_definition(parser, tokens);
	}
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

LANG_PARSER_API
void lang_parser_parse(lang_parser* parser, lang_tokenizer* tokens) {
	_lang_next_token(parser, tokens); // Get first token

	while(tokens->current.type != lang_token_end_of_file) {
		parser->pfnNextStatement(parser);
		_lang_parse_statement(parser, tokens);
	}
}

static
void _lang_default_print_error_fn(lang_parser* p, const char* fmt, ...) {
	va_list myargs;
	va_start(myargs, fmt);
	vfprintf(stderr, fmt, myargs);
	va_end(myargs);
}

static
void _lang_default_void_error_fn(lang_parser* p, const char* fmt, ...) {}

static void _lang_default_ignore0(lang_parser* p) {}
static void _lang_default_ignore1(lang_parser* p, lang_token const* where) {}

LANG_PARSER_API
lang_parser lang_parser_defaults(int flags) {
	int printErrors = flags & lang_parser_defaults_print_errors;

	lang_parser result = {
		.pfnComment            = _lang_default_ignore1,
		.pfnBeginSubexpression = _lang_default_ignore1,
		.pfnEndSubexpression   = _lang_default_ignore1,
		.pfnBinop              = _lang_default_ignore1,
		.pfnBeginCall          = _lang_default_ignore1,
		.pfnNextCallArgument   = _lang_default_ignore1,
		.pfnEndCall            = _lang_default_ignore1,
		.pfnValue              = _lang_default_ignore1,
		.pfnBeginFunction      = _lang_default_ignore1,
		.pfnFuncArgument       = _lang_default_ignore1,
		.pfnEndFunction        = _lang_default_ignore1,
		.pfnNextStatement      = _lang_default_ignore0,
		.pfnDeclare            = _lang_default_ignore1,
		.pfnInitDeclaration    = _lang_default_ignore0,
		.pfnBeginClass         = _lang_default_ignore1,
		.pfnEndClass           = _lang_default_ignore1,

		.pfnError = printErrors?_lang_default_print_error_fn:_lang_default_void_error_fn,
	};
	return result;
}
