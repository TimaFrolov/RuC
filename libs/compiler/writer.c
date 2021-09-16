/*
 *	Copyright 2021 Andrey Terekhov, Ilya Andreev
 *
 *	Licensed under the Apache License, Version 2.0 (the "License");
 *	you may not use this file except in compliance with the License.
 *	You may obtain a copy of the License at
 *
 *		http://www.apache.org/licenses/LICENSE-2.0
 *
 *	Unless required by applicable law or agreed to in writing, software
 *	distributed under the License is distributed on an "AS IS" BASIS,
 *	WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *	See the License for the specific language governing permissions and
 *	limitations under the License.
 */

#include "writer.h"
#include "uniprinter.h"


#define INDENT			"  "


/** AST writer */
typedef struct writer
{
	const syntax *sx;					/**< Syntax structure */
	universal_io *io;					/**< Output file */
} writer;

static void write_expression(writer *const wrt, const node *const nd, const size_t indent);
static void write_statement(writer *const wrt, const node *const nd, const size_t indent);


//===----------------------------------------------------------------------===//
//                                Writer Utils                                //
//===----------------------------------------------------------------------===//

static void write(writer *const wrt, const char *const string)
{
	uni_printf(wrt->io, string);
}

/**
 *	Write indentation
 *
 *	@param	wrt			Writer
 *	@param	indent		Indentation
 */
static void write_indent(writer *const wrt, const size_t indent)
{
	for (size_t i = 0; i < indent; i++)
	{
		write(wrt, INDENT);
	}
}

/**
 *	Write source location
 *
 *	@param	wrt			Writer
 *	@param	loc			Source location
 */
static void write_location(writer *const wrt, const location loc)
{
	uni_printf(wrt->io, " at <%lu, %lu>\n", loc.begin, loc.end);
}

/**
 *	Write type spelling
 *
 *	@param	wrt			Writer
 *	@param	type		Type
 */
static void write_type(writer *const wrt, const item_t type)
{
	if (type_is_null_pointer(type))
	{
		write(wrt, "nullptr");
	}
	else if (type_is_integer(type))
	{
		write(wrt, "int");
	}
	else if (type_is_floating(type))
	{
		write(wrt, "float");
	}
	else if (type_is_file(type))
	{
		write(wrt, "FILE");
	}
	else if (type_is_void(type))
	{
		write(wrt, "void");
	}
	else if (type_is_array(wrt->sx, type))
	{
		write_type(wrt, type_array_get_element_type(wrt->sx, type));
		write(wrt, "[]");
	}
	else if (type_is_pointer(wrt->sx, type))
	{
		write_type(wrt, type_pointer_get_element_type(wrt->sx, type));
		write(wrt, "*");
	}
	else if (type_is_structure(wrt->sx, type))
	{
		write(wrt, "struct { ");

		const size_t member_amount = type_structure_get_member_amount(wrt->sx, type);
		for (size_t i = 0; i < member_amount; i++)
		{
			const item_t member_type = type_structure_get_member_type(wrt->sx, type, i);
			const size_t member_repr = type_structure_get_member_name(wrt->sx, type, i);

			write_type(wrt, member_type);
			uni_printf(wrt->io, " %s; ", repr_get_name(wrt->sx, member_repr));
		}

		write(wrt, "}");
	}
	else if (type_is_function(wrt->sx, type))
	{
		write_type(wrt, type_function_get_return_type(wrt->sx, type));
		write(wrt, " (");

		const size_t parameter_amount = type_function_get_parameter_amount(wrt->sx, type);
		for (size_t i = 0; i < parameter_amount; i++)
		{
			const item_t parameter_type = type_function_get_parameter_type(wrt->sx, type, i);

			write_type(wrt, parameter_type);
			if (i != parameter_amount - 1)
			{
				write(wrt, ", ");
			}
		}
		write_type(wrt, type_pointer_get_element_type(wrt->sx, type));
		write(wrt, ")");
	}
}


//===----------------------------------------------------------------------===//
//                            Expression Writing                              //
//===----------------------------------------------------------------------===//

/**
 *	Write identifier expression
 *
 *	@param	wrt			Writer
 *	@param	nd			Node in AST
 *	@param	indent		Indentation
 */
static void write_identifier_expression(writer *const wrt, const node *const nd, const size_t indent)
{
	write_indent(wrt, indent);
	write(wrt, "EXPR_IDENTIFIER");

	write_location(wrt, expression_get_location(nd));
}

/**
 *	Write literal expression
 *
 *	@param	wrt			Writer
 *	@param	nd			Node in AST
 *	@param	indent		Indentation
 */
static void write_literal_expression(writer *const wrt, const node *const nd, const size_t indent)
{
	write_indent(wrt, indent);
	write(wrt, "EXPR_LITERAL");
	write_location(wrt, expression_get_location(nd));
}

/**
 *	Write subscript expression
 *
 *	@param	wrt			Writer
 *	@param	nd			Node in AST
 *	@param	indent		Indentation
 */
static void write_subscript_expression(writer *const wrt, const node *const nd, const size_t indent)
{
	write_indent(wrt, indent);
	write(wrt, "EXPR_SUBSCRIPT");
	write_location(wrt, expression_get_location(nd));

	const node base = expression_subscript_get_base(nd);
	write_expression(wrt, &base, indent + 1);

	const node index = expression_subscript_get_index(nd);
	write_expression(wrt, &index, indent + 1);
}

/**
 *	Write call expression
 *
 *	@param	wrt			Writer
 *	@param	nd			Node in AST
 *	@param	indent		Indentation
 */
static void write_call_expression(writer *const wrt, const node *const nd, const size_t indent)
{
	write_indent(wrt, indent);
	write(wrt, "EXPR_CALL");
	write_location(wrt, expression_get_location(nd));

	const node callee = expression_call_get_callee(nd);
	write_expression(wrt, &callee, indent + 1);

	const size_t arguments_amount = expression_call_get_arguments_amount(nd);
	for (size_t i = 0; i < arguments_amount; i++)
	{
		const node argument = expression_call_get_argument(nd, i);
		write_expression(wrt, &argument, indent + 1);
	}
}

/**
 *	Write member expression
 *
 *	@param	wrt			Writer
 *	@param	nd			Node in AST
 *	@param	indent		Indentation
 */
static void write_member_expression(writer *const wrt, const node *const nd, const size_t indent)
{
	write_indent(wrt, indent);
	write(wrt, "EXPR_MEMBER");
	write_location(wrt, expression_get_location(nd));

	const node base = expression_member_get_base(nd);
	write_expression(wrt, &base, indent + 1);

	const size_t index = expression_member_get_member_index(nd);
	(void)index;
}

/**
 *	Write unary expression
 *
 *	@param	wrt			Writer
 *	@param	nd			Node in AST
 *	@param	indent		Indentation
 */
static void write_unary_expression(writer *const wrt, const node *const nd, const size_t indent)
{
	write_indent(wrt, indent);
	write(wrt, "EXPR_UNARY");
	write_location(wrt, expression_get_location(nd));

	const node operand = expression_unary_get_operand(nd);
	write_expression(wrt, &operand, indent + 1);
}

/**
 *	Write binary expression
 *
 *	@param	wrt			Writer
 *	@param	nd			Node in AST
 *	@param	indent		Indentation
 */
static void write_binary_expression(writer *const wrt, const node *const nd, const size_t indent)
{
	write_indent(wrt, indent);
	write(wrt, "EXPR_BINARY");
	write_location(wrt, expression_get_location(nd));

	const node LHS = expression_binary_get_LHS(nd);
	write_expression(wrt, &LHS, indent + 1);

	const node RHS = expression_binary_get_RHS(nd);
	write_expression(wrt, &RHS, indent + 1);
}

/**
 *	Write ternary expression
 *
 *	@param	wrt			Writer
 *	@param	nd			Node in AST
 *	@param	indent		Indentation
 */
static void write_ternary_expression(writer *const wrt, const node *const nd, const size_t indent)
{
	write_indent(wrt, indent);
	write(wrt, "EXPR_TERNARY");
	write_location(wrt, expression_get_location(nd));

	const node condition = expression_ternary_get_condition(nd);
	write_expression(wrt, &condition, indent + 1);

	const node LHS = expression_binary_get_LHS(nd);
	write_expression(wrt, &LHS, indent + 1);

	const node RHS = expression_binary_get_RHS(nd);
	write_expression(wrt, &RHS, indent + 1);
}

/**
 *	Write expression list
 *
 *	@param	wrt			Writer
 *	@param	nd			Node in AST
 *	@param	indent		Indentation
 */
static void write_expression_list(writer *const wrt, const node *const nd, const size_t indent)
{
	write_indent(wrt, indent);
	write(wrt, "EXPR_LIST");
	write_location(wrt, expression_get_location(nd));

	const size_t size = expression_list_get_size(nd);
	for (size_t i = 0; i < size; i++)
	{
		const node subexpr = expression_list_get_subexpr(nd, i);
		write_expression(wrt, &subexpr, indent + 1);
	}
}

/**
 *	Write expression
 *
 *	@param	wrt			Writer
 *	@param	nd			Node in AST
 *	@param	indent		Indentation
 */
static void write_expression(writer *const wrt, const node *const nd, const size_t indent)
{
	switch (expression_get_class(nd))
	{
		case EXPR_IDENTIFIER:
			write_identifier_expression(wrt, nd, indent);
			return;

		case EXPR_LITERAL:
			write_literal_expression(wrt, nd, indent);
			return;

		case EXPR_SUBSCRIPT:
			write_subscript_expression(wrt, nd, indent);
			return;

		case EXPR_CALL:
			write_call_expression(wrt, nd, indent);
			return;

		case EXPR_MEMBER:
			write_member_expression(wrt, nd, indent);
			return;

		case EXPR_UNARY:
			write_unary_expression(wrt, nd, indent);
			return;

		case EXPR_BINARY:
			write_binary_expression(wrt, nd, indent);
			return;

		case EXPR_TERNARY:
			write_ternary_expression(wrt, nd, indent);
			return;

		case EXPR_LIST:
			write_expression_list(wrt, nd, indent);
			return;
	}
}


//===----------------------------------------------------------------------===//
//                            Declaration Writing                             //
//===----------------------------------------------------------------------===//

/**
 *	Write variable declaration
 *
 *	@param	wrt			Writer
 *	@param	nd			Node in AST
 *	@param	indent		Indentation
 */
static void write_variable_declaration(writer *const wrt, const node *const nd, const size_t indent)
{
	write_indent(wrt, indent);
	write(wrt, "DECL_VAR\n");

	if (declaration_variable_has_initializer(nd))
	{
		const node initializer = declaration_variable_get_initializer(nd);
		write_expression(wrt, &initializer, indent + 1);
	}
}

/**
 *	Write type declaration
 *
 *	@param	wrt			Writer
 *	@param	nd			Node in AST
 *	@param	indent		Indentation
 */
static void write_type_declaration(writer *const wrt, const node *const nd, const size_t indent)
{
	write_indent(wrt, indent);
	write(wrt, "DECL_TYPE\n");
	(void)nd;
}

/**
 *	Write function declaration
 *
 *	@param	wrt			Writer
 *	@param	nd			Node in AST
 *	@param	indent		Indentation
 */
static void write_function_declaration(writer *const wrt, const node *const nd, const size_t indent)
{
	write_indent(wrt, indent);
	write(wrt, "DECL_FUNC\n");

	const node body = declaration_function_get_body(nd);
	write_statement(wrt, &body, indent + 1);
}

/**
 *	Write declaration
 *
 *	@param	wrt			Writer
 *	@param	nd			Node in AST
 *	@param	indent		Indentation
 */
static void write_declaration(writer *const wrt, const node *const nd, const size_t indent)
{
	switch (declaration_get_class(nd))
	{
		case DECL_VAR:
			write_variable_declaration(wrt, nd, indent);
			return;

		case DECL_TYPE:
			write_type_declaration(wrt, nd, indent);
			return;

		case DECL_FUNC:
			write_function_declaration(wrt, nd, indent);
			return;
	}
}


//===----------------------------------------------------------------------===//
//                             Statement Writing                              //
//===----------------------------------------------------------------------===//

/**
 *	Write labeled statement
 *
 *	@param	wrt			Writer
 *	@param	nd			Node in AST
 *	@param	indent		Indentation
 */
static void write_labeled_statement(writer *const wrt, const node *const nd, const size_t indent)
{
	write_indent(wrt, indent);
	write(wrt, "STMT_LABEL\n");

	const size_t label = statement_labeled_get_label(nd);
	(void)label;

	const node substmt = statement_labeled_get_substmt(nd);
	write_statement(wrt, &substmt, indent + 1);
}

/**
 *	Write case statement
 *
 *	@param	wrt			Writer
 *	@param	nd			Node in AST
 *	@param	indent		Indentation
 */
static void write_case_statement(writer *const wrt, const node *const nd, const size_t indent)
{
	write_indent(wrt, indent);
	write(wrt, "STMT_CASE\n");
	(void)nd;

	// TODO: case writing
}

/**
 *	Write default statement
 *
 *	@param	wrt			Writer
 *	@param	nd			Node in AST
 *	@param	indent		Indentation
 */
static void write_default_statement(writer *const wrt, const node *const nd, const size_t indent)
{
	write_indent(wrt, indent);
	write(wrt, "STMT_DEFAULT\n");
	(void)nd;

	// TODO: default writing
}

/**
 *	Write compound statement
 *
 *	@param	wrt			Writer
 *	@param	nd			Node in AST
 *	@param	indent		Indentation
 */
static void write_compound_statement(writer *const wrt, const node *const nd, const size_t indent)
{
	write_indent(wrt, indent);
	write(wrt, "STMT_COMPOUND\n");

	const size_t size = statement_compound_get_size(nd);
	for (size_t i = 0; i < size; i++)
	{
		const node substmt = statement_compound_get_substmt(nd, i);
		write_statement(wrt, &substmt, indent + 1);
	}
}

/**
 *	Write null statement
 *
 *	@param	wrt			Writer
 *	@param	nd			Node in AST
 *	@param	indent		Indentation
 */
static void write_null_statement(writer *const wrt, const node *const nd, const size_t indent)
{
	write_indent(wrt, indent);
	write(wrt, "STMT_NULL\n");
	(void)nd;
}

/**
 *	Write if statement
 *
 *	@param	wrt			Writer
 *	@param	nd			Node in AST
 *	@param	indent		Indentation
 */
static void write_if_statement(writer *const wrt, const node *const nd, const size_t indent)
{
	write_indent(wrt, indent);
	write(wrt, "STMT_IF\n");

	const node condition = statement_if_get_condition(nd);
	write_expression(wrt, &condition, indent + 1);

	const node then_substmt = statement_if_get_then_substmt(nd);
	write_statement(wrt, &then_substmt, indent + 1);

	if (statement_if_has_else_substmt(nd))
	{
		const node else_substmt = statement_if_get_else_substmt(nd);
		write_statement(wrt, &else_substmt, indent + 1);
	}
}

/**
 *	Write switch statement
 *
 *	@param	wrt			Writer
 *	@param	nd			Node in AST
 *	@param	indent		Indentation
 */
static void write_switch_statement(writer *const wrt, const node *const nd, const size_t indent)
{
	write_indent(wrt, indent);
	write(wrt, "STMT_SWITCH\n");
	(void)nd;

	// TODO: switch writing
}

/**
 *	Write while statement
 *
 *	@param	wrt			Writer
 *	@param	nd			Node in AST
 *	@param	indent		Indentation
 */
static void write_while_statement(writer *const wrt, const node *const nd, const size_t indent)
{
	write_indent(wrt, indent);
	write(wrt, "STMT_WHILE\n");

	const node condition = statement_while_get_condition(nd);
	write_expression(wrt, &condition, indent + 1);

	const node body = statement_while_get_body(nd);
	write_statement(wrt, &body, indent + 1);
}

/**
 *	Write do statement
 *
 *	@param	wrt			Writer
 *	@param	nd			Node in AST
 *	@param	indent		Indentation
 */
static void write_do_statement(writer *const wrt, const node *const nd, const size_t indent)
{
	write_indent(wrt, indent);
	write(wrt, "STMT_DO\n");

	const node body = statement_while_get_body(nd);
	write_statement(wrt, &body, indent + 1);

	const node condition = statement_while_get_condition(nd);
	write_expression(wrt, &condition, indent + 1);
}

/**
 *	Write for statement
 *
 *	@param	wrt			Writer
 *	@param	nd			Node in AST
 *	@param	indent		Indentation
 */
static void write_for_statement(writer *const wrt, const node *const nd, const size_t indent)
{
	write_indent(wrt, indent);
	write(wrt, "STMT_FOR\n");

	if (statement_for_has_inition(nd))
	{
		const node inition = statement_for_get_inition(nd);
		write_statement(wrt, &inition, indent + 1);
	}

	if (statement_for_has_condition(nd))
	{
		const node condition = statement_for_get_condition(nd);
		write_expression(wrt, &condition, indent + 1);
	}

	if (statement_for_has_increment(nd))
	{
		const node increment = statement_for_get_increment(nd);
		write_statement(wrt, &increment, indent + 1);
	}

	const node body = statement_for_get_body(nd);
	write_statement(wrt, &body, indent + 1);
}

/**
 *	Write goto statement
 *
 *	@param	wrt			Writer
 *	@param	nd			Node in AST
 *	@param	indent		Indentation
 */
static void write_goto_statement(writer *const wrt, const node *const nd, const size_t indent)
{
	write_indent(wrt, indent);
	write(wrt, "STMT_GOTO\n");

	const size_t label = statement_goto_get_label(nd);
	(void)label;
}

/**
 *	Write continue statement
 *
 *	@param	wrt			Writer
 *	@param	nd			Node in AST
 *	@param	indent		Indentation
 */
static void write_continue_statement(writer *const wrt, const node *const nd, const size_t indent)
{
	write_indent(wrt, indent);
	write(wrt, "STMT_CONTINUE\n");

	(void)nd;
}

/**
 *	Write break statement
 *
 *	@param	wrt			Writer
 *	@param	nd			Node in AST
 *	@param	indent		Indentation
 */
static void write_break_statement(writer *const wrt, const node *const nd, const size_t indent)
{
	write_indent(wrt, indent);
	write(wrt, "STMT_BREAK\n");

	(void)nd;
}

/**
 *	Write return statement
 *
 *	@param	wrt			Writer
 *	@param	nd			Node in AST
 *	@param	indent		Indentation
 */
static void write_return_statement(writer *const wrt, const node *const nd, const size_t indent)
{
	write_indent(wrt, indent);
	write(wrt, "STMT_RETURN\n");

	if (statement_return_has_expression(nd))
	{
		const node expression = statement_return_get_expression(nd);
		write_expression(wrt, &expression, indent + 1);
	}
}

/**
 *	Write statement
 *
 *	@param	wrt			Writer
 *	@param	nd			Node in AST
 *	@param	indent		Indentation
 */
static void write_statement(writer *const wrt, const node *const nd, const size_t indent)
{
	switch (statement_get_class(nd))
	{
		case STMT_DECL:
			write_declaration(wrt, nd, indent);
			return;

		case STMT_LABEL:
			write_labeled_statement(wrt, nd, indent);
			return;

		case STMT_CASE:
			write_case_statement(wrt, nd, indent);
			return;

		case STMT_DEFAULT:
			write_default_statement(wrt, nd, indent);
			return;

		case STMT_COMPOUND:
			write_compound_statement(wrt, nd, indent);
			return;

		case STMT_EXPR:
			write_expression(wrt, nd, indent);
			return;

		case STMT_NULL:
			write_null_statement(wrt, nd, indent);
			return;

		case STMT_IF:
			write_if_statement(wrt, nd, indent);
			return;

		case STMT_SWITCH:
			write_switch_statement(wrt, nd, indent);
			return;

		case STMT_WHILE:
			write_while_statement(wrt, nd, indent);
			return;

		case STMT_DO:
			write_do_statement(wrt, nd, indent);
			return;

		case STMT_FOR:
			write_for_statement(wrt, nd, indent);
			return;

		case STMT_GOTO:
			write_goto_statement(wrt, nd, indent);
			return;

		case STMT_CONTINUE:
			write_continue_statement(wrt, nd, indent);
			return;

		case STMT_BREAK:
			write_break_statement(wrt, nd, indent);
			return;

		case STMT_RETURN:
			write_return_statement(wrt, nd, indent);
			return;
	}
}

/**
 *	Write translation unit
 *
 *	@param	wrt			Writer
 *	@param	nd			Node in AST
 */
static void write_translation_unit(writer *const wrt, const node *const nd)
{
	write(wrt, "Translation unit\n");

	const size_t size = translation_unit_get_size(nd);
	for (size_t i = 0; i < size; i++)
	{
		const node declaration = translation_unit_get_declaration(nd, i);
		write_declaration(wrt, &declaration, 1);
	}
}


/*
 *	 __     __   __     ______   ______     ______     ______   ______     ______     ______
 *	/\ \   /\ "-.\ \   /\__  _\ /\  ___\   /\  == \   /\  ___\ /\  __ \   /\  ___\   /\  ___\
 *	\ \ \  \ \ \-.  \  \/_/\ \/ \ \  __\   \ \  __<   \ \  __\ \ \  __ \  \ \ \____  \ \  __\
 *	 \ \_\  \ \_\\"\_\    \ \_\  \ \_____\  \ \_\ \_\  \ \_\    \ \_\ \_\  \ \_____\  \ \_____\
 *	  \/_/   \/_/ \/_/     \/_/   \/_____/   \/_/ /_/   \/_/     \/_/\/_/   \/_____/   \/_____/
 */


void write_AST(const char *const path, syntax *const sx)
{
	universal_io io = io_create();
	if (path == NULL || sx == NULL || out_set_file(&io, path))
	{
		return;
	}

	writer wrt = { .sx = sx, .io = &io };

	const node root = node_get_root(&sx->tree);
	write_translation_unit(&wrt, &root);
	//wrt_clear(&wrt);

	io_erase(&io);
}
