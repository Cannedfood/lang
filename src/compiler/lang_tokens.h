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
	const char* text;
	int length;
};
typedef struct lang_token lang_token;

// Token Stream
struct lang_tokenizer {
	void* userdata;
	void(*pfnNextToken)(void* userdata, lang_token* into);
};
typedef struct lang_tokenizer lang_tokenizer;

// Token Stream String
struct lang_tokenizer_string {
	lang_tokenizer stream;
	const char* file;
	int line;
	const char* lineStart;
	const char* current;
};
typedef struct lang_tokenizer_string lang_tokenizer_string;
void lang_tokenizer_string_init(lang_tokenizer_string* stream, const char* text, const char* filepath_or_null);
