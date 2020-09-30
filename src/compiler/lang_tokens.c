#include "lang_tokens.h"

#include "./lang_parse_util.h"

#include <string.h> // strlen
#include <stdio.h> // TODO: remove (printf)

const char* lang_token_names[] = {
	#define LANG_TOKEN(NAME) #NAME
	#include "./lang_tokens.txt"
	#undef LANG_TOKEN
};

typedef lang_token_type  token_type;
typedef lang_token       token;
typedef lang_tokenizer tokenizer;

static inline int _is_numeric(int c) {
	return
		(c >= '0' && c <= '9') ||
		c == '.';
}

static inline int _is_name(int c) {
	return
		(c >= 'a' && c <= 'z') ||
		(c >= 'A' && c <= 'Z');
}

static
void _nextToken(lang_tokenizer* userdata) {
	lang_token* token = &userdata->token;

	token->text += token->length;
	token->character += token->length;
	token->length = -1;

	{
		const char* lineStart = token->text - token->character;
		const char* nextStart = token->text;
		while(nextStart[0] != '\0' && nextStart[0] <= ' ') {
			if(
				nextStart[0] == '\r' && nextStart[1] == '\n' || // Windows line ending
				nextStart[0] == '\n')
			{
				if(nextStart[0] == '\r') nextStart++; // Skip \r of windows line ending

				// TODO: handle windows line endings
				lineStart = nextStart;
				token->line++;
			}
			nextStart++;
		}
		token->character = nextStart - lineStart;
		token->text = nextStart;
	}

	// The starts_with and strlen calls should be optimized away
	#define TOKEN(NAME, TOKEN_TYPE) if(lang_starts_with(NAME, token->text)) { token->type = (TOKEN_TYPE); token->length = strlen(NAME); }
	// Special
	TOKEN("\"", lang_token_quote) else
	TOKEN(";", lang_token_end_stmt) else
	// Brackets
	TOKEN("(", lang_token_open_brace) else
	TOKEN(")", lang_token_close_brace) else
	TOKEN("[", lang_token_open_square) else
	TOKEN("]", lang_token_close_square) else
	TOKEN("{", lang_token_open_curly) else
	TOKEN("}", lang_token_close_curly) else
	// Operators: Various
	TOKEN(".",  lang_token_dot) else
	TOKEN(",",  lang_token_comma) else
	TOKEN("=",  lang_token_assign) else
	// Operators: Comparison
	TOKEN("==", lang_token_equals) else
	TOKEN("!=", lang_token_not_equals) else
	TOKEN("<",  lang_token_less) else
	TOKEN(">",  lang_token_greater) else
	TOKEN("<=", lang_token_less_equal) else
	TOKEN(">=", lang_token_greater_equal) else
	// Operators: Math
	TOKEN("+",  lang_token_plus) else
	TOKEN("+=", lang_token_plus_assign) else
	TOKEN("-",  lang_token_minus) else
	TOKEN("-=", lang_token_minus_assign) else
	TOKEN("/",  lang_token_div) else
	TOKEN("/=", lang_token_div_assign) else
	TOKEN("*",  lang_token_mul) else
	TOKEN("*=", lang_token_mul_assign) else
	// Keywords
	TOKEN("class", lang_token_class) else
	TOKEN("var",   lang_token_var) else
	TOKEN("pub",   lang_token_pub) else
	TOKEN("get",   lang_token_get) else
	TOKEN("set",   lang_token_set)
	#undef TOKEN
	// Special
	else if(token->text[0] == '\0') {
		token->type = lang_token_end_of_file;
		token->length = 0;
	}
	// Number
	else if(_is_numeric(token->text[0])) {
		token->type = lang_token_number;
		token->length = 1;
		while(_is_numeric(token->text[token->length]))
			token->length++;
	}
	else if(_is_name(token->text[0])) {
		token->type = lang_token_name;
		token->length = 1;
		while(_is_name(token->text[token->length]))
			token->length++;
	}

	// printf(
	// 	"%s:%i:%i: %.*s -> %s\n",
	// 	token->file, token->line, token->character,
	// 	token->length, token->text,
	// 	lang_token_names[token->type]
	// );
}

void lang_tokenizer_init(
	lang_tokenizer* stream,
	const char* text,
	const char* filepath_or_null)
{
	stream->token.type      = lang_token_end_of_file;
	stream->token.file      = filepath_or_null;
	stream->token.line      = 1;
	stream->token.character = 0;
	stream->token.text      = text;
	stream->token.length    = 0;

	stream->pfnNextToken = &_nextToken;
}
