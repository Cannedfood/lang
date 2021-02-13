#pragma once

#include "../config.h"

static inline
int lang_is_whitespace(int c) {
	return c <= ' ';
}

static inline
int lang_starts_with(const char* with, const char* text) {
	for (int i = 0; with[i] != '\0'; i++) {
		if(with[i] != text[i]) return 0;
	}
	return 1;
}

static inline
int lang_starts_with_e(const char* with, const char* text, const char* end) {
	for (int i = 0; with[i] != '\0'; i++) {
		if((text + i) >= end) return 0;
		if(with[i] != text[i]) return 0;
	}
	return 1;
}

static inline
int lang_starts_with_keyword_e(const char* with, const char* text, const char* end) {
	for (int i = 0; text + i < end; i++) {
		if(with[i] == '\0') {
			return lang_is_whitespace(text[i]);
		}
		else if(with[i] != text[i]) {
			return 0;
		}
	}
	return 0;
}
