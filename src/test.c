#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>

#include "./util/lang_buffer.h"
#include "./runtime/lang_statemachine.h"
#include "./compiler/lang_bytecode.h"
#include "./compiler/lang_tokens.h"

lang_buffer readFile(const char* path) {
	lang_buffer buf = lang_buffer_new();

	FILE* file = fopen(path, "r");
	fseek(file, 0, SEEK_END);
	lang_buffer_reserve(&buf, buf.length = ftell(file));
	fseek(file, 0, SEEK_SET);
	fread(buf.data, buf.length, 1, file);
	fclose(file);

	return buf;
}

void printTokens(const char* data, const char* filepath) {
	lang_tokenizer_string tokenizer;
	lang_tokenizer_string_init(&tokenizer, data, filepath);

	lang_token token;
	do {
		tokenizer.stream.pfnNextToken(tokenizer.stream.userdata, &token);
		printf("%s '%.*s'\n", lang_token_names[token.type], token.length, token.text);
	} while(token.type != lang_token_end_of_file);
}

int main(int argc, const char** argv) {
	const char* path = "./test.lang";

	lang_buffer fileData = readFile(path);
	lang_buffer_pushc(&fileData, '\0');

	printTokens(fileData.data, path);

	lang_buffer_free(&fileData);
}
