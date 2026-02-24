/*************************************************************************
 * Copyright (c) 2011 AT&T Intellectual Property
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * https://www.eclipse.org/legal/epl-v10.html
 *
 * Contributors: Details at https://graphviz.org
 *************************************************************************/

#include "config.h"

#include "gml2gv.h"
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "openFile.h"
#include <getopt.h>
#include <util/agxbuf.h>
#include <util/alloc.h>
#include <util/debug.h>
#include <util/exit.h>
#include <util/unreachable.h>

static char *gname = "";
static FILE *outFile;
static char *CmdName;
static char **Files;

static FILE *getFile(void) {
  FILE *rv = NULL;
  static FILE *savef = NULL;
  static int cnt = 0;

  if (Files == NULL) {
    if (cnt++ == 0) {
      rv = stdin;
    }
  } else {
    if (savef)
      fclose(savef);
    while (Files[cnt]) {
      if ((rv = fopen(Files[cnt++], "r")) != 0)
        break;
      else
        fprintf(stderr, "Can't open %s\n", Files[cnt - 1]);
    }
  }
  savef = rv;
  return rv;
}

static char *useString = "Usage: %s [-v?] [-g<name>] [-o<file>] <files>\n\
  -g<name>  : use <name> as template for graph names\n\
  -v        : verbose mode\n\
  -o<file>  : output to <file> (stdout)\n\
  -?        : print usage\n\
If no files are specified, stdin is used\n";

static void usage(int v) {
  printf(useString, CmdName);
  graphviz_exit(v);
}

static char *cmdName(char *path) {
  char *sp;

  sp = strrchr(path, '/');
  if (sp)
    sp++;
  else
    sp = path;
  return sp;
}

static void initargs(int argc, char **argv) {
  int c;

  CmdName = cmdName(argv[0]);
  opterr = 0;
  while ((c = getopt(argc, argv, ":g:vo:")) != -1) {
    switch (c) {
    case 'g':
      gname = optarg;
      break;
    case 'v':
      Verbose = 1;
      break;
    case 'o':
      if (outFile != NULL)
        fclose(outFile);
      outFile = openFile(CmdName, optarg, "w");
      break;
    case ':':
      fprintf(stderr, "%s: option -%c missing argument\n", CmdName, optopt);
      usage(1);
      break;
    case '?':
      if (optopt == '?')
        usage(0);
      else {
        fprintf(stderr, "%s: option -%c unrecognized\n", CmdName, optopt);
        usage(1);
      }
      break;
    default:
      UNREACHABLE();
    }
  }

  argv += optind;
  argc -= optind;

  if (argc)
    Files = argv;
  if (!outFile)
    outFile = stdout;
}

static char *nameOf(agxbuf *buf, char *name, int cnt) {
  if (*name == '\0')
    return name;
  if (cnt) {
    agxbprint(buf, "%s%d", name, cnt);
    return agxbuse(buf);
  } else
    return name;
}

int main(int argc, char **argv) {
  Agraph_t *G;
  FILE *inFile;
  int gcnt, cnt, rv;
  agxbuf buf = {0};

  rv = 0;
  gcnt = 0;
  initargs(argc, argv);
  while ((inFile = getFile())) {
    cnt = 0;
    while ((G = gml_to_gv(nameOf(&buf, gname, gcnt), inFile, cnt, &rv))) {
      cnt++;
      gcnt++;
      GV_INFO("%s: %d nodes %d edges", agnameof(G), agnnodes(G), agnedges(G));
      agwrite(G, outFile);
      agclose(G);
      fflush(outFile);
    }
  }
  agxbfree(&buf);
  graphviz_exit(rv);
}
