/*************************************************************************
 * Copyright (c) 2011 AT&T Intellectual Property 
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v2.0
 * which accompanies this distribution, and is available at
 * https://www.eclipse.org/org/documents/epl-2.0/EPL-2.0.html
 *
 * Contributors: Details at https://graphviz.org
 *************************************************************************/

/*
 * Glenn Fowler
 * AT&T Research
 *
 * expression library C program generator
 */

#include "config.h"

#include <expr/exlib.h>

/*
 * return C type name for type
 */

char *extype(long int type) {
	switch (type)
	{
	case FLOATING:
		return "double";
	case STRING:
		return "char*";
	case UNSIGNED:
		return "uintmax_t";
	}
	return "intmax_t";
}
