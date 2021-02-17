#pragma once

#include "../config.h"

#include "../parser/lang_tokens.h"

#define LANG_AST_STRUCT(SHORTNAME) lang_ast_##SHORTNAME
#define LANG_AST_TAG(SHORTNAME) lang_ast_type_##SHORTNAME

// Node types
typedef enum lang_ast_type lang_ast_type;
enum lang_ast_type {
	#define LANG_AST_NODE(NAME, BODY) LANG_AST_TAG(NAME),
		#include "lang_ast_nodes.txt"
	#undef LANG_AST_NODE
	lang_num_ast_types
};

LANG_AST_API_DECL const char* LANG_AST_TAGs[];

// Base node

// Forward declarations, typedefs
typedef union lang_ast_node lang_ast_node;
#define LANG_AST_NODE(NAME, MEMBERS) typedef struct LANG_AST_STRUCT(NAME) LANG_AST_STRUCT(NAME);
	#include "lang_ast_nodes.txt"
#undef LANG_AST_NODE

// Definitions
#define LANG_AST_NODE(NAME, MEMBERS)         \
    struct LANG_AST_STRUCT(NAME) {             \
        lang_ast_type type;                  \
        lang_ast_node *parent, *next, *prev; \
        MEMBERS                              \
    };

	#include "lang_ast_nodes.txt"

#undef LANG_AST_NODE

union lang_ast_node {
	struct { // Common data
		lang_ast_type type;
		lang_ast_node *parent, **self, *next;
	};

	#define LANG_AST_NODE(NAME, MEMBERS) LANG_AST_STRUCT(NAME) as_##NAME;
		#include "lang_ast_nodes.txt"
	#undef LANG_AST_NODE
};

LANG_AST_API lang_ast_node* lang_ast_append (lang_ast_node* parent, lang_ast_node** list, lang_ast_node* newElement);
LANG_AST_API lang_ast_node* lang_ast_prepend(lang_ast_node* parent, lang_ast_node** list, lang_ast_node* newElement);
LANG_AST_API void           lang_insert_after(lang_ast_node* where, lang_ast_node* node);
LANG_AST_API void           lang_insert_before(lang_ast_node* where, lang_ast_node* node);
LANG_AST_API lang_ast_node* lang_ast_remove (lang_ast_node*);
LANG_AST_API lang_ast_node* lang_ast_replace(lang_ast_node* target, lang_ast_node* with);
LANG_AST_API lang_ast_node* lang_ast_last_sibling(lang_ast_node* list);
