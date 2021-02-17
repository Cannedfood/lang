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
	return
		type == lang_ast_type_block ||
		type == lang_ast_type_function ||
		type == lang_ast_type_class ||
		type == lang_ast_type_while ||
		type == lang_ast_type_for ||
		type == lang_ast_type_foreach;
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
	if(!node->as_block.body) { // no content: can be removed
		p->allocator.free(p->allocator.userdata, lang_ast_remove(node));
	}
	else if(!node->as_block.body->next) { // one child: just replace with child
		lang_ast_replace(node, node->as_block.body);
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
		lang_ast_append(current, &current->as_block.body, node);
	else if(current->type == lang_ast_type_binop)
		lang_ast_append(current, &current->as_binop.right, node);
	else if(current->type == lang_ast_type_expression)
		lang_ast_append(current, &current->as_expression.children, node);
	else if(current->type == lang_ast_type_declaration)
		lang_ast_append(current, &current->as_declaration.initial_value, node);
	else
		assert(!"Invalid location for value");

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
	if(_lang_is_block(p->current->type))
		p->current = lang_ast_append(p->current, &p->current->as_block.body, node);
	else
		p->current = _append_value(p->current, node);
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
		p->current = lang_ast_append(p->current, &p->current->as_block.body, binop);
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
		lang_ast_node* target = _associated_expression(p->current, _node_precedence(call));
		lang_ast_replace(target, call);
		lang_ast_append(call, &call->as_call.target, target);
		p->current = call;
	}
}
static void _lang_ast_next_call_argument(lang_parser* parser, lang_token const* where) {
	lang_ast_parser* p = (lang_ast_parser*)parser;
	lang_ast_node* call = _lang_ast_find(p->current, lang_ast_type_call);
	p->current = lang_ast_append(call, &call->as_call.arguments, _new_node(p, lang_ast_type_expression));
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
	p->current = _append_value(p->current, _new_node(p, lang_ast_type_value));
	p->current->as_value.value = *value;
}


static void _lang_ast_begin_function(lang_parser* parser, lang_token const* token) {
	lang_ast_parser* p = (lang_ast_parser*)parser;
	p->current = _append_value(p->current, _new_node(p, lang_ast_type_function));
}
static void _lang_ast_add_func_argument(lang_parser* parser, lang_token const* name) {
	lang_ast_parser* p = (lang_ast_parser*)parser;
	assert(p->current->type == lang_ast_type_function);
	lang_ast_node* arg = lang_ast_append(p->current, &p->current->as_function.arguments, _new_node(p, lang_ast_type_declaration));
	arg->as_declaration.name = *name;
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
	p->current = lang_ast_append(p->current, &p->current->as_block.body, _new_node(p, lang_ast_type_declaration));
	p->current->as_declaration.name = *name;
}
static void _lang_ast_init_declaration(lang_parser* parser) {
	lang_ast_parser* p = (lang_ast_parser*)parser;
	assert(_lang_is_block(p->current->type));
	lang_ast_node* last = lang_ast_last_sibling(p->current->as_block.body);
	assert(last);
	assert(last->type == lang_ast_type_declaration);
	p->current = last;
}

static void _lang_ast_begin_class(lang_parser* parser, lang_token const* where) {
	lang_ast_parser* p = (lang_ast_parser*)parser;
	p->current = _append_value(p->current, _new_node(p, lang_ast_type_class));
}
static void _lang_ast_end_class(lang_parser* parser, lang_token const* token) {
	lang_ast_parser* p = (lang_ast_parser*)parser;
	p->current = _parent_block(_lang_ast_find(p->current, lang_ast_type_class));
}


static void _lang_ast_if(lang_parser* parser, lang_token const* where) {
	lang_ast_parser* p = (lang_ast_parser*)parser;
	assert(_lang_is_block(p->current->type));

	lang_ast_node* if_stmt = lang_ast_append(p->current, &p->current->as_block.body, _new_node(p, lang_ast_type_if));
	p->current = lang_ast_append(if_stmt, &if_stmt->as_if.condition, _new_node(p, lang_ast_type_expression));
}
static void _lang_ast_if_body(lang_parser* parser) {
	lang_ast_parser* p = (lang_ast_parser*)parser;

	lang_ast_node* if_stmt = _lang_ast_find(p->current, lang_ast_type_if);
	p->current = lang_ast_append(if_stmt, &if_stmt->as_if.if_true, _new_node(p, lang_ast_type_block));
}
static void _lang_ast_else(lang_parser* parser, lang_token const* where) {
	lang_ast_parser* p = (lang_ast_parser*)parser;
	lang_ast_node* if_stmt = _lang_ast_find(p->current, lang_ast_type_if);
	p->current = lang_ast_append(if_stmt, &if_stmt->as_if.if_false, _new_node(p, lang_ast_type_block));
}
static void _lang_ast_end_if(lang_parser* parser) {
	lang_ast_parser* p = (lang_ast_parser*)parser;

	lang_ast_node* if_stmt = _lang_ast_find(p->current, lang_ast_type_if);
	_simplify_expression(p, if_stmt->as_if.condition);
	_simplify_block(p, if_stmt->as_if.if_true);
	_simplify_block(p, if_stmt->as_if.if_false);

	p->current = _parent_block(if_stmt);
}

static int _lang_ast_is_loop(lang_ast_type type) { return type == lang_ast_type_while || type == lang_ast_type_for || type == lang_ast_type_foreach; }
static lang_ast_node* _lang_walk_up_to_loop(lang_ast_node* node) {
	while(!_lang_ast_is_loop(node->type)) {
		assert(node->parent);
		node = node->parent;
	}
	return node;
}

static void _lang_ast_while(lang_parser* parser, lang_token const* where) {
	lang_ast_parser* p = (lang_ast_parser*)parser;

	assert(_lang_is_block(p->current->type));
	lang_ast_node* loop = lang_ast_append(p->current, &p->current->as_block.body,
		_new_node(p, lang_ast_type_while)
	);
	p->current = lang_ast_append(loop, &loop->as_while.condition,
		_new_node(p, lang_ast_type_expression)
	);
}
static void _lang_ast_for(lang_parser* parser, lang_token const* where) {
	lang_ast_parser* p = (lang_ast_parser*)parser;
	lang_ast_node* loop = lang_ast_append(p->current, &p->current->as_block.body,
		_new_node(p, lang_ast_type_for)
	);
	p->current = lang_ast_append(loop, &loop->as_for.arguments,
		_new_node(p, lang_ast_type_block)
	);
}
static void _lang_ast_foreach(lang_parser* parser, lang_token const* where) {
	lang_ast_parser* p = (lang_ast_parser*)parser;
	lang_ast_node* loop = lang_ast_append(p->current, &p->current->as_block.body,
		_new_node(p, lang_ast_type_foreach)
	);
	p->current = lang_ast_append(loop, &loop->as_foreach.collection,
		_new_node(p, lang_ast_type_block)
	);
}
static void _lang_ast_foreach_variable(lang_parser* parser, lang_token const* name) {
	lang_ast_parser* p = (lang_ast_parser*)parser;
	lang_ast_node* loop = _lang_walk_up_to_loop(p->current);
	assert(loop->type == lang_ast_type_foreach);
	lang_ast_node* decomp = lang_ast_append(loop, &loop->as_foreach.decomposition, _new_node(p, lang_ast_type_declaration));
	decomp->as_declaration.name = *name;
}

static void _lang_ast_loop_body(lang_parser* parser, lang_token const* where) {
	lang_ast_parser* p = (lang_ast_parser*)parser;
	p->current = _lang_walk_up_to_loop(p->current);
}
static void _lang_ast_end_loop(lang_parser* parser, lang_token const* where) {
	lang_ast_parser* p = (lang_ast_parser*)parser;
	p->current = _parent_block(_lang_walk_up_to_loop(p->current));
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
	result.parser.pfnWhile              = _lang_ast_while;
	result.parser.pfnFor                = _lang_ast_for;
	result.parser.pfnForeach            = _lang_ast_foreach;
	result.parser.pfnForeachVariable    = _lang_ast_foreach_variable;
	result.parser.pfnLoopBody           = _lang_ast_loop_body;
	result.parser.pfnEndLoop            = _lang_ast_end_loop;


	result.root    = _new_node(&result, lang_ast_type_block);
	result.current = result.root;

	return result;
}
