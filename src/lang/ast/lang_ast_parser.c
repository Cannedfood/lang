#include "lang_ast_parser.h"

#include <assert.h>
#include <string.h>

lang_ast_node* _lang_alloc_node(lang_alloc_callbacks const* alloc, lang_ast_type type) {
	assert(type != lang_num_ast_types);

	lang_ast_node* result;
	switch (type) {
	#define LANG_AST_NODE(NAME, MEMBERS)        \
	    case LANG_AST_TAG(NAME):                \
	        result = alloc->alloc(              \
	            alloc->userdata,                \
	            sizeof(LANG_AST_STRUCT(NAME))   \
	        );                                  \
	        memset(&result->as_##NAME, 0, sizeof(result->as_##NAME)); \
	        break;

	#include "lang_ast_nodes.txt"

	#undef LANG_AST_NODE
	default: LANG_UNREACHABLE;
	}
	result->type = type;
	return result;
}

static int precedence(lang_token_type op) {
	switch (op) {
	default: return 0;
	case lang_token_plus: case lang_token_minus: return 1;
	case lang_token_div: case lang_token_mul: return 2;
	case lang_token_open_bracket: case lang_token_dot: return 3;
	}
}

static int _lang_is_scope(lang_ast_type type) {
	return type == lang_ast_type_scope || type == lang_ast_type_function || type == lang_ast_type_class;
}

static void _lang_pop_scope(lang_ast_parser* parser, lang_ast_type scopeType) {
	while(parser->current->type != scopeType) {
		assert(parser->current->parent);
		parser->current = parser->current->parent;
	}
	parser->current = parser->current->parent;
	while(!_lang_is_scope(parser->current->type)) {
		assert(parser->current->parent);
		parser->current = parser->current->parent;
	}
}

static lang_ast_node* _lang_ast_append_expression(lang_ast_node* to, lang_ast_node* what) {
	if(what->type == lang_ast_type_binop) {
		assert(to->type == lang_ast_type_value || to->type == lang_ast_type_expression);
		// TODO: enforce operator precedence
		lang_ast_replace(to, what);
		lang_ast_append(what, &what->as_binop.left, what);
		return what;
	}

	if(_lang_is_scope(to->type)) {
		lang_ast_append(to, &to->as_scope.content, what);
		return what;
	}

	LANG_UNREACHABLE;
}

static void _lang_ast_begin_subexpression(lang_parser* parser, lang_token const* token) {
	lang_ast_parser* ast_parser = (lang_ast_parser*)parser;

	lang_ast_node* node = _lang_alloc_node(&ast_parser->allocator, lang_ast_type_expression);
	if(_lang_is_scope(ast_parser->current->type)) {
		lang_ast_append(ast_parser->current, &ast_parser->current->as_scope.content, node);
		ast_parser->current = node;
	}
	else {
		// TODO, skip for now
	}
}
static void _lang_ast_end_subexpression(lang_parser* parser, lang_token const* token) {
	lang_ast_parser* ast_parser = (lang_ast_parser*)parser;
	while(ast_parser->current->type != lang_ast_type_expression) {
		ast_parser->current = ast_parser->current->parent;
	}
	ast_parser->current = ast_parser->current->parent;
}
static void _lang_ast_binop(lang_parser* parser, lang_token const* op) {
	lang_ast_parser* ast_parser = (lang_ast_parser*)parser;

	lang_ast_node* binop = _lang_alloc_node(&ast_parser->allocator, lang_ast_type_binop);
	binop->as_binop.op = *op;

	if(_lang_is_scope(ast_parser->current->type)) {
		lang_ast_append(ast_parser->current, &ast_parser->current->as_scope.content, binop);
		ast_parser->current = binop;
	}
	else if(ast_parser->current->type == lang_ast_type_value) {
		binop->as_binop.left = lang_ast_replace(ast_parser->current, binop);
		ast_parser->current = binop;
	}
}
static void _lang_ast_begin_call(lang_parser* parser, lang_token const* where) {
	lang_ast_parser* ast_parser = (lang_ast_parser*)parser;

	lang_ast_node* node = _lang_alloc_node(&ast_parser->allocator, lang_ast_type_call);
	node->as_call.where = *where;

	if(
		ast_parser->current->type == lang_ast_type_value ||
		ast_parser->current->type == lang_ast_type_binop ||
		ast_parser->current->type == lang_ast_type_expression)
	{
		lang_ast_node* target = lang_ast_replace(ast_parser->current, node);
		lang_ast_append(node, &node->as_call.target, target);
		ast_parser->current = node;
	}
}
static void _lang_ast_next_call_argument(lang_parser* parser, lang_token const* where) {
	lang_ast_parser* ast_parser = (lang_ast_parser*)parser;
	while(ast_parser->current->type != lang_ast_type_call) {
		assert(ast_parser->current->parent);
		ast_parser->current = ast_parser->current->parent;
	}

	lang_ast_node* arg = _lang_alloc_node(&ast_parser->allocator, lang_ast_type_expression);
	lang_ast_append(ast_parser->current, &ast_parser->current->as_call.arguments, arg);
	ast_parser->current = arg;
}
static void _lang_ast_end_call(lang_parser* parser, lang_token const* where) {
	lang_ast_parser* ast_parser = (lang_ast_parser*)parser;
	while(ast_parser->current->type != lang_ast_type_call) {
		assert(ast_parser->current->parent);
		ast_parser->current = ast_parser->current->parent;
	}
}

static void _lang_ast_value(lang_parser* parser, lang_token const* value) {
	lang_ast_parser* ast_parser = (lang_ast_parser*)parser;

	lang_ast_node* node = _lang_alloc_node(&ast_parser->allocator, lang_ast_type_value);
	node->as_value.value = *value;

	if(_lang_is_scope(ast_parser->current->type)) {
		lang_ast_append(ast_parser->current, &ast_parser->current->as_scope.content, node);
		ast_parser->current = node;
	}
	else if(ast_parser->current->type == lang_ast_type_binop) {
		lang_ast_append(ast_parser->current, &ast_parser->current->as_binop.right, node);
		ast_parser->current = node;
	}
}

static void _lang_ast_begin_function(lang_parser* parser, lang_token const* token) {
	lang_ast_parser* ast_parser = (lang_ast_parser*)parser;

	assert(_lang_is_scope(ast_parser->current->type));

	lang_ast_node* fn = _lang_alloc_node(&ast_parser->allocator, lang_ast_type_function);
	lang_ast_append(ast_parser->current, &ast_parser->current->as_scope.content, fn);
	ast_parser->current = fn;
}
static void _lang_ast_add_func_argument(lang_parser* parser, lang_token const* name) {
	lang_ast_parser* ast_parser = (lang_ast_parser*)parser;

	assert(ast_parser->current->type == lang_ast_type_function);

	lang_ast_node* arg = _lang_alloc_node(&ast_parser->allocator, lang_ast_type_declaration);
	arg->as_declaration.name = *name;
	lang_ast_append(ast_parser->current, &ast_parser->current->as_function.arguments, arg);
}
static void _lang_ast_end_function(lang_parser* parser, lang_token const* token) {
	_lang_pop_scope((lang_ast_parser*)parser, lang_ast_type_function);
}
static void _lang_ast_next_statement(lang_parser* parser) {
	lang_ast_parser* ast_parser = (lang_ast_parser*)parser;
	while(!_lang_is_scope(ast_parser->current->type)) {
		assert(ast_parser->current->parent);
		ast_parser->current = ast_parser->current->parent;
	}
}

static void _lang_ast_declare(lang_parser* parser, lang_token const* name) {
	lang_ast_parser* ast_parser = (lang_ast_parser*)parser;

	assert(_lang_is_scope(ast_parser->current->type));

	lang_ast_node* declaration = _lang_alloc_node(&ast_parser->allocator, lang_ast_type_declaration);
	lang_ast_append(ast_parser->current, &ast_parser->current->as_scope.content, declaration);
	declaration->as_declaration.name = *name;
}
static void _lang_ast_init_declaration(lang_parser* parser) {
	lang_ast_parser* ast_parser = (lang_ast_parser*)parser;

	assert(_lang_is_scope(ast_parser->current->type));

	lang_ast_node* last = lang_ast_last(ast_parser->current->as_scope.content);
	assert(last);
	assert(last->type == lang_ast_type_declaration);
	ast_parser->current = last;
}

static void _lang_ast_begin_class(lang_parser* parser, lang_token const* where) {
	lang_ast_parser* ast_parser = (lang_ast_parser*)parser;

	assert(_lang_is_scope(ast_parser->current->type));

	lang_ast_node* node = _lang_alloc_node(&ast_parser->allocator, lang_ast_type_class);

	lang_ast_append(ast_parser->current, &ast_parser->current->as_scope.content, node);
	ast_parser->current = node;
}
static void _lang_ast_end_class(lang_parser* parser, lang_token const* token) {
	_lang_pop_scope((lang_ast_parser*)parser, lang_ast_type_class);
}

LANG_AST_API
lang_ast_parser lang_create_parser_ast(lang_alloc_callbacks alloc, unsigned lang_ast_parser_flags) {
	lang_ast_parser result;

	result.allocator = alloc;

	result.root    = _lang_alloc_node(&alloc, lang_ast_type_scope);
	result.current = result.root;

	result.parser.pfnBeginSubexpression = _lang_ast_begin_subexpression;
	result.parser.pfnEndSubexpression   = _lang_ast_end_subexpression;
	result.parser.pfnBinop              = _lang_ast_binop;
	result.parser.pfnBeginCall          = _lang_ast_begin_call;
	result.parser.pfnNextCallArgument   = _lang_ast_next_call_argument;
	result.parser.pfnEndCall            = _lang_ast_end_call;
	result.parser.pfnValue              = _lang_ast_value;
	result.parser.pfnBeginFunction      = _lang_ast_begin_function;
	result.parser.pfnFuncArgument       = _lang_ast_add_func_argument;
	result.parser.pfnEndFunction        = _lang_ast_end_function;
	result.parser.pfnNextStatement      = _lang_ast_next_statement;
	result.parser.pfnDeclare            = _lang_ast_declare;
	result.parser.pfnInitDeclaration    = _lang_ast_init_declaration;
	result.parser.pfnBeginClass         = _lang_ast_begin_class;
	result.parser.pfnEndClass           = _lang_ast_end_class;

	return result;
}
