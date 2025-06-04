/*************************************************************************
 * Copyright (c) 2011 AT&T Intellectual Property 
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * https://www.eclipse.org/legal/epl-v10.html
 *
 * Contributors: Details at https://graphviz.org
 *************************************************************************/

%require "3.0"

  /* By default, Bison emits a parser using symbols prefixed with "yy". Graphviz
   * contains multiple Bison-generated parsers, so we alter this prefix to avoid
   * symbol clashes.
   */
%define api.prefix {ex_}

%{

/*
 * Glenn Fowler
 * AT&T Research
 *
 * expression library grammar and compiler
 */

#include <assert.h>
#include <expr/exop.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ast/ast.h>
#include <util/gv_ctype.h>
#include <util/streq.h>

%}

%union
{
	struct Exnode_s*expr;
	double		floating;
	struct Exref_s*	reference;
	struct Exid_s*	id;
	long long integer;
	int		op;
	char*		string;
}

%start	program

%token	MINTOKEN

%token	INTEGER
%token	UNSIGNED
%token	CHARACTER
%token	FLOATING
%token	STRING
%token	VOIDTYPE

%token	ADDRESS
%token	ARRAY
%token	BREAK
%token	CALL
%token	CASE
%token	CONSTANT
%token	CONTINUE
%token	DECLARE
%token	DEFAULT
%token	DYNAMIC
%token	ELSE
%token	EXIT
%token	FOR
%token	FUNCTION
%token	GSUB
%token	ITERATE
%token	ITERATOR
%token	ID
%token	IF
%token	LABEL
%token	MEMBER
%token	NAME
%token	POS
%token	PRAGMA
%token	PRE
%token	PRINT
%token	PRINTF
%token	PROCEDURE
%token	QUERY
%token	RAND
%token	RETURN
%token	SCANF
%token	SPLIT
%token	SPRINTF
%token	SRAND
%token	SSCANF
%token	SUB
%token	SUBSTR
%token	SWITCH
%token	TOKENS
%token	UNSET
%token	WHILE

%token	F2I
%token	F2S
%token	I2F
%token	I2S
%token	S2B
%token	S2F
%token	S2I

%token	F2X
%token	I2X
%token	S2X
%token	X2F
%token	X2I
%token	X2S
%token	X2X
%token	XPRINT

%left	<op>	','
%right	<op>	'='
%right	<op>	'?'	':'
%left	<op>	OR
%left	<op>	AND
%left	<op>	'|'
%left	<op>	'^'
%left	<op>	'&'
%binary	<op>	EQ	NE
%binary	<op>	'<'	'>'	LE	GE
%left	<op>	LSH	RSH
%left	<op>	'+'	'-'	IN_OP
%left	<op>	'*'	'/'	'%'
%right	<op>	'!'	'~'	'#'	UNARY
%right	<op>	INC	DEC
%right	<op>	CAST
%left	<op>	'('

%type <expr>		statement	statement_list	arg_list
%type <expr>		else_opt	expr_opt	expr
%type <expr>		args		variable	assign
%type <expr>		dcl_list	dcl_item	index
%type <expr>		initialize	switch_item	constant
%type <expr>		formals		formal_list	formal_item
%type <reference>	members
%type <id>		ID		LABEL		NAME
%type <id>		CONSTANT	ARRAY		FUNCTION	DECLARE
%type <id>		EXIT		PRINT		PRINTF		QUERY
%type <id>		RAND		SRAND
%type <id>		SPRINTF		PROCEDURE	name		dcl_name
%type <id>		GSUB		SUB		SUBSTR
%type <id>		SPLIT		TOKENS          splitop
%type <id>		IF		WHILE		FOR		ITERATOR
%type <id>		BREAK		CONTINUE	print		member
%type <id>		RETURN		DYNAMIC		SWITCH		UNSET
%type <id>		SCANF		SSCANF		scan
%type <floating>	FLOATING
%type <integer>		INTEGER		UNSIGNED	array
%type <string>		STRING

%token	MAXTOKEN

  /* ask Bison to generate a table, yytname, containing string representations
   * of all the above tokens
   */
%token-table

%{

#include <expr/exgram.h>

void ex_error(const char *message);

%}

%%

program		:	statement_list action_list
		{
			if ($1)	{
				if (expr.program->main.value)
					exfreenode(expr.program, expr.program->main.value);
				if ($1->op == S2B)
				{
					Exnode_t *x = $1;
					$1 = x->data.operand.left;
					x->data.operand.left = 0;
					exfreenode(expr.program, x);
				}
				expr.program->main.lex = PROCEDURE;
				expr.program->main.value = exnewnode(expr.program, PROCEDURE, true, $1->type, NULL, $1);
			}
		}
		;

action_list	:	/* empty */
		|	action_list action
		;

action		:	LABEL ':' {
				if (expr.procedure)
					exerror("no nested function definitions");
				$1->lex = PROCEDURE;
				expr.procedure = $1->value = exnewnode(expr.program, PROCEDURE, true, $1->type, NULL, NULL);
				expr.procedure->type = INTEGER;
				static Dtdisc_t disc = {.key = offsetof(Exid_t, name)};
				if (expr.assigned && !streq($1->name, "begin"))
				{
					if (!(expr.procedure->data.procedure.frame = dtopen(&disc, Dtset)) ||
					    !dtview(expr.procedure->data.procedure.frame, expr.program->symbols))
						exnospace();
					expr.program->symbols = expr.program->frame = expr.procedure->data.procedure.frame;
				}
			} statement_list
		{
			expr.procedure = NULL;
			if (expr.program->frame)
			{
				expr.program->symbols = expr.program->frame->view;
				dtview(expr.program->frame, NULL);
				expr.program->frame = NULL;
			}
			if ($4 && $4->op == S2B)
			{
				Exnode_t *x = $4;
				$4 = x->data.operand.left;
				x->data.operand.left = 0;
				exfreenode(expr.program, x);
			}
			$1->value->data.procedure.body = excast(expr.program, $4, $1->type, NULL, 0);
		}
		;

statement_list	:	/* empty */
		{
			$$ = 0;
		}
		|	statement_list statement
		{
			if (!$1)
				$$ = $2;
			else if (!$2)
				$$ = $1;
			else if ($1->op == CONSTANT)
			{
				exfreenode(expr.program, $1);
				$$ = $2;
			}
			else $$ = exnewnode(expr.program, ';', true, $2->type, $1, $2);
		}
		;

statement	:	'{' statement_list '}'
		{
			$$ = $2;
		}
		|	expr_opt ';'
		{
			$$ = ($1 && $1->type == STRING) ? exnewnode(expr.program, S2B, true, INTEGER, $1, NULL) : $1;
		}
		|	DECLARE {expr.declare = $1->type;} dcl_list ';'
		{
			$$ = $3;
			expr.declare = 0;
		}
		|	IF '(' expr ')' statement else_opt
		{
			if (exisAssign ($3))
				exwarn ("assignment used as boolean in if statement");
			if ($3->type == STRING)
				$3 = exnewnode(expr.program, S2B, true, INTEGER, $3, NULL);
			else if (!INTEGRAL($3->type))
				$3 = excast(expr.program, $3, INTEGER, NULL, 0);
			$$ = exnewnode(expr.program, $1->index, true, INTEGER, $3, exnewnode(expr.program, ':', true, $5 ? $5->type : 0, $5, $6));
		}
		|	FOR '(' variable ')' statement
		{
			$$ = exnewnode(expr.program, ITERATE, false, INTEGER, NULL, NULL);
			$$->data.generate.array = $3;
			if (!$3->data.variable.index || $3->data.variable.index->op != DYNAMIC)
				exerror("simple index variable expected");
			$$->data.generate.index = $3->data.variable.index->data.variable.symbol;
			if ($3->op == ID && $$->data.generate.index->type != INTEGER)
				exerror("integer index variable expected");
			exfreenode(expr.program, $3->data.variable.index);
			$3->data.variable.index = 0;
			$$->data.generate.statement = $5;
		}
		|	FOR '(' expr_opt ';' expr_opt ';' expr_opt ')' statement
		{
			if (!$5)
			{
				$5 = exnewnode(expr.program, CONSTANT, false, INTEGER, NULL, NULL);
				$5->data.constant.value.integer = 1;
			}
			else if ($5->type == STRING)
				$5 = exnewnode(expr.program, S2B, true, INTEGER, $5, NULL);
			else if (!INTEGRAL($5->type))
				$5 = excast(expr.program, $5, INTEGER, NULL, 0);
			$$ = exnewnode(expr.program, $1->index, true, INTEGER, $5, exnewnode(expr.program, ';', 1, 0, $7, $9));
			if ($3)
				$$ = exnewnode(expr.program, ';', true, INTEGER, $3, $$);
		}
		|	ITERATOR '(' variable ')' statement
		{
			$$ = exnewnode(expr.program, ITERATOR, false, INTEGER, NULL, NULL);
			$$->data.generate.array = $3;
			if (!$3->data.variable.index || $3->data.variable.index->op != DYNAMIC)
				exerror("simple index variable expected");
			$$->data.generate.index = $3->data.variable.index->data.variable.symbol;
			if ($3->op == ID && $$->data.generate.index->type != INTEGER)
				exerror("integer index variable expected");
			exfreenode(expr.program, $3->data.variable.index);
			$3->data.variable.index = 0;
			$$->data.generate.statement = $5;
		}
		|	UNSET '(' DYNAMIC ')'
		{
			if ($3->local == NULL)
              			exerror("cannot apply unset to non-array %s", $3->name);
			$$ = exnewnode(expr.program, UNSET, false, INTEGER, NULL, NULL);
			$$->data.variable.symbol = $3;
			$$->data.variable.index = NULL;
		}
		|	UNSET '(' DYNAMIC ',' expr  ')'
		{
			if ($3->local == NULL)
              			exerror("cannot apply unset to non-array %s", $3->name);
			if (($3->index_type > 0) && ($5->type != $3->index_type))
            		    exerror("%s indices must have type %s, not %s", 
				$3->name, extypename(expr.program, $3->index_type),extypename(expr.program, $5->type));
			$$ = exnewnode(expr.program, UNSET, false, INTEGER, NULL, NULL);
			$$->data.variable.symbol = $3;
			$$->data.variable.index = $5;
		}
		|	WHILE '(' expr ')' statement
		{
			if (exisAssign ($3))
				exwarn ("assignment used as boolean in while statement");
			if ($3->type == STRING)
				$3 = exnewnode(expr.program, S2B, true, INTEGER, $3, NULL);
			else if (!INTEGRAL($3->type))
				$3 = excast(expr.program, $3, INTEGER, NULL, 0);
			$$ = exnewnode(expr.program, $1->index, true, INTEGER, $3, exnewnode(expr.program, ';', true, 0, NULL, $5));
		}
		|	SWITCH '(' expr {expr.declare=$3->type;} ')' '{' switch_list '}'
		{
			Switch_t*	sw = expr.swstate;

			$$ = exnewnode(expr.program, $1->index, true, INTEGER, $3, exnewnode(expr.program, DEFAULT, true, 0, sw->defcase, sw->firstcase));
			expr.swstate = expr.swstate->prev;
			free(sw->base);
			free(sw);
			expr.declare = 0;
		}
		|	BREAK expr_opt ';'
		{
		loopop:
			if (!$2)
			{
				$2 = exnewnode(expr.program, CONSTANT, false, INTEGER, NULL, NULL);
				$2->data.constant.value.integer = 1;
			}
			else if (!INTEGRAL($2->type))
				$2 = excast(expr.program, $2, INTEGER, NULL, 0);
			$$ = exnewnode(expr.program, $1->index, true, INTEGER, $2, NULL);
		}
		|	CONTINUE expr_opt ';'
		{
			goto loopop;
		}
		|	RETURN expr_opt ';'
		{
			if ($2)
			{
				if (expr.procedure && !expr.procedure->type)
					exerror("return in void function");
				$2 = excast(expr.program, $2, expr.procedure ? expr.procedure->type : INTEGER, NULL, 0);
			}
			$$ = exnewnode(expr.program, RETURN, true, $2 ? $2->type : 0, $2, NULL);
		}
		;

switch_list	:	/* empty */
		{
			Switch_t*		sw;

			if (!(sw = calloc(1, sizeof(Switch_t)))) {
				exnospace();
			} else {
				expr.swstate = sw;
				sw->type = expr.declare;
				size_t n = 8;
				if (!(sw->base = calloc(n, sizeof(Extype_t*)))) {
					exnospace();
					n = 0;
				}
				sw->cap = n;
			}
		}
		|	switch_list switch_item
		;

switch_item	:	case_list statement_list
		{
			Switch_t*	sw = expr.swstate;

			$$ = exnewnode(expr.program, CASE, true, 0, $2, NULL);
			if (sw->cur > 0)
			{
				if (sw->lastcase)
					sw->lastcase->data.select.next = $$;
				else
					sw->firstcase = $$;
				sw->lastcase = $$;
				const size_t n = sw->cap;
				sw->cur = 0;
				$$->data.select.constant = exalloc(expr.program, (n + 1) * sizeof(Extype_t*));
				memcpy($$->data.select.constant, sw->base, n * sizeof(Extype_t*));
				$$->data.select.constant[n] = 0;
			}
			else
				$$->data.select.constant = 0;
			if (sw->def)
			{
				sw->def = 0;
				if (sw->defcase)
					exerror("duplicate default in switch");
				else
					sw->defcase = $2;
			}
		}
		;

case_list	:	case_item
		|	case_list case_item
		;

case_item	:	CASE constant ':'
		{
			if (expr.swstate->cur >= expr.swstate->cap)
			{
				size_t n = expr.swstate->cap;
				if (!(expr.swstate->base = realloc(expr.swstate->base, sizeof(Extype_t*) * 2 * n)))
				{
					exerror("too many case labels for switch");
					n = 0;
				}
				expr.swstate->cap = 2 * n;
			}
			if (expr.swstate->base != NULL)
			{
				$2 = excast(expr.program, $2, expr.swstate->type, NULL, 0);
				expr.swstate->base[expr.swstate->cur++] = &$2->data.constant.value;
			}
		}
		|	DEFAULT ':'
		{
			expr.swstate->def = 1;
		}
		;

dcl_list	:	dcl_item
		|	dcl_list ',' dcl_item
		{
			if ($3)
				$$ = $1 ? exnewnode(expr.program, ',', true, $3->type, $1, $3) : $3;
		}
		;

dcl_item	:	dcl_name {checkName ($1); expr.id=$1;} array initialize
		{
			$$ = 0;
			if (!$1->type || expr.declare)
				$1->type = expr.declare;
			if ($4 && $4->op == PROCEDURE)
			{
				$1->lex = PROCEDURE;
				$1->type = $4->type;
				$1->value = $4;
			}
			else
			{
				if ($1->type == 0) {
					exerror("%s: a variable cannot be void typed", $1->name);
				}
				$1->lex = DYNAMIC;
				$1->value = exnewnode(expr.program, 0, false, 0, NULL, NULL);
				if ($3 && $1->local == NULL)
				{
					static Dtdisc_t disc_key = {
						.key = offsetof(Exassoc_t, key),
						.size = sizeof(Extype_t),
						.freef = free,
						.comparf = cmpKey,
					};
					static Dtdisc_t disc_name = {
						.key = offsetof(Exassoc_t, name),
						.freef = free,
					};
					Dtdisc_t *const disc = $3 == INTEGER ? &disc_key : &disc_name;
					if (!($1->local = dtopen(disc, Dtoset)))
						exerror("%s: cannot initialize associative array", $1->name);
					$1->index_type = $3; /* -1 indicates no typechecking */
				}
				if ($4)
				{
					if ($4->type != $1->type)
					{
						$4->type = $1->type;
						$4->data.operand.right = excast(expr.program, $4->data.operand.right, $1->type, NULL, 0);
					}
					$4->data.operand.left = exnewnode(expr.program, DYNAMIC, false, $1->type, NULL, NULL);
					$4->data.operand.left->data.variable.symbol = $1;
					$$ = $4;
				}
				else if (!$3)
					$1->value->data.value = exzero($1->type);
			}
		}
		;

dcl_name	:	NAME
		|	DYNAMIC
		|	ID
		|	FUNCTION
		;

name		:	NAME
		|	DYNAMIC
		;

else_opt	:	/* empty */
		{
			$$ = 0;
		}
		|	ELSE statement
		{
			$$ = $2;
		}
		;

expr_opt	:	/* empty */
		{
			$$ = 0;
		}
		|	expr
		;

expr		:	'(' expr ')'
		{
			$$ = $2;
		}
		|	'(' DECLARE ')' expr	%prec CAST
		{
			$$ = ($4->type == $2->type) ? $4 : excast(expr.program, $4, $2->type, NULL, 0);
		}
		|	expr '<' expr
		{
			long rel;

		relational:
			rel = INTEGER;
			goto coerce;
		binary:
			rel = 0;
		coerce:
			if (!$1->type)
			{
				if (!$3->type)
					$1->type = $3->type = rel ? STRING : INTEGER;
				else
					$1->type = $3->type;
			}
			else if (!$3->type)
				$3->type = $1->type;
			if ($1->type != $3->type)
			{
				if ($1->type == STRING)
					$1 = excast(expr.program, $1, $3->type, $3, 0);
				else if ($3->type == STRING)
					$3 = excast(expr.program, $3, $1->type, $1, 0);
				else if ($1->type == FLOATING)
					$3 = excast(expr.program, $3, FLOATING, $1, 0);
				else if ($3->type == FLOATING)
					$1 = excast(expr.program, $1, FLOATING, $3, 0);
			}
			if (!rel)
				rel = ($1->type == STRING) ? STRING : (($1->type == UNSIGNED) ? UNSIGNED : $3->type);
			$$ = exnewnode(expr.program, $2, true, rel, $1, $3);
			if (!expr.program->errors && $1->op == CONSTANT && $3->op == CONSTANT)
			{
				$$->data.constant.value = exeval(expr.program, $$, NULL);
				/* If a constant string, re-allocate from program heap. This is because the
				 * value was constructed from string operators, which create a value in the 
				 * temporary heap, which is cleared when exeval is called again. 
				 */
				if ($$->type == STRING) {
					$$->data.constant.value.string =
						vmstrdup(expr.program->vm, $$->data.constant.value.string);
				}
				$$->binary = false;
				$$->op = CONSTANT;
				exfreenode(expr.program, $1);
				exfreenode(expr.program, $3);
			}
			else if (!BUILTIN($1->type) || !BUILTIN($3->type)) {
				checkBinary(expr.program, $1, $$, $3);
			}
		}
		|	expr '-' expr
		{
			goto binary;
		}
		|	expr '*' expr
		{
			goto binary;
		}
		|	expr '/' expr
		{
			goto binary;
		}
		|	expr '%' expr
		{
			goto binary;
		}
		|	expr LSH expr
		{
			goto binary;
		}
		|	expr RSH expr
		{
			goto binary;
		}
		|	expr '>' expr
		{
			goto relational;
		}
		|	expr LE expr
		{
			goto relational;
		}
		|	expr GE expr
		{
			goto relational;
		}
		|	expr EQ expr
		{
			goto relational;
		}
		|	expr NE expr
		{
			goto relational;
		}
		|	expr '&' expr
		{
			goto binary;
		}
		|	expr '|' expr
		{
			goto binary;
		}
		|	expr '^' expr
		{
			goto binary;
		}
		|	expr '+' expr
		{
			goto binary;
		}
		|	expr AND expr
		{
		logical:
			if ($1->type == STRING)
				$1 = exnewnode(expr.program, S2B, true, INTEGER, $1, NULL);
			else if (!BUILTIN($1->type))
				$1 = excast(expr.program, $1, INTEGER, NULL, 0);
			if ($3->type == STRING)
				$3 = exnewnode(expr.program, S2B, true, INTEGER, $3, NULL);
			else if (!BUILTIN($3->type))
				$3 = excast(expr.program, $3, INTEGER, NULL, 0);
			goto binary;
		}
		|	expr OR expr
		{
			goto logical;
		}
		|	expr ',' expr
		{
			if ($1->op == CONSTANT)
			{
				exfreenode(expr.program, $1);
				$$ = $3;
			}
			else
				$$ = exnewnode(expr.program, ',', true, $3->type, $1, $3);
		}
		|	expr '?' {expr.nolabel=1;} expr ':' {expr.nolabel=0;} expr
		{
			if (!$4->type)
			{
				if (!$7->type)
					$4->type = $7->type = INTEGER;
				else
					$4->type = $7->type;
			}
			else if (!$7->type)
				$7->type = $4->type;
			if ($1->type == STRING)
				$1 = exnewnode(expr.program, S2B, true, INTEGER, $1, NULL);
			else if (!INTEGRAL($1->type))
				$1 = excast(expr.program, $1, INTEGER, NULL, 0);
			if ($4->type != $7->type)
			{
				if ($4->type == STRING || $7->type == STRING)
					exerror("if statement string type mismatch");
				else if ($4->type == FLOATING)
					$7 = excast(expr.program, $7, FLOATING, NULL, 0);
				else if ($7->type == FLOATING)
					$4 = excast(expr.program, $4, FLOATING, NULL, 0);
			}
			if ($1->op == CONSTANT)
			{
				if ($1->data.constant.value.integer)
				{
					$$ = $4;
					exfreenode(expr.program, $7);
				}
				else
				{
					$$ = $7;
					exfreenode(expr.program, $4);
				}
				exfreenode(expr.program, $1);
			}
			else
				$$ = exnewnode(expr.program, '?', true, $4->type, $1, exnewnode(expr.program, ':', true, $4->type, $4, $7));
		}
		|	'!' expr
		{
		iunary:
			if ($2->type == STRING)
				$2 = exnewnode(expr.program, S2B, true, INTEGER, $2, NULL);
			else if (!INTEGRAL($2->type))
				$2 = excast(expr.program, $2, INTEGER, NULL, 0);
		unary:
			$$ = exnewnode(expr.program, $1, true, $2->type == UNSIGNED ? INTEGER : $2->type, $2, NULL);
			if ($2->op == CONSTANT)
			{
				$$->data.constant.value = exeval(expr.program, $$, NULL);
				$$->binary = false;
				$$->op = CONSTANT;
				exfreenode(expr.program, $2);
			}
			else if (!BUILTIN($2->type)) {
				checkBinary(expr.program, $2, $$, 0);
			}
		}
		|	'#' DYNAMIC
		{
			if ($2->local == NULL)
              			exerror("cannot apply '#' operator to non-array %s", $2->name);
			$$ = exnewnode(expr.program, '#', false, INTEGER, NULL, NULL);
			$$->data.variable.symbol = $2;
		}
		|	'#' ARRAY
		{
			$$ = exnewnode(expr.program, '#', false, INTEGER, NULL, NULL);
			$$->data.variable.symbol = $2;
		}
		|	'~' expr
		{
			goto iunary;
		}
		|	'-' expr	%prec UNARY
		{
			goto unary;
		}
		|	'+' expr	%prec UNARY
		{
			$$ = $2;
		}
		|	'&' variable	%prec UNARY
		{
			$$ = exnewnode(expr.program, ADDRESS, false, T($2->type), $2, NULL);
		}
		|	ARRAY '[' args ']'
		{
			$$ = exnewnode(expr.program, ARRAY, true, T($1->type), call(0, $1, $3), $3);
		}
		|	FUNCTION '(' args ')'
		{
			$$ = exnewnode(expr.program, FUNCTION, true, T($1->type), call(0, $1, $3), $3);
		}
		|	GSUB '(' args ')'
		{
			$$ = exnewsub (expr.program, $3, GSUB);
		}
		|	SUB '(' args ')'
		{
			$$ = exnewsub (expr.program, $3, SUB);
		}
		|	SUBSTR '(' args ')'
		{
			$$ = exnewsubstr (expr.program, $3);
		}
		|	splitop '(' expr ',' DYNAMIC ')'
		{
			$$ = exnewsplit (expr.program, $1->index, $5, $3, NULL);
		}
		|	splitop '(' expr ',' DYNAMIC ',' expr ')'
		{
			$$ = exnewsplit (expr.program, $1->index, $5, $3, $7);
		}
		|	EXIT '(' expr ')'
		{
			if (!INTEGRAL($3->type))
				$3 = excast(expr.program, $3, INTEGER, NULL, 0);
			$$ = exnewnode(expr.program, EXIT, true, INTEGER, $3, NULL);
		}
		|	RAND '(' ')'
		{
			$$ = exnewnode(expr.program, RAND, false, FLOATING, NULL, NULL);
		}
		|	SRAND '(' ')'
		{
			$$ = exnewnode(expr.program, SRAND, false, INTEGER, NULL, NULL);
		}
		|	SRAND '(' expr ')'
		{
			if (!INTEGRAL($3->type))
				$3 = excast(expr.program, $3, INTEGER, NULL, 0);
			$$ = exnewnode(expr.program, SRAND, true, INTEGER, $3, NULL);
		}
		|	PROCEDURE '(' args ')'
		{
			$$ = exnewnode(expr.program, CALL, true, $1->type, NULL, $3);
			$$->data.call.procedure = $1;
		}
		|	PRINT '(' args ')'
		{
			$$ = exprint(expr.program, $1, $3);
		}
		|	print '(' args ')'
		{
			$$ = exnewnode(expr.program, $1->index, false, $1->type, NULL, NULL);
			if ($3 && $3->data.operand.left->type == INTEGER)
			{
				$$->data.print.descriptor = $3->data.operand.left;
				$3 = $3->data.operand.right;
			}
			else 
				switch ($1->index)
				{
				case QUERY:
					$$->data.print.descriptor = exnewnode(expr.program, CONSTANT, false, INTEGER, NULL, NULL);
					$$->data.print.descriptor->data.constant.value.integer = 2;
					break;
				case PRINTF:
					$$->data.print.descriptor = exnewnode(expr.program, CONSTANT, false, INTEGER, NULL, NULL);
					$$->data.print.descriptor->data.constant.value.integer = 1;
					break;
				case SPRINTF:
					$$->data.print.descriptor = 0;
					break;
				}
			$$->data.print.args = preprint($3);
		}
		|	scan '(' args ')'
		{
			Exnode_t*	x;

			$$ = exnewnode(expr.program, $1->index, false, $1->type, NULL, NULL);
			if ($3 && $3->data.operand.left->type == INTEGER)
			{
				$$->data.scan.descriptor = $3->data.operand.left;
				$3 = $3->data.operand.right;
			}
			else 
				switch ($1->index)
				{
				case SCANF:
					$$->data.scan.descriptor = 0;
					break;
				case SSCANF:
					if ($3 && $3->data.operand.left->type == STRING)
					{
						$$->data.scan.descriptor = $3->data.operand.left;
						$3 = $3->data.operand.right;
					}
					else
						exerror("%s: string argument expected", $1->name);
					break;
				}
			if (!$3 || !$3->data.operand.left || $3->data.operand.left->type != STRING)
				exerror("%s: format argument expected", $1->name);
			$$->data.scan.format = $3->data.operand.left;
			for (x = $$->data.scan.args = $3->data.operand.right; x; x = x->data.operand.right)
			{
				if (x->data.operand.left->op != ADDRESS)
					exerror("%s: address argument expected", $1->name);
				x->data.operand.left = x->data.operand.left->data.operand.left;
			}
		}
		|	variable assign
		{
			if ($2)
			{
				if ($1->op == ID && !expr.program->disc->setf)
					exerror("%s: variable assignment not supported", $1->data.variable.symbol->name);
				else
				{
					if (!$1->type)
						$1->type = $2->type;
					else if ($2->type != $1->type)
					{
						$2->type = $1->type;
						$2->data.operand.right = excast(expr.program, $2->data.operand.right, $1->type, NULL, 0);
					}
					$2->data.operand.left = $1;
					$$ = $2;
				}
			}
		}
		|	INC variable
		{
		pre:
			if ($2->type == STRING)
				exerror("++ and -- invalid for string variables");
			$$ = exnewnode(expr.program, $1, false, $2->type, $2, NULL);
			$$->subop = PRE;
		}
		|	variable INC
		{
		pos:
			if ($1->type == STRING)
				exerror("++ and -- invalid for string variables");
			$$ = exnewnode(expr.program, $2, false, $1->type, $1, NULL);
			$$->subop = POS;
		}
		|	expr IN_OP DYNAMIC
		{
			if ($3->local == NULL)
              			exerror("cannot apply IN to non-array %s", $3->name);
			if ($3->index_type > 0 && $1->type != $3->index_type)
            		    exerror("%s indices must have type %s, not %s", 
				$3->name, extypename(expr.program, $3->index_type),extypename(expr.program, $1->type));
			$$ = exnewnode(expr.program, IN_OP, false, INTEGER, NULL, NULL);
			$$->data.variable.symbol = $3;
			$$->data.variable.index = $1;
		}
		|	expr IN_OP ARRAY
		{
			if ($3->index_type > 0 && $1->type != $3->index_type)
            		    exerror("%s indices must have type %s, not %s",
				$3->name, extypename(expr.program, $3->index_type),extypename(expr.program, $1->type));
			$$ = exnewnode(expr.program, IN_OP, false, INTEGER, NULL, NULL);
			$$->data.variable.symbol = $3;
			$$->data.variable.index = $1;
		}
		|	DEC variable
		{
			goto pre;
		}
		|	variable DEC
		{
			goto pos;
		}
		|	constant
		;

splitop		:	SPLIT
		|	TOKENS
		;
constant	:	CONSTANT
		{
			$$ = exnewnode(expr.program, CONSTANT, false, $1->type, NULL, NULL);
			if (!expr.program->disc->reff)
				exerror("%s: identifier references not supported", $1->name);
			else
				$$->data.constant.value = expr.program->disc->reff(expr.program, $$, $1, NULL);
		}
		|	FLOATING
		{
			$$ = exnewnode(expr.program, CONSTANT, false, FLOATING, NULL, NULL);
			$$->data.constant.value.floating = $1;
		}
		|	INTEGER
		{
			$$ = exnewnode(expr.program, CONSTANT, false, INTEGER, NULL, NULL);
			$$->data.constant.value.integer = $1;
		}
		|	STRING
		{
			$$ = exnewnode(expr.program, CONSTANT, false, STRING, NULL, NULL);
			$$->data.constant.value.string = $1;
		}
		|	UNSIGNED
		{
			$$ = exnewnode(expr.program, CONSTANT, false, UNSIGNED, NULL, NULL);
			$$->data.constant.value.integer = $1;
		}
		;

print		:	PRINTF
		|	QUERY
		|	SPRINTF
		;

scan		:	SCANF
		|	SSCANF
		;

variable	:	ID members
		{
			$$ = makeVar(expr.program, $1, 0, 0, $2);
		}
		|	DYNAMIC index members
		{
			Exnode_t *n = exnewnode(expr.program, DYNAMIC, false, $1->type, NULL, NULL);
			n->data.variable.symbol = $1;
			n->data.variable.reference = 0;
			if (((n->data.variable.index = $2) == 0) != ($1->local == NULL))
				exerror("%s: is%s an array", $1->name, $1->local != NULL ? "" : " not");
			if ($1->local != NULL && $1->index_type > 0) {
				if ($2->type != $1->index_type)
					exerror("%s: indices must have type %s, not %s", 
						$1->name, extypename(expr.program, $1->index_type),extypename(expr.program, $2->type));
			}
			if ($3) {
				n->data.variable.dyna = exnewnode(expr.program, 0, false, 0, NULL, NULL);
				$$ = makeVar(expr.program, $1, $2, n, $3);
			}
			else $$ = n;
		}
		|	NAME
		{
			$$ = exnewnode(expr.program, ID, false, STRING, NULL, NULL);
			$$->data.variable.symbol = $1;
			$$->data.variable.reference = 0;
			$$->data.variable.index = 0;
			$$->data.variable.dyna = 0;
			if (!(expr.program->disc->flags & EX_UNDECLARED))
				exerror("unknown identifier");
		}
		;

array		:	/* empty */
		{
			$$ = 0;
		}
		|	'[' ']'
		{
			$$ = -1;
		}
		|	'[' DECLARE ']'
		{
			/* If DECLARE is VOID, its type is 0, so this acts like
			 * the empty case.
			 */
			if (INTEGRAL($2->type))
				$$ = INTEGER;
			else
				$$ = $2->type;
				
		}
		;

index		:	/* empty */
		{
			$$ = 0;
		}
		|	'[' expr ']'
		{
			$$ = $2;
		}
		;

args		:	/* empty */
		{
			$$ = 0;
		}
		|	arg_list
		{
			$$ = $1->data.operand.left;
			$1->data.operand.left = $1->data.operand.right = 0;
			exfreenode(expr.program, $1);
		}
		;

arg_list	:	expr		%prec ','
		{
			$$ = exnewnode(expr.program, ',', true, 0, exnewnode(expr.program, ',', true, $1->type, $1, NULL), NULL);
			$$->data.operand.right = $$->data.operand.left;
		}
		|	arg_list ',' expr
		{
			$1->data.operand.right = $1->data.operand.right->data.operand.right = exnewnode(expr.program, ',', true, $1->type, $3, NULL);
		}
		;

formals		:	/* empty */
		{
			$$ = 0;
		}
		|	DECLARE
		{
			$$ = 0;
			if ($1->type)
				exerror("(void) expected");
		}
		|	formal_list
		;

formal_list	:	formal_item
		{
			$$ = exnewnode(expr.program, ',', true, $1->type, $1, NULL);
		}
		|	formal_list ',' formal_item
		{
			Exnode_t*	x;
			Exnode_t*	y;

			$$ = $1;
			for (x = $1; (y = x->data.operand.right); x = y);
			x->data.operand.right = exnewnode(expr.program, ',', true, $3->type, $3, NULL);
		}
		;

formal_item	:	DECLARE {expr.declare=$1->type;} name
		{
			if ($1->type == 0) {
				exerror("%s: parameters to functions cannot be void typed", $3->name);
			}
			$$ = exnewnode(expr.program, ID, false, $1->type, NULL, NULL);
			$$->data.variable.symbol = $3;
			$3->lex = DYNAMIC;
			$3->type = $1->type;
			$3->value = exnewnode(expr.program, 0, false, 0, NULL, NULL);
			expr.procedure->data.procedure.arity++;
			expr.declare = 0;
		}
		;

members	:	/* empty */
		{
			$$ = expr.refs = 0;
		}
		|	member
		{
			Exref_t*	r;

			r = ALLOCATE(expr.program, Exref_t);
			*r = (Exref_t){0};
			r->symbol = $1;
			expr.refs = r;
			r->next = 0;
			r->index = 0;
			$$ = expr.refs;
		}
		|	'.' ID member
		{
			Exref_t*	r;
			Exref_t*	l;

			r = ALLOCATE(expr.program, Exref_t);
			*r = (Exref_t){0};
			r->symbol = $3;
			r->index = 0;
			r->next = 0;
			l = ALLOCATE(expr.program, Exref_t);
			*l = (Exref_t){0};
			l->symbol = $2;
			l->index = 0;
			l->next = r;
			expr.refs = l;
			$$ = expr.refs;
		}
		;

member	:	'.' ID
		{
			$$ = $2;
		}
		|	'.' NAME
		{
			$$ = $2;
		}
		;
assign		:	/* empty */
		{
			$$ = 0;
		}
		|	'=' expr
		{
			$$ = exnewnode(expr.program, '=', true, $2->type, NULL, $2);
			$$->subop = $1;
		}
		;

initialize	:	assign
		|	'(' {
				if (expr.procedure)
					exerror("%s: nested function definitions not supported", expr.id->name);
				expr.procedure = exnewnode(expr.program, PROCEDURE, true, expr.declare, NULL, NULL);
				if (!streq(expr.id->name, "begin"))
				{
					static Dtdisc_t disc = {.key = offsetof(Exid_t, name)};
					if (!(expr.procedure->data.procedure.frame = dtopen(&disc, Dtset)) || !dtview(expr.procedure->data.procedure.frame, expr.program->symbols))
						exnospace();
					expr.program->symbols = expr.program->frame = expr.procedure->data.procedure.frame;
				}
				expr.declare = 0;
			} formals {
				expr.id->lex = PROCEDURE;
				expr.id->type = expr.procedure->type;
				expr.declare = 0;
			} ')' '{' statement_list '}'
		{
			$$ = expr.procedure;
			expr.procedure = NULL;
			if (expr.program->frame)
			{
				expr.program->symbols = expr.program->frame->view;
				dtview(expr.program->frame, NULL);
				expr.program->frame = NULL;
			}
			// dictionary of locals no longer required, now that we have parsed the body
			(void)dtclose($$->data.procedure.frame);
			$$->data.procedure.frame = NULL;
			$$->data.procedure.args = $3;
			$$->data.procedure.body = excast(expr.program, $7, $$->type, NULL, 0);

			/*
			 * NOTE: procedure definition was slipped into the
			 *	 declaration initializer statement production,
			 *	 therefore requiring the statement terminator
			 */

			exunlex(expr.program, ';');
		}
		;

%%

const char *exop(size_t index) {

  /* yytname is generated by the %token-table directive */

  /* find the index of MINTOKEN */
  size_t minid;
  for (minid = 0; yytname[minid] != NULL; ++minid) {
    if (strcmp(yytname[minid], "MINTOKEN") == 0) {
      break;
    }
  }

  assert(yytname[minid] != NULL
    && "failed to find MINTOKEN; incorrect token list in exparse.y?");

  /* find the requested token */
  {
    size_t i, j;
    for (i = j = minid; yytname[i] != NULL; ++i) {

      /* if this token is not a word, skip it */
      {
        size_t k;
        for (k = 0; yytname[i][k] != '\0'; ++k) {
          if (yytname[i][k] != '_' && !gv_isalnum(yytname[i][k])) {
            break;
          }
        }
        if (yytname[i][k] != '\0') {
          continue;
        }
      }

      if (j == index + minid) {
        return yytname[i];
      }
      ++j;
    }
  }

  /* failed to find the requested token */
  return NULL;
}

void ex_error(const char *message) {
  exerror("%s", message);
}

#include <expr/exgram.h>
