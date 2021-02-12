#pragma once

#include "../config.h"

#include "../parser/lang_tokens.h"

// Node types
typedef enum lang_ast_type lang_ast_type;
enum lang_ast_type {
	#define LANG_AST_NODE(NAME, BODY) lang_ast_type_##NAME,
		#include "lang_ast_nodes.txt"
	#undef LANG_AST_NODE
	lang_num_ast_types
};

LANG_AST_API_DECL const char* lang_ast_type_names[];

// Base node
typedef struct lang_ast_node lang_ast_node;
struct lang_ast_node {
	lang_ast_type type;
};

// Forward declarations
#define LANG_AST_NODE(NAME, MEMBERS) typedef struct lang_ast_node_##NAME lang_ast_node_##NAME;
	#include "lang_ast_nodes.txt"
#undef LANG_AST_NODE

// Definitions
#define LANG_AST_NODE(NAME, MEMBERS) \
    struct lang_ast_node##NAME {     \
        lang_ast_type type;          \
        MEMBERS                      \
    };

	#include "lang_ast_nodes.txt"

#undef LANG_AST_NODE
