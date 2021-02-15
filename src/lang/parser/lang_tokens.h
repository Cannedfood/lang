#pragma once

#include "../config.h"

// Token
enum lang_token_type {
	#define LANG_TOKEN(NAME) lang_token_##NAME
	#include "./lang_tokens.txt"
	#undef LANG_TOKEN

	lang_num_token_types
};
typedef enum lang_token_type lang_token_type;

LANG_TOKENIZER_API_DECL
const char* lang_token_names[lang_num_token_types];

struct lang_token {
	lang_token_type type;

	const char*     file;
	int             line, character;

	const char*     text;
	int             length;


	#ifdef __cplusplus
	std::string_view string_view() const noexcept { return std::string_view(text, length); }
	#endif
};
typedef struct lang_token lang_token;

// Token Stream
typedef struct lang_tokenizer lang_tokenizer;
struct lang_tokenizer {
	lang_token current;
	void* userdata;

	void(*pfnNextToken)(lang_tokenizer* tokenizer);

	#ifdef __cplusplus
	void next() noexcept { this->pfnNextToken(this); }
	#endif
};

LANG_TOKENIZER_API
lang_tokenizer lang_tokenizer_create(
	const char* text,
	int length LANG_DEFAULT(-1),
	const char* filename LANG_DEFAULT(NULL));
