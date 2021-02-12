#include "lang_ast_nodes.h"

LANG_AST_API const char* lang_ast_type_names[] = {
	#define LANG_AST_NODE(NAME, BODY) #NAME,
		#include "lang_ast_nodes.txt"
	#undef LANG_AST_NODE
};
