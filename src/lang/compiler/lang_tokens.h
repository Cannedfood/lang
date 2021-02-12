#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __cplusplus
	#define LANG_DEFAULT(VALUE) = VALUE
#else
	#define LANG_DEFAULT(VALUE)
#endif

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

	#ifdef __cplusplus
	std::string_view string_view() const noexcept { return std::string_view(text, length); }
	#endif
};
typedef struct lang_token lang_token;

// Token Stream
typedef struct lang_tokenizer lang_tokenizer;
struct lang_tokenizer {
	lang_token token;

	void(*pfnNextToken)(lang_tokenizer* tokenizer);

	#ifdef __cplusplus
	void next() noexcept { this->pfnNextToken(this); }
	#endif
};

void lang_tokenizer_init(
	lang_tokenizer* stream,
	const char* text,
	const char* file_name_or_null  LANG_DEFAULT(nullptr));

#undef LANG_DEFAULT

#ifdef __cplusplus
} // extern "C"
#endif
