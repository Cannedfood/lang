#pragma once

#include "../config.h"

static inline
int lang_is_whitespace_char(int c) {
	return c <= ' ';
}

static inline
int lang_is_numeric_char(int c) {
	return
		(c >= '0' && c <= '9') ||
		c == '.';
}

static inline
int lang_is_name_char(int c) {
	return
		(c >= 'a' && c <= 'z') ||
		(c >= 'A' && c <= 'Z');
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
			return !lang_is_name_char(text[i]);
		}
		else if(with[i] != text[i]) {
			return 0;
		}
	}
	return 0;
}
