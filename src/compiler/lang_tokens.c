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
typedef lang_token_stream token_stream;

static
void _token_stream_string_nextToken(void* userdata, lang_token* token) {
	lang_token_stream_string* stream = userdata;

	while(stream->current[0] == '\0' && stream->current[0] <= ' ') {
		if(stream->current[0] == '\n') {
			stream->lineStart = stream->current;
			stream->line++;
		}
	}

	token->line  = stream->line;
	token->token = stream->current;
	token->file  = stream->file;
	token->character = stream->lineStart - stream->current;

	token->token = stream->current;

	#define TOKEN(NAME, TOKEN_TYPE) if(lang_starts_with(NAME, stream->current)) { token->type = (TOKEN_TYPE); token->length = strlen(NAME); }

	// Brackets
	TOKEN("(",  lang_token_open_brace) else
	TOKEN(")",  lang_token_close_brace) else
	TOKEN("[",  lang_token_open_bracket) else
	TOKEN("]",  lang_token_close_bracket) else
	// Operators
	TOKEN("=",  lang_token_plus) else
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
	// Special
	TOKEN("\0", lang_token_end_of_file)
	// Default
	else {

	}

	#undef TOKEN

	if(token->token[0])
		stream->current += token->length;
}

void lang_parser_init_token_stream_string(
	lang_token_stream_string* stream,
	const char* text,
	const char* filepath_or_null)
{
	stream->current = text;
	stream->lineStart = text;
	stream->line = 0;
	stream->file = filepath_or_null;

	stream->stream.userdata     = stream;
	stream->stream.pfnNextToken = _token_stream_string_nextToken;
}
