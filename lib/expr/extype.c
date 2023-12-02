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
