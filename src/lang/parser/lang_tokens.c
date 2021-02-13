#include "lang_tokens.h"

#include "./lang_parse_util.h"

#include <string.h> // strlen
#include <stdio.h> // TODO: remove (printf)

LANG_TOKENIZER_API
const char* lang_token_names[] = {
	#define LANG_TOKEN(NAME) #NAME
	#include "./lang_tokens.txt"
	#undef LANG_TOKEN
};

typedef lang_token_type  token_type;
typedef lang_token       token;
typedef lang_tokenizer tokenizer;

static inline int _lang_is_numeric(int c) {
	return
		(c >= '0' && c <= '9') ||
		c == '.';
}

static inline int _lang_is_name(int c) {
	return
		(c >= 'a' && c <= 'z') ||
		(c >= 'A' && c <= 'Z');
}

static
void _lang_tokenizer_skip_over_token(lang_token* token) {
	// Add number of lines in token to token->lines
	int mayContainNewline =
		token->type == lang_token_block_comment ||
		token->type == lang_token_string_literal;

	if(!mayContainNewline) {
		token->text += token->length;
		token->character += token->length;
	}
	else {
		const char* lineStart = token->text - token->character;
		const char* end = token->text + token->length;
		const char* s = token->text;
		while(s[0] && s < end) {
			if(s[0] == '\n') {
				token->line++;
				lineStart = s + 1;
			}
			else if(s[0] == '\r') {
				lineStart = s + 1;
			}
			s++;
		}
		token->text += token->length;
		token->character = token->text - lineStart;
	}

	token->length = -1;
}

static
void _lang_tokenizer_skip_over_whitespace(lang_token* token) {
	// Skip whitespace
	const char* lineStart = token->text - token->character; // Used to calculate the column afterwards
	const char* nextStart = token->text; // Start of next token
	while(nextStart[0] != '\0' && nextStart[0] <= ' ') {
		if(nextStart[0] == '\n') {
			token->line++;
			lineStart = nextStart + 1;
		}
		else if(nextStart[0] == '\r') {
			lineStart = nextStart + 1;
		}

		nextStart++;
	}

	token->character = nextStart - lineStart;
	token->text = nextStart;
}

static
const char* _lang_tokenizer_end_of_line(const char* text) {
	while(text[0] && !lang_starts_with("\r\n", text) && !lang_starts_with("\n", text)) {
		text++;
	}
	return text;
}

static
const char* _lang_tokenizer_end_of_string_literal(const char* text) {
	const char quote = text[0];

	text++;
	while(text[0] && text[0] != quote) {
		if(text[0] == '\\' && text[1] != '\0') {
			// Skip escaped stuff
			if(lang_starts_with(text, "\\\r\n"))
				text += 3;
			else
				text += 2;
		}
		else {
			text += 1;
		}
	}

	if(text[0] == quote) {
		text++;
	}

	return text;
}

static
const char* _lang_tokenizer_end_of_block_comment(const char* text) {
	// TODO: assert(lang_starts_with("/*", text));
	text += 2;
	while(text[0]) {
		if(lang_starts_with("*/", text)) {
			return text + 2;
		}
		text++;
	}
	return text;
}

static
void _lang_tokenizer_next_token(lang_tokenizer* userdata) {
	lang_token* token = &userdata->current;

	_lang_tokenizer_skip_over_token(token);
	_lang_tokenizer_skip_over_whitespace(token);

	// The starts_with and strlen calls should be optimized away
	#define TOKEN(NAME, TOKEN_TYPE) if(lang_starts_with(NAME, token->text)) { token->type = (TOKEN_TYPE); token->length = strlen(NAME); }

	// Comments
	if(lang_starts_with("//", token->text) || lang_starts_with("#", token->text)) {
		token->type = lang_token_line_comment;
		token->length = _lang_tokenizer_end_of_line(token->text) - token->text;
	}
	else if(lang_starts_with("/*", token->text)) {
		token->type = lang_token_block_comment;
		token->length = _lang_tokenizer_end_of_block_comment(token->text) - token->text;
	}
	// Special
	else TOKEN(";",  lang_token_end_stmt)
	// Brackets
	else TOKEN("(", lang_token_open_parenthesis)
	else TOKEN(")", lang_token_close_parenthesis)
	else TOKEN("[", lang_token_open_bracket)
	else TOKEN("]", lang_token_close_bracket)
	else TOKEN("{", lang_token_open_curly)
	else TOKEN("}", lang_token_close_curly)
	// Operators: Various
	else TOKEN(".",  lang_token_dot)
	else TOKEN(",",  lang_token_comma)
	else TOKEN(":",  lang_token_colon)
	else TOKEN("=",  lang_token_assign)
	// Operators: Comparison
	else TOKEN("==", lang_token_equals)
	else TOKEN("!=", lang_token_not_equals)
	else TOKEN("<",  lang_token_less)
	else TOKEN(">",  lang_token_greater)
	else TOKEN("<=", lang_token_less_equal)
	else TOKEN(">=", lang_token_greater_equal)
	// Operators: Math
	else TOKEN("+",  lang_token_plus)
	else TOKEN("+=", lang_token_plus_assign)
	else TOKEN("-",  lang_token_minus)
	else TOKEN("-=", lang_token_minus_assign)
	else TOKEN("/",  lang_token_div)
	else TOKEN("/=", lang_token_div_assign)
	else TOKEN("*",  lang_token_mul)
	else TOKEN("*=", lang_token_mul_assign)
	// Keywords
	else TOKEN("class", lang_token_class)
	else TOKEN("var",   lang_token_var)
	else TOKEN("pub",   lang_token_pub)
	else TOKEN("get",   lang_token_get)
	else TOKEN("set",   lang_token_set)
	else TOKEN("new",   lang_token_new)
	#undef TOKEN
	// Quote
	else if(lang_starts_with("\"", token->text) || lang_starts_with("'", token->text)) {
		token->type = lang_token_string_literal;
		token->length = _lang_tokenizer_end_of_string_literal(token->text) - token->text;
	}
	// Special
	else if(token->text[0] == '\0') {
		token->type = lang_token_end_of_file;
		token->length = 0;
	}
	// Number
	else if(_lang_is_numeric(token->text[0])) {
		token->type = lang_token_number;
		token->length = 1;
		while(_lang_is_numeric(token->text[token->length]))
			token->length++;
	}
	else if(_lang_is_name(token->text[0])) {
		token->type = lang_token_name;
		token->length = 1;
		while(_lang_is_name(token->text[token->length]))
			token->length++;
	}

	// printf(
	// 	"%s:%i:%i: %.*s -> %s\n",
	// 	token->file, token->line, token->character,
	// 	token->length, token->text,
	// 	lang_token_names[token->type]
	// );
}

LANG_TOKENIZER_API
void lang_tokenizer_init(
	lang_tokenizer* stream,
	const char* text,
	const char* file_name_or_null)
{
	stream->current.type      = lang_token_end_of_file;
	stream->current.file      = file_name_or_null;
	stream->current.line      = 0;
	stream->current.character = 0;
	stream->current.text      = text;
	stream->current.length    = 0;

	stream->pfnNextToken = &_lang_tokenizer_next_token;
}
