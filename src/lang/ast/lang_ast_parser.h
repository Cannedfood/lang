#include "../parser/lang_parser.h"
#include "../util/lang_allocator.h"

#include "lang_ast.h"

enum lang_ast_parser_flag {
	lang_ast_parser_flag_include_comments = 1
};

typedef struct lang_ast_parser lang_ast_parser;
struct lang_ast_parser {
	lang_parser parser;
	lang_alloc_callbacks allocator;
	lang_ast_node* current;
	lang_ast_node* root;
};

LANG_AST_API
lang_ast_parser lang_create_parser_ast(lang_alloc_callbacks alloc, unsigned lang_ast_parser_flags);
