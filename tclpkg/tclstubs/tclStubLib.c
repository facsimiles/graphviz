/*************************************************************************
 * Copyright (c) 2011 AT&T Intellectual Property 
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v2.0
 * which accompanies this distribution, and is available at
 * https://www.eclipse.org/org/documents/epl-2.0/EPL-2.0.html
 *
 * Contributors: Details at https://graphviz.org
 *************************************************************************/

#include "config.h"

/* this dummy file is here to fool libtool and automake */
#include "tcl.h"

/* need to reference an entry point to fool Sun's ld */
void tclstubs_dummy(void);
void tclstubs_dummy(void)
{
    Tcl_InitStubs(0, 0, 0);
}
