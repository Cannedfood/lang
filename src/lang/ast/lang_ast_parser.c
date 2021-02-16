#include "lang_ast_parser.h"

#include <assert.h>
#include <string.h>
#include <limits.h>

lang_ast_node* _new_node(lang_ast_parser const* parser, lang_ast_type type) {
	assert(type != lang_num_ast_types);

	lang_ast_node* result;
	switch (type) {
	#define LANG_AST_NODE(NAME, MEMBERS)            \
	    case LANG_AST_TAG(NAME):                    \
	        result = parser->allocator.alloc(       \
	            parser->allocator.userdata,         \
	            sizeof(LANG_AST_STRUCT(NAME))       \
	        );                                      \
	        memset(&result->as_##NAME, 0, sizeof(LANG_AST_STRUCT(NAME))); \
	        break;

	#include "lang_ast_nodes.txt"

	#undef LANG_AST_NODE
	default: LANG_UNREACHABLE;
	}
	result->type = type;
	return result;
}

static int _node_precedence(lang_ast_node* node) {
	if(node->type == lang_ast_type_expression) return INT_MAX;
	if(node->type == lang_ast_type_unop) return INT_MAX;
	if(node->type == lang_ast_type_call) return 4;

	assert(node->type == lang_ast_type_binop);
	switch (node->as_binop.op.type) {
	case lang_token_plus: case lang_token_minus: return 1;
	case lang_token_mul:  case lang_token_div:   return 2;
	case lang_token_dot:  case lang_token_open_bracket: return 3;
	case lang_token_assign: return 4;
	default: return INT_MAX;
	}
}

static lang_ast_node* _associated_expression(lang_ast_node* from, int precedence) {
	if(from->parent->type == lang_ast_type_binop) {
		int parentPrecedence = _node_precedence(from->parent);
		if(parentPrecedence > precedence)
			return from; // Replace the argument
		else
			return _associated_expression(from->parent, precedence);
	}
	else {
		return from;
	}
}

static lang_ast_node* _lang_ast_find(lang_ast_node* from, lang_ast_type type) {
	while(from->type != type) {
		assert(from->parent);
		from = from->parent;
	}
	return from;
}

static lang_ast_node* _walk_out_of(lang_ast_node* from, lang_ast_type type) {
	from = _lang_ast_find(from, type);
	assert(from->parent);
	return from->parent;
}

static int _lang_is_block(lang_ast_type type) {
	return type == lang_ast_type_block || type == lang_ast_type_function || type == lang_ast_type_class;
}

static lang_ast_node* _parent_block(lang_ast_node* node) {
	do {
		assert(node->parent);
		node = node->parent;
	} while(!_lang_is_block(node->type));
	return node;
}

static void _simplify_block(lang_ast_parser* p, lang_ast_node* node) {
	if(node == NULL) return;

	assert(node->type == lang_ast_type_block);
	if(!node->as_block.content) { // no content: can be removed
		p->allocator.free(p->allocator.userdata, lang_ast_remove(node));
	}
	else if(!node->as_block.content->next) { // one child: just replace with child
		lang_ast_replace(node, node->as_block.content);
		p->allocator.free(p->allocator.userdata, node);
	}
}
static void _simplify_expression(lang_ast_parser* p, lang_ast_node* node) {
	if(node == NULL) return;

	assert(node->type == lang_ast_type_expression);
	if(!node->as_expression.children) { // no content: can be removed
		p->allocator.free(p->allocator.userdata, lang_ast_remove(node));
	}
	else if(!node->as_expression.children->next) { // one child: just replace with child
		lang_ast_replace(node, node->as_expression.children);
		p->allocator.free(p->allocator.userdata, node);
	}
}

static lang_ast_node* _append_value(lang_ast_node* current, lang_ast_node* node) {
	if(_lang_is_block(current->type))
		lang_ast_append(current, &current->as_block.content, node);
	else if(current->type == lang_ast_type_binop)
		lang_ast_append(current, &current->as_binop.right, node);
	else if(current->type == lang_ast_type_expression)
		lang_ast_append(current, &current->as_expression.children, node);
	else if(current->type == lang_ast_type_declaration)
		lang_ast_append(current, &current->as_declaration.initial_value, node);
	else
		assert(!"Invalid location for value"); // I'm tired and drunk, so take this message or leave it.

	return node;
}

// Parser interface

static void _lang_ast_add_comment(lang_parser* parser, lang_token const* value) {
	lang_ast_parser* p = (lang_ast_parser*)parser;
	if(p->flags & lang_ast_parser_flag_include_comments) {
		lang_ast_node* node = _new_node(p, lang_ast_type_comment);
		node->as_comment.value = *value;
		lang_insert_after(p->current, node);
	}
}

static void _lang_ast_begin_subexpression(lang_parser* parser, lang_token const* token) {
	lang_ast_parser* p = (lang_ast_parser*)parser;

	lang_ast_node* node = _new_node(p, lang_ast_type_expression);
	if(_lang_is_block(p->current->type)) {
		lang_ast_append(p->current, &p->current->as_block.content, node);
		p->current = node;
	}
	else {
		// TODO, skip for now
	}
}
static void _lang_ast_end_subexpression(lang_parser* parser, lang_token const* token) {
	lang_ast_parser* p = (lang_ast_parser*)parser;

	lang_ast_node* expression = _lang_ast_find(p->current, lang_ast_type_expression);
	p->current = _walk_out_of(p->current, lang_ast_type_expression);

	_simplify_expression(p, expression);
}
static void _lang_ast_binop(lang_parser* parser, lang_token const* op) {
	lang_ast_parser* p = (lang_ast_parser*)parser;

	lang_ast_node* binop = _new_node(p, lang_ast_type_binop);
	binop->as_binop.op = *op;

	if(_lang_is_block(p->current->type)) {
		lang_ast_append(p->current, &p->current->as_block.content, binop);
		p->current = binop;
	}
	else if(p->current->type == lang_ast_type_value || p->current->type == lang_ast_type_binop) {
		lang_ast_node* target = _associated_expression(p->current, _node_precedence(binop));

		binop->as_binop.left = lang_ast_replace(target, binop);
		p->current = binop;
	}
}
static void _lang_ast_begin_call(lang_parser* parser, lang_token const* where) {
	lang_ast_parser* p = (lang_ast_parser*)parser;

	lang_ast_node* call = _new_node(p, lang_ast_type_call);
	call->as_call.where = *where;

	if(
		p->current->type == lang_ast_type_value ||
		p->current->type == lang_ast_type_binop ||
		p->current->type == lang_ast_type_expression)
	{
		lang_ast_node* target = lang_ast_replace(
			_associated_expression(p->current, _node_precedence(call)),
			call
		);
		lang_ast_append(call, &call->as_call.target, target);
		p->current = call;
	}
}
static void _lang_ast_next_call_argument(lang_parser* parser, lang_token const* where) {
	lang_ast_parser* p = (lang_ast_parser*)parser;
	p->current = _lang_ast_find(p->current, lang_ast_type_call);

	lang_ast_node* arg = _new_node(p, lang_ast_type_expression);
	lang_ast_append(p->current, &p->current->as_call.arguments, arg);
	p->current = arg;
}
static void _lang_ast_end_call(lang_parser* parser, lang_token const* where) {
	lang_ast_parser* p = (lang_ast_parser*)parser;
	p->current = _lang_ast_find(p->current, lang_ast_type_call);

	// Simplify arguments
	lang_ast_node* arg = p->current->as_call.arguments;
	while(arg) {
		_simplify_expression(p, arg);
		arg = arg->next;
	}
}

static void _lang_ast_value(lang_parser* parser, lang_token const* value) {
	lang_ast_parser* p = (lang_ast_parser*)parser;

	lang_ast_node* node = _new_node(p, lang_ast_type_value);
	node->as_value.value = *value;

	p->current = _append_value(p->current, node);
}


static void _lang_ast_begin_function(lang_parser* parser, lang_token const* token) {
	lang_ast_parser* p = (lang_ast_parser*)parser;

	p->current = _append_value(
		p->current,
		_new_node(p, lang_ast_type_function)
	);
}
static void _lang_ast_add_func_argument(lang_parser* parser, lang_token const* name) {
	lang_ast_parser* p = (lang_ast_parser*)parser;

	assert(p->current->type == lang_ast_type_function);

	lang_ast_node* arg = _new_node(p, lang_ast_type_declaration);
	arg->as_declaration.name = *name;
	lang_ast_append(p->current, &p->current->as_function.arguments, arg);
}
static void _lang_ast_end_function(lang_parser* parser, lang_token const* token) {
	lang_ast_parser* p = (lang_ast_parser*)parser;

	p->current = _parent_block(_lang_ast_find(p->current, lang_ast_type_function));
}
static void _lang_ast_next_statement(lang_parser* parser) {
	lang_ast_parser* p = (lang_ast_parser*)parser;
	while(!_lang_is_block(p->current->type)) {
		assert(p->current->parent);
		p->current = p->current->parent;
	}
}

static void _lang_ast_declare(lang_parser* parser, lang_token const* name) {
	lang_ast_parser* p = (lang_ast_parser*)parser;

	assert(_lang_is_block(p->current->type));

	lang_ast_node* declaration = _new_node(p, lang_ast_type_declaration);
	lang_ast_append(p->current, &p->current->as_block.content, declaration);
	declaration->as_declaration.name = *name;

	p->current = declaration;
}
static void _lang_ast_init_declaration(lang_parser* parser) {
	lang_ast_parser* p = (lang_ast_parser*)parser;

	assert(_lang_is_block(p->current->type));

	lang_ast_node* last = lang_ast_last(p->current->as_block.content);
	assert(last);
	assert(last->type == lang_ast_type_declaration);
	p->current = last;
}

static void _lang_ast_begin_class(lang_parser* parser, lang_token const* where) {
	lang_ast_parser* p = (lang_ast_parser*)parser;

	p->current = _append_value(
		p->current,
		_new_node(p, lang_ast_type_class)
	);
}
static void _lang_ast_end_class(lang_parser* parser, lang_token const* token) {
	lang_ast_parser* p = (lang_ast_parser*)parser;

	p->current = _parent_block(_lang_ast_find(p->current, lang_ast_type_class));
}


static void _lang_ast_if(lang_parser* parser, lang_token const* where) {
	lang_ast_parser* p = (lang_ast_parser*)parser;
	assert(_lang_is_block(p->current->type));
	lang_ast_node* if_stmt = _new_node(p, lang_ast_type_if);
	lang_ast_append(p->current, &p->current->as_block.content, if_stmt);

	lang_ast_node* condition = _new_node(p, lang_ast_type_expression);
	lang_ast_append(if_stmt, &if_stmt->as_if.condition, condition);

	p->current = condition;
}
static void _lang_ast_if_body(lang_parser* parser) {
	lang_ast_parser* p = (lang_ast_parser*)parser;

	lang_ast_node* if_true = _new_node(p, lang_ast_type_block);

	p->current = _lang_ast_find(p->current, lang_ast_type_if);
	lang_ast_append(p->current, &p->current->as_if.if_true, if_true);
	p->current = if_true;
}
static void _lang_ast_else(lang_parser* parser, lang_token const* where) {
	lang_ast_parser* p = (lang_ast_parser*)parser;

	lang_ast_node* if_false = _new_node(p, lang_ast_type_block);

	p->current = _lang_ast_find(p->current, lang_ast_type_if);
	lang_ast_append(p->current, &p->current->as_if.if_false, if_false);
	p->current = if_false;
}
static void _lang_ast_end_if(lang_parser* parser) {
	lang_ast_parser* p = (lang_ast_parser*)parser;

	lang_ast_node* if_stmt = _lang_ast_find(p->current, lang_ast_type_if);
	_simplify_expression(p, if_stmt->as_if.condition);
	_simplify_block(p, if_stmt->as_if.if_true);
	_simplify_block(p, if_stmt->as_if.if_false);

	p->current = _parent_block(if_stmt);
}

LANG_AST_API
lang_ast_parser lang_create_parser_ast(lang_alloc_callbacks alloc, unsigned lang_ast_parser_flags) {
	lang_ast_parser result;

	result.allocator = alloc;

	result.flags = lang_ast_parser_flags;

	result.parser = lang_parser_defaults(
		(lang_ast_parser_flags & lang_ast_parser_flag_print_errors)?
			lang_parser_defaults_print_errors : 0
	);

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
	result.parser.pfnIf                 = _lang_ast_if;
	result.parser.pfnIfBody             = _lang_ast_if_body;
	result.parser.pfnElse               = _lang_ast_else;
	result.parser.pfnEndIf              = _lang_ast_end_if;


	result.root    = _new_node(&result, lang_ast_type_block);
	result.current = result.root;

	return result;
}
