#include "lang_tokens.h"

#include "./lang_parse_util.h"

#include <string.h> // strlen

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
void _tokenizer_string_nextToken(void* userdata, lang_token* token) {
	lang_tokenizer_string* stream = userdata;

	while(stream->current[0] != '\0' && stream->current[0] <= ' ') {
		if(stream->current[0] == '\n') {
			stream->lineStart = stream->current;
			stream->line++;
		}
		stream->current++;
	}

	token->line      = stream->line;
	token->text      = stream->current;
	token->file      = stream->file;
	token->character = stream->lineStart - stream->current;

	// The starts_with and strlen calls should be optimized away
	#define TOKEN(NAME, TOKEN_TYPE) if(lang_starts_with(NAME, stream->current)) { token->type = (TOKEN_TYPE); token->length = strlen(NAME); }

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
	TOKEN("var",   lang_token_var)
	// Special
	else if(stream->current[0] == '\0') {
		token->type = lang_token_end_of_file;
		token->length = 0;
	}
	// Number
	else if(_is_numeric(stream->current[0])) {
		token->type = lang_token_number;
		token->length = 1;
		while(_is_numeric(stream->current[token->length]))
			token->length++;
	}
	else if(_is_name(stream->current[0])) {
		token->type = lang_token_name;
		token->length = 1;
		while(_is_name(stream->current[token->length]))
			token->length++;
	}

#undef TOKEN

	stream->current += token->length;
}

void lang_tokenizer_string_init(
	lang_tokenizer_string* stream,
	const char* text,
	const char* filepath_or_null)
{
	stream->current = text;
	stream->lineStart = text;
	stream->line = 0;
	stream->file = filepath_or_null;

	stream->stream.userdata     = stream;
	stream->stream.pfnNextToken = _tokenizer_string_nextToken;
}
