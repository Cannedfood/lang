#pragma once

// Token
enum lang_token_type {
	#define LANG_TOKEN(NAME) lang_token_##NAME
	#include "./lang_tokens.txt"
	#undef LANG_TOKEN
};
typedef enum lang_token_type lang_token_type;

extern const char* lang_token_names[];

struct lang_token {
	lang_token_type type;
	const char* file;
	int line, character;
	const char* token;
	int length;
};
typedef struct lang_token lang_token;

// Token Stream
struct lang_token_stream {
	void* userdata;
	void(*pfnNextToken)(void* userdata, lang_token* into);
};
typedef struct lang_token_stream lang_token_stream;

// Token Stream String
struct lang_token_stream_string {
	lang_token_stream stream;
	const char* file;
	int line;
	const char* lineStart;
	const char* current;
};
typedef struct lang_token_stream_string lang_token_stream_string;
void lang_parser_init_token_stream_string(lang_token_stream_string* stream, const char* text, const char* filepath_or_null);
