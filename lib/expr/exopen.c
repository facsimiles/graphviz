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
 * expression library
 */

#include <expr/exlib.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/// destructor for `Exid_t`
static void free_exid(void *p) {
  Exid_t *const exid = p;

  // the `Exid_t` itself was allocated through the program’s `vm` allocator, so
  // will be automatically deallocated, but we need to manually clean up the
  // `local` set that is used for arrays
  dtclose(exid->local);
}

/*
 * allocate a new expression program environment
 */

Expr_t*
exopen(Exdisc_t* disc)
{
	Expr_t*	program;
	Exid_t*	sym;

	if (!(program = calloc(1, sizeof(Expr_t))))
		return 0;
	static Dtdisc_t symdisc = {.key = offsetof(Exid_t, name), .freef = free_exid};
	if (!(program->symbols = dtopen(&symdisc, Dtset)) ||
	    !(program->vm = vmopen()) ||
	    !(program->ve = vmopen()))
	{
		exclose(program);
		return 0;
	}
	program->id = "libexpr:expr";
	program->disc = disc;
	setcontext(program);
	program->file[0] = stdin;
	program->file[1] = stdout;
	program->file[2] = stderr;
	strcpy(program->main.name, "main");
	program->main.lex = PROCEDURE;
	program->main.index = PROCEDURE;
	dtinsert(program->symbols, &program->main);
	for (sym = exbuiltin; *sym->name; sym++)
		dtinsert(program->symbols, sym);
	if ((sym = disc->symbols))
		for (; *sym->name; sym++)
			dtinsert(program->symbols, sym);
	return program;
}
