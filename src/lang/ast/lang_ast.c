#include "lang_ast.h"

#include <stddef.h>
#include <assert.h>

LANG_AST_API const char* LANG_AST_TAGs[] = {
	#define LANG_AST_NODE(NAME, BODY) #NAME,
		#include "lang_ast_nodes.txt"
	#undef LANG_AST_NODE
};

static
lang_ast_node** _lang_last_next_pointer(lang_ast_node** list) {
	lang_ast_node** result = list;
	while(*result != NULL)
		result = &(*result)->next;
	return result;
}

LANG_AST_API void lang_ast_append (lang_ast_node* parent, lang_ast_node** list, lang_ast_node* newElement) {
	lang_ast_node** p = _lang_last_next_pointer(list);
	newElement->parent = parent;
	newElement->self = p;
	newElement->next = NULL;

	*newElement->self = newElement;
}
LANG_AST_API void lang_ast_prepend(lang_ast_node* parent, lang_ast_node** list, lang_ast_node* newElement) {
	assert(newElement);

	lang_ast_remove(newElement);

	lang_ast_node* next = *list;
	*list = newElement;
	newElement->parent = parent;
	// TODO
}
LANG_AST_API void lang_insert_after(lang_ast_node* where, lang_ast_node* node) {
	assert(where);
	assert(where->self&&"lang_insert_after: 'where' argument must be in a list!");
	assert(node);
	lang_ast_remove(node);
}
LANG_AST_API void lang_insert_before(lang_ast_node* where, lang_ast_node* node) {
	assert(where);
	assert(where->self&&"lang_insert_before: 'where' argument must be in a list!");
	assert(node);
	lang_ast_remove(node);
}
LANG_AST_API lang_ast_node* lang_ast_remove (lang_ast_node* node) {
	assert(node);
	if(node->self) *node->self = node->next;
	if(node->next) node->next->self = node->self;
	node->parent = NULL;
	node->self   = NULL;
	node->next   = NULL;
	return node;
}
LANG_AST_API lang_ast_node* lang_ast_replace(lang_ast_node* target, lang_ast_node* with) {
	with->parent = target->parent;
	with->self   = target->self;
	with->next   = target->next;

	target->parent = NULL;
	target->self   = NULL;
	target->next   = NULL;

	*with->self = with;
	if(with->next) {
		with->next->self = &with->next;
	}

	return target;
}

LANG_AST_API lang_ast_node* lang_ast_last(lang_ast_node* list) {
	if(!list) return NULL;

	while(list->next) {
		list = list->next;
	}

	return list;
}
