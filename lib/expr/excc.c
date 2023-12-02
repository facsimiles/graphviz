/*************************************************************************
 * Copyright (c) 2011 AT&T Intellectual Property 
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v2.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v20.html
 *
 * Contributors: Details at https://graphviz.org
 *************************************************************************/

/*
 * Glenn Fowler
 * AT&T Research
 *
 * expression library C program generator
 */

#include <expr/exlib.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <util/agxbuf.h>
#include <util/exit.h>

/// excc() state
typedef struct {
	Expr_t*		expr;		/* exopen() state		*/
	Exdisc_t*	disc;		/* exopen() discipline		*/
	int		tmp;		/* temp var index		*/
	agxbuf *text; ///< result of dumping
} Excc_t;

static const char	quote[] = "\"";

static void		gen(Excc_t*, Exnode_t*);

/*
 * return C name for op
 */

char *exopname(long op) {
	static char	buf[16];

	switch (op)
	{
	case '!':
		return "!";
	case '%':
		return "%";
	case '&':
		return "&";
	case '(':
		return "(";
	case '*':
		return "*";
	case '+':
		return "+";
	case ',':
		return ",";
	case '-':
		return "-";
	case '/':
		return "/";
	case ':':
		return ":";
	case '<':
		return "<";
	case '=':
		return "=";
	case '>':
		return ">";
	case '?':
		return "?";
	case '^':
		return "^";
	case '|':
		return "|";
	case '~':
		return "~";
	case AND:
		return "&&";
	case EQ:
		return "==";
	case GE:
		return ">=";
	case LE:
		return "<=";
	case LSH:
		return "<<";
	case NE:
		return "!=";
	case OR:
		return "||";
	case RSH:
		return ">>";
	default:
		break;
	}
	snprintf(buf, sizeof(buf) - 1, "(OP=%03lo)", op);
	return buf;
}

/*
 * generate printf()
 */

static void print(Excc_t *cc, Exnode_t *exnode) {
	Print_t*	x;

	if ((x = exnode->data.print.args))
	{
		char *quoted = fmtesq(x->format, quote);
		agxbprint(cc->text, "sfprintf(%s, \"%s", exnode->data.print.descriptor->op == CONSTANT && exnode->data.print.descriptor->data.constant.value.integer == 2 ? "sfstderr" : "sfstdout", quoted);
		free(quoted);
		while ((x = x->next)) {
			quoted = fmtesq(x->format, quote);
			agxbput(cc->text, quoted);
			free(quoted);
		}
		agxbputc(cc->text, '"');
		for (x = exnode->data.print.args; x; x = x->next)
		{
			if (x->arg)
			{
				for (size_t i = 0; i < elementsof(x->param) && x->param[i]; i++)
				{
					agxbput(cc->text, ", (");
					gen(cc, x->param[i]);
					agxbputc(cc->text, ')');
				}
				agxbput(cc->text, ", (");
				gen(cc, x->arg);
				agxbputc(cc->text, ')');
			}
		}
		agxbput(cc->text, ");\n");
	}
}

/*
 * generate scanf()
 */

static void scan(Excc_t *cc, Exnode_t *exnode) {
	Print_t*	x;

	if ((x = exnode->data.print.args))
	{
		char *quoted = fmtesq(x->format, quote);
		agxbprint(cc->text, "sfscanf(sfstdin, \"%s", quoted);
		free(quoted);
		while ((x = x->next)) {
			quoted = fmtesq(x->format, quote);
			agxbput(cc->text, quoted);
			free(quoted);
		}
		agxbputc(cc->text, '"');
		for (x = exnode->data.print.args; x; x = x->next)
		{
			if (x->arg)
			{
				for (size_t i = 0; i < elementsof(x->param) && x->param[i]; i++)
				{
					agxbput(cc->text, ", &(");
					gen(cc, x->param[i]);
					agxbputc(cc->text, ')');
				}
				agxbput(cc->text, ", &(");
				gen(cc, x->arg);
				agxbputc(cc->text, ')');
			}
		}
		agxbput(cc->text, ");\n");
	}
}

/*
 * internal excc
 */

static void gen(Excc_t *cc, Exnode_t *exnode) {
	Exnode_t*	x;
	Exnode_t*	y;
	int		n;
	int		m;
	char*			s;
	Extype_t*		v;
	Extype_t**		p;

	if (!exnode)
		return;
	if (exnode->op == CALL) {
		agxbprint(cc->text, "%s(", exnode->data.call.procedure->name);
		if (exnode->data.call.args)
			gen(cc, exnode->data.call.args);
		agxbputc(cc->text, ')');
		return;
	}
	x = exnode->data.operand.left;
	switch (exnode->op)
	{
	case BREAK:
		agxbput(cc->text, "break;\n");
		return;
	case CONTINUE:
		agxbput(cc->text, "continue;\n");
		return;
	case CONSTANT:
		switch (exnode->type)
		{
		case FLOATING:
			agxbprint(cc->text, "%g", exnode->data.constant.value.floating);
			break;
		case STRING: {
			char *quoted = fmtesq(exnode->data.constant.value.string, quote);
			agxbprint(cc->text, "\"%s\"", quoted);
			free(quoted);
			break;
		}
		case UNSIGNED:
			agxbprint(cc->text, "%llu",
			          (long long unsigned)exnode->data.constant.value.integer);
			break;
		default:
			agxbprint(cc->text, "%lld", exnode->data.constant.value.integer);
			break;
		}
		return;
	case DEC:
		agxbprint(cc->text, "%s--", x->data.variable.symbol->name);
		return;
	case DYNAMIC:
		agxbput(cc->text, exnode->data.variable.symbol->name);
		return;
	case EXIT:
		agxbput(cc->text, "exit(");
		gen(cc, x);
		agxbput(cc->text, ");\n");
		return;
	case FUNCTION:
		gen(cc, x);
		agxbputc(cc->text, '(');
		if ((y = exnode->data.operand.right)) {
			gen(cc, y);
		}
		agxbputc(cc->text, ')');
		return;
	case RAND:
		agxbput(cc->text, "rand();\n");
		return;
	case SRAND:
		if (exnode->binary) {
			agxbput(cc->text, "srand(");
			gen(cc, x);
			agxbput(cc->text, ");\n");
		} else
			agxbput(cc->text, "srand();\n");
		return;
   	case GSUB:
   	case SUB:
   	case SUBSTR:
		s = (exnode->op == GSUB ? "gsub(" : exnode->op == SUB ? "sub(" : "substr(");
		agxbput(cc->text, s);
		gen(cc, exnode->data.string.base);
		agxbput(cc->text, ", ");
		gen(cc, exnode->data.string.pat);
		if (exnode->data.string.repl) {
			agxbput(cc->text, ", ");
			gen(cc, exnode->data.string.repl);
		}
		agxbputc(cc->text, ')');
		return;
   	case IN_OP:
		gen(cc, exnode->data.variable.index);
		agxbprint(cc->text, " in %s", exnode->data.variable.symbol->name);
		return;
	case IF:
		agxbput(cc->text, "if (");
		gen(cc, x);
		agxbput(cc->text, ") {\n");
		gen(cc, exnode->data.operand.right->data.operand.left);
		if (exnode->data.operand.right->data.operand.right)
		{
			agxbput(cc->text, "} else {\n");
			gen(cc, exnode->data.operand.right->data.operand.right);
		}
		agxbput(cc->text, "}\n");
		return;
	case FOR:
		agxbput(cc->text, "for (;");
		gen(cc, x);
		agxbput(cc->text, ");");
		if (exnode->data.operand.left)
		{
			agxbputc(cc->text, '(');
			gen(cc, exnode->data.operand.left);
			agxbputc(cc->text, ')');
		}
		agxbput(cc->text, ") {");
		if (exnode->data.operand.right)
			gen(cc, exnode->data.operand.right);
		agxbputc(cc->text, '}');
		return;
	case ID:
		agxbput(cc->text, exnode->data.variable.symbol->name);
		return;
	case INC:
		agxbprint(cc->text, "%s++", x->data.variable.symbol->name);
		return;
	case ITERATE:
	case ITERATOR:
		if (exnode->op == DYNAMIC)
		{
			agxbprint(cc->text, "{ Exassoc_t* tmp_%d;", ++cc->tmp);
			agxbprint(cc->text, "for (tmp_%d = (Exassoc_t*)dtfirst(%s); tmp_%d && (%s = tmp_%d->name); tmp_%d = (Exassoc_t*)dtnext(%s, tmp_%d)) {", cc->tmp, exnode->data.generate.array->data.variable.symbol->name, cc->tmp, exnode->data.generate.index->name, cc->tmp, cc->tmp, exnode->data.generate.array->data.variable.symbol->name, cc->tmp);
			gen(cc, exnode->data.generate.statement);
			agxbput(cc->text, "} }");
		}
		return;
	case PRINT:
		agxbput(cc->text, "print");
		if (x)
			gen(cc, x);
		else
			agxbput(cc->text, "()");
		return;
	case PRINTF:
		print(cc, exnode);
		return;
	case RETURN:
		agxbput(cc->text, "return(");
		gen(cc, x);
		agxbput(cc->text, ");\n");
		return;
	case SCANF:
		scan(cc, exnode);
		return;
	case SPLIT:
	case TOKENS:
		if (exnode->op == SPLIT)
			agxbput(cc->text, "split (");
		else
			agxbput(cc->text, "tokens (");
		gen(cc, exnode->data.split.string);
		agxbprint(cc->text, ", %s", exnode->data.split.array->name);
		if (exnode->data.split.seps) {
			agxbputc(cc->text, ',');
			gen(cc, exnode->data.split.seps);
		}
		agxbputc(cc->text, ')');
		return;
	case SWITCH: {
		long t = x->type;
		agxbprint(cc->text, "{ %s tmp_%d = ", extype(t), ++cc->tmp);
		gen(cc, x);
		agxbputc(cc->text, ';');
		x = exnode->data.operand.right;
		y = x->data.select.statement;
		n = 0;
		while ((x = x->data.select.next))
		{
			if (n)
				agxbput(cc->text, "else ");
			if (!(p = x->data.select.constant))
				y = x->data.select.statement;
			else
			{
				m = 0;
				while ((v = *p++))
				{
					if (m)
						agxbput(cc->text, "||");
					else
					{
						m = 1;
						agxbput(cc->text, "if (");
					}
					if (t == STRING) {
						char *quoted = fmtesq(v->string, quote);
						agxbprint(cc->text, "strmatch(tmp_%d, \"%s\")", cc->tmp, quoted);
						free(quoted);
					} else {
						agxbprint(cc->text, "tmp_%d == ", cc->tmp);
						switch (t)
						{
						case INTEGER:
						case UNSIGNED:
							agxbprint(cc->text, "%llu",
							          (unsigned long long)v->integer);
							break;
						default:
							agxbprint(cc->text, "%g", v->floating);
							break;
						}
					}
				}
				agxbput(cc->text, ") {");
				gen(cc, x->data.select.statement);
				agxbputc(cc->text, '}');
			}
		}
		if (y)
		{
			if (n)
				agxbput(cc->text, "else ");
			agxbputc(cc->text, '{');
			gen(cc, y);
			agxbputc(cc->text, '}');
		}
		agxbputc(cc->text, '}');
		return;
	}
	case UNSET:
		agxbprint(cc->text, "unset(%s", exnode->data.variable.symbol->name);
		if (exnode->data.variable.index) {
			agxbputc(cc->text, ',');
			gen(cc, exnode->data.variable.index);
		}
		agxbputc(cc->text, ')');
		return;
	case WHILE:
		agxbput(cc->text, "while (");
		gen(cc, x);
		agxbput(cc->text, ") {");
		if (exnode->data.operand.right)
			gen(cc, exnode->data.operand.right);
		agxbputc(cc->text, '}');
		return;
    case '#':
		agxbprint(cc->text, "# %s", exnode->data.variable.symbol->name);
		return;
	case '=':
		agxbprint(cc->text, "(%s%s=", x->data.variable.symbol->name, exnode->subop == '=' ? "" : exopname(exnode->subop));
		gen(cc, exnode->data.operand.right);
		agxbputc(cc->text, ')');
		return;
	case ';':
		for (;;)
		{
			if (!(x = exnode->data.operand.right))
				switch (exnode->data.operand.left->op)
				{
				case FOR:
				case IF:
				case PRINTF:
				case PRINT:
				case RETURN:
				case WHILE:
					break;
				default:
					agxbput(cc->text, "_value=");
					break;
				}
			gen(cc, exnode->data.operand.left);
			agxbput(cc->text, ";\n");
			if (!(exnode = x))
				break;
			switch (exnode->op)
			{
			case ';':
				continue;
			case FOR:
			case IF:
			case PRINTF:
			case PRINT:
			case RETURN:
			case WHILE:
				break;
			default:
				agxbput(cc->text, "_%svalue=");
				break;
			}
			gen(cc, exnode);
			agxbput(cc->text, ";\n");
			break;
		}
		return;
	case ',':
		agxbputc(cc->text, '(');
		gen(cc, x);
		while ((exnode = exnode->data.operand.right) && exnode->op == ',')
		{
			agxbput(cc->text, "), (");
			gen(cc, exnode->data.operand.left);
		}
		if (exnode)
		{
			agxbput(cc->text, "), (");
			gen(cc, exnode);
		}
		agxbputc(cc->text, ')');
		return;
	case '?':
		agxbputc(cc->text, '(');
		gen(cc, x);
		agxbput(cc->text, ") ? (");
		gen(cc, exnode->data.operand.right->data.operand.left);
		agxbput(cc->text, ") : (");
		gen(cc, exnode->data.operand.right->data.operand.right);
		agxbputc(cc->text, ')');
		return;
	case AND:
		agxbputc(cc->text, '(');
		gen(cc, x);
		agxbput(cc->text, ") && (");
		gen(cc, exnode->data.operand.right);
		agxbputc(cc->text, ')');
		return;
	case OR:
		agxbputc(cc->text, '(');
		gen(cc, x);
		agxbput(cc->text, ") || (");
		gen(cc, exnode->data.operand.right);
		agxbputc(cc->text, ')');
		return;
	case F2I:
		agxbprint(cc->text, "(%s)(", extype(INTEGER));
		gen(cc, x);
		agxbputc(cc->text, ')');
		return;
	case I2F:
		agxbprint(cc->text, "(%s)(", extype(FLOATING));
		gen(cc, x);
		agxbputc(cc->text, ')');
		return;
	case S2I:
		agxbput(cc->text, "strtoll(");
		gen(cc, x);
		agxbput(cc->text, ",(char**)0,0)");
		return;
    case X2I:
		agxbput(cc->text, "X2I(");
		gen(cc, x);
		agxbputc(cc->text, ')');
		return;
	case X2X:
		agxbput(cc->text, "X2X(");
		gen(cc, x);
		agxbputc(cc->text, ')');
		return;
	}
	y = exnode->data.operand.right;
	if (x->type == STRING)
	{
		switch (exnode->op)
		{
		case S2B:
			agxbput(cc->text, "*(");
			gen(cc, x);
			agxbput(cc->text, ")!=0");
			return;
		case S2F:
			agxbput(cc->text, "strtod(");
			gen(cc, x);
			agxbput(cc->text, ",0)");
			return;
		case S2I:
			agxbput(cc->text, "strtol(");
			gen(cc, x);
			agxbput(cc->text, ",0,0)");
			return;
		case S2X:
			agxbput(cc->text, "** cannot convert string value to external **");
			return;
		case NE:
			agxbputc(cc->text, '!');
			/*FALLTHROUGH*/
		case EQ:
			agxbput(cc->text, "strmatch(");
			gen(cc, x);
			agxbputc(cc->text, ',');
			gen(cc, y);
			agxbputc(cc->text, ')');
			return;
		case '+':
		case '|':
		case '&':
		case '^':
		case '%':
		case '*':
			agxbput(cc->text, "** string bits not supported **");
			return;
		}
		switch (exnode->op)
		{
		case '<':
			s = "<0";
			break;
		case LE:
			s = "<=0";
			break;
		case GE:
			s = ">=0";
			break;
		case '>':
			s = ">0";
			break;
		default:
			s = "** unknown string op **";
			break;
		}
		agxbput(cc->text, "strcoll(");
		gen(cc, x);
		agxbputc(cc->text, ',');
		gen(cc, y);
		agxbprint(cc->text, ")%s", s);
		return;
	}
	else
	{
		if (!y)
			agxbput(cc->text, exopname(exnode->op));
		agxbputc(cc->text, '(');
		gen(cc, x);
		if (y)
		{
			agxbprint(cc->text, ")%s(", exopname(exnode->op));
			gen(cc, y);
		}
		agxbputc(cc->text, ')');
	}
	return;
}

/*
 * dump an expression tree to a buffer
 */

void exdump(Expr_t *ex, Exnode_t *node, agxbuf *xb) {
	Excc_t cc = {.expr = ex, .disc = ex->disc, .text = xb};
	if (node)
		gen(&cc, node);
	else
		for (Exid_t *sym = dtfirst(ex->symbols); sym; sym = dtnext(ex->symbols, sym))
			if (sym->lex == PROCEDURE && sym->value)
			{
				agxbprint(xb, "%s:\n", sym->name);
				gen(&cc, sym->value->data.procedure.body);
			}
	agxbputc(xb, '\n');
}
