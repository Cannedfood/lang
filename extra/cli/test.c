#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>

#include <util/lang_buffer.h>
#include <compiler/lang_tokens.h>
#include <compiler/lang_parser.h>

lang_buffer readFile(const char* path) {
	lang_buffer buf = lang_buffer_new();

	FILE* file = fopen(path, "rb");
	fseek(file, 0, SEEK_END);
	lang_buffer_reserve(&buf, buf.length = ftell(file));
	fseek(file, 0, SEEK_SET);
	int bytes_read = fread(buf.data, 1, buf.length, file);
	assert(bytes_read == buf.length);
	fclose(file);

	return buf;
}

void parseFile(const char* filepath) {
	lang_buffer fileContents = readFile(filepath);

	lang_tokenizer tokenizer;
	lang_tokenizer_init(&tokenizer, fileContents.data, filepath);

	lang_parser parser;
	parser.pfnError = 0; // Default: stderr
	lang_parser_parse(&parser, &tokenizer);

	while(tokenizer.token.type != lang_token_end_of_file)
		tokenizer.pfnNextToken(&tokenizer);

	lang_buffer_free(&fileContents);
}

int main(int argc, const char** argv) {
	parseFile("./test.lang");
}
