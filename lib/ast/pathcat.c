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
 * AT&T Bell Laboratories
 *
 * single dir support for pathaccess()
 */

#include <ast/ast.h>
#include <cgraph/agxbuf.h>

const char *pathcat(agxbuf *path, const char *dirs, const char *a,
                    const char *b) {
  const char sep = ':';

  while (*dirs && *dirs != sep)
    agxbputc(path, *dirs++);
  if (agxblen(path) > 0)
    agxbputc(path, '/');
  if (a) {
    agxbput(path, a);
    if (b)
      agxbputc(path, '/');
  } else if (!b)
    b = ".";
  if (b)
    agxbput(path, b);
  return *dirs ? ++dirs : 0;
}
