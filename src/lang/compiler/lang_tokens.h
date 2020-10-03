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
typedef struct lang_tokenizer lang_tokenizer;
struct lang_tokenizer {
	lang_token token;

	void(*pfnNextToken)(lang_tokenizer* tokenizer);
};

void lang_tokenizer_init(lang_tokenizer* stream, const char* text, const char* filepath_or_null);
