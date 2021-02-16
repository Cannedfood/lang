#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>

#include <lang/util/lang_buffer.h>
#include <lang/parser/lang_tokens.h>
#include <lang/parser/lang_parser.h>
#include <lang/ast/lang_ast_parser.h>

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

void printAst(lang_ast_node* node, int indentN, int depth) {
	while(node) {
		switch (node->type) {
		case lang_ast_type_if:
			printf("%*sif\n", depth, "");
			printAst(node->as_if.condition, indentN, depth + indentN);
			printf("%*sthen\n", depth, "");
			printAst(node->as_if.if_true, indentN, depth + indentN);
			if(node->as_if.if_false) {
				printf("%*selse:\n", depth, "");
				printAst(node->as_if.if_false, indentN, depth + indentN);
			}
			break;
		case lang_ast_type_comment:
			printf("%*scomment %.*s\n", depth, "", node->as_comment.value.length, node->as_comment.value.text);
		break;
		case lang_ast_type_block:
			printf("%*sblock\n", depth, "");
			printAst(node->as_block.content, indentN, depth + indentN);
		break;
		case lang_ast_type_class:
			printf("%*sclass\n", depth, "");
			printAst(node->as_block.content, indentN, depth + indentN);
		break;
		case lang_ast_type_function:
			printf("%*sfunction\n", depth, "");
			printAst(node->as_function.arguments, indentN, depth + indentN);
			printf("%*s body:\n", depth, "");
			printAst(node->as_block.content, indentN, depth + indentN);
		break;
		case lang_ast_type_declaration:
			printf("%*sdef %.*s\n", depth, "", node->as_declaration.name.length, node->as_declaration.name.text);
			printAst(node->as_declaration.initial_value, indentN, depth + indentN);
		break;
		case lang_ast_type_expression:
			printf("%*sexpression\n", depth, "");
			printAst(node->as_expression.children, indentN, depth + indentN);
		break;
		case lang_ast_type_binop:
			printf("%*sbinop %.*s\n", depth, "", node->as_binop.op.length, node->as_binop.op.text);
			printAst(node->as_binop.left, indentN, depth + indentN);
			printAst(node->as_binop.right, indentN, depth + indentN);
		break;
		case lang_ast_type_unop:
			printf("%*sunop %.*s\n", depth, "", node->as_unop.op.length, node->as_unop.op.text);
			printAst(node->as_unop.expression, indentN, depth + indentN);
		break;
		case lang_ast_type_call:
			printf("%*scall\n", depth, "");
			printAst(node->as_call.target, indentN, depth + indentN);
			if(node->as_call.arguments) {
				printf("%*swith arguments:\n", depth, "");
				printAst(node->as_call.arguments, indentN, depth + indentN);
			}
		break;
		case lang_ast_type_value:
			printf("%*svalue %.*s\n", depth, "", node->as_value.value.length, node->as_value.value.text);
		break;
		case lang_num_ast_types: LANG_UNREACHABLE; break;
		}

		node = node->next;
	}
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
	lang_allocator* allocator = lang_new_allocator();

	lang_tokenizer  tokenizer = lang_tokenizer_create(fileContents.data, fileContents.length, filepath);

	lang_alloc_callbacks alloc  = lang_alloc_callbacks_for(allocator);
	lang_ast_parser      parser = lang_create_parser_ast(alloc, 0);
	lang_parser_parse(&parser.parser, &tokenizer);

	printAst(parser.root->as_block.content, 4, 0);

	lang_free_allocator(allocator);

	// Done
	lang_buffer_destroy(&fileContents);
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
