#pragma once

static inline
int lang_starts_with(const char* with, const char* text) {
	for (int i = 0; with[i] != '\0'; i++) {
		if(with[i] != text[i]) return 0;
	}
	return 1;
}
