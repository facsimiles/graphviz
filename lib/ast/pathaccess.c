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
 * return path to file a/b using : separated dirs
 * both a and b may be 0
 * path returned in path buffer
 */

#include <ast/ast.h>
#include <cgraph/agxbuf.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

char *pathaccess(const char *dirs, const char *a, const char *b) {
  int m = 0;
  struct stat st;
  agxbuf path = {0};

#ifdef EFF_ONLY_OK
  m |= EFF_ONLY_OK;
#endif
  do {
    dirs = pathcat(&path, dirs, a, b);
    const char *p = agxbuse(&path);
    if (!access(p, m)) {
      if (stat(p, &st) || S_ISDIR(st.st_mode))
        continue;
#ifdef _WIN32
      char *resolved = _fullpath(NULL, p, 0);
#else
      char *resolved = realpath(p, NULL);
#endif
      agxbfree(&path);
      return resolved;
    }
  } while (dirs);
  agxbfree(&path);
  return (0);
}
