#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>

#include <lang/util/lang_buffer.h>
#include <lang/parser/lang_tokens.h>
#include <lang/parser/lang_parser.h>

#include "print-parser.h"

lang_buffer readFile(const char* path) {
	lang_buffer buf = lang_buffer_new();

	FILE* file = fopen(path, "rb");
	if(!file) {
		fprintf(stderr, "Failed opening '%s'\n", path);
		exit(-1);
	}
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

	// Print tokens
	// puts("-- Tokens ------------------");
	// lang_tokenizer tokenizer = lang_tokenizer_create(fileContents.data, fileContents.length, filepath);
	// do {
	// 	tokenizer.pfnNextToken(&tokenizer);
	// 	char buffer[20];
	// 	sprintf(buffer,
	// 		"%s:%i:%i:",
	// 		tokenizer.current.file, tokenizer.current.line+1, tokenizer.current.character+1
	// 	);

	// 	printf(
	// 		"%-20s %-20s '%.*s'\n",
	// 		buffer,
	// 		lang_token_names[tokenizer.current.type],
	// 		tokenizer.current.length, tokenizer.current.text
	// 	);
	// } while(tokenizer.current.type != lang_token_end_of_file);

	// puts("-- Parsed ------------------");
	lang_tokenizer tokenizer = lang_tokenizer_create(fileContents.data, fileContents.length, filepath);
	lang_parser    parser    = print_parser_create();

	lang_parser_parse(&parser, &tokenizer);

	// Done
	lang_buffer_free(&fileContents);
}

int main(int argc, const char** argv) {
	if(argc == 1) {
		const char* programName = argv[0];
		printf("Usage:\n");
		printf("	%s [options] [files...]\n", programName);
		printf("\n");
		printf("Examples:\n");
		printf("	%s file.lang               # Execute file\n", programName);
		printf("	%s --tokenize file.lang    # Tokenize file\n", programName);
		printf("	%s --ast file.lang         # Print AST of file\n", programName);
		printf("\n");
		printf("Options:\n");
		printf("	--help, -h, -?    Print this help\n");
		printf("	--tokenize        Tokenize the file\n");
		printf("	--ast             Print parser-AST (ast before transformations like operator order etc.)\n");
		return -1;
	}
	parseFile(argv[1]);
}
