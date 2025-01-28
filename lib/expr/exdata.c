/*************************************************************************
 * Copyright (c) 2011 AT&T Intellectual Property 
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * https://www.eclipse.org/legal/epl-v10.html
 *
 * Contributors: Details at https://graphviz.org
 *************************************************************************/

/*
 * Glenn Fowler
 * AT&T Research
 *
 * expression library readonly tables
 */

static const char id[] = "\n@(#)$Id: libexpr (AT&T Research) 2011-06-30 $\0\n";

#include <expr/exlib.h>

const char*	exversion = id + 10;

Exid_t		exbuiltin[] =
{

	/* id_string references the first entry */

	EX_ID("string",	DECLARE,	STRING,		STRING),

	// order not important after this point

	EX_ID("break",	BREAK,		BREAK,		0),
	EX_ID("case",	CASE,		CASE,		0),
	EX_ID("char",	DECLARE,	CHARACTER,		CHARACTER),
	EX_ID("continue",CONTINUE,	CONTINUE,	0),
	EX_ID("default",	DEFAULT,	DEFAULT,	0),
	EX_ID("double",	DECLARE,	FLOATING,	FLOATING),
	EX_ID("else",	ELSE,		ELSE,		0),
	EX_ID("exit",	EXIT,		EXIT,		INTEGER),
	EX_ID("for",	FOR,		FOR,		0),
	EX_ID("forr",	ITERATOR,	ITERATOR,	0),
	EX_ID("float",	DECLARE,	FLOATING,	FLOATING),
	EX_ID("gsub",	GSUB,		GSUB,		STRING),
	EX_ID("if",	IF,		IF,		0),
	EX_ID("in",	IN_OP,		IN_OP,		0),
	EX_ID("int",	DECLARE,	INTEGER,	INTEGER),
	EX_ID("long",	DECLARE,	INTEGER,	INTEGER),
	EX_ID("print",	PRINT,		PRINT,		INTEGER),
	EX_ID("printf",	PRINTF,		PRINTF,		INTEGER),
	EX_ID("query",	QUERY,		QUERY,		INTEGER),
	EX_ID("rand",	RAND,		RAND,		FLOATING),
	EX_ID("return",	RETURN,		RETURN,		0),
	EX_ID("scanf",	SCANF,		SCANF,		INTEGER),
	EX_ID("sscanf",	SSCANF,		SSCANF,		INTEGER),
	EX_ID("split",	SPLIT,		SPLIT,		INTEGER),
	EX_ID("sprintf",	SPRINTF,	SPRINTF,	STRING),
	EX_ID("srand",	SRAND,		SRAND,		INTEGER),
	EX_ID("sub",	SUB,		SUB,		STRING),
	EX_ID("substr",	SUBSTR,		SUBSTR,		STRING),
	EX_ID("switch",	SWITCH,		SWITCH,		0),
	EX_ID("tokens",	TOKENS,		TOKENS,		INTEGER),
	EX_ID("unset",	UNSET,		UNSET,		0),
	EX_ID("unsigned",DECLARE,	UNSIGNED,	UNSIGNED),
	EX_ID("void",	DECLARE,	VOIDTYPE,	0),
	EX_ID("while",	WHILE,		WHILE,		0),
	EX_ID({0},		0,		0,		0)

};
