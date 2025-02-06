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

#include <cstdio>
#include <cstdlib>
#include <sys/stat.h>
#include <sys/types.h>
#ifdef HAVE_SYS_MMAN_H
#include <sys/mman.h>
#endif

#include <common/render.h>
#include <common/utils.h>
#include <gvc/gvio.h>
#include <gvc/gvplugin_loadimage.h>
#include <util/agxbuf.h>

enum {
  FORMAT_PS_PS,
};

static void ps_freeimage(usershape_t *us) {
#ifdef HAVE_SYS_MMAN_H
  munmap(us->data, us->datasize);
#else
  delete[] us->data;
#endif
}

extern "C" {

/// usershape described by a postscript file
static void lasi_loadimage_ps(GVJ_t *job, usershape_t *us, boxf b, bool) {
  assert(job);
  assert(us);
  assert(us->name);

  if (us->data) {
    if (us->datafree != ps_freeimage) {
      us->datafree(us); // free incompatible cache data
      us->data = nullptr;
      us->datafree = nullptr;
      us->datasize = 0;
    }
  }

  if (!us->data) { // read file into cache
    int fd;
    struct stat statbuf;

    if (!gvusershape_file_access(us))
      return;
    fd = fileno(us->f);
    switch (us->type) {
    case FT_PS:
    case FT_EPS:
      fstat(fd, &statbuf);
      us->datasize = statbuf.st_size;
#ifdef HAVE_SYS_MMAN_H
      us->data = mmap(0, statbuf.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
      if (us->data == MAP_FAILED)
        us->data = nullptr;
#else
      us->data = new char[statbuf.st_size];
      fread(us->data, 1, (size_t)statbuf.st_size, us->f);
#endif
      us->must_inline = true;
      break;
    default:
      break;
    }
    if (us->data)
      us->datafree = ps_freeimage;
    gvusershape_file_release(us);
  }

  if (us->data) {
    gvprintf(job, "gsave %g %g translate newpath\n", b.LL.x - (double)(us->x),
             b.LL.y - (double)(us->y));
    if (us->must_inline)
      epsf_emit_body(job, us);
    else
      gvprintf(job, "user_shape_%d\n", us->macro_id);
    gvprintf(job, "grestore\n");
  }
}
}

static gvloadimage_engine_t engine_ps = {lasi_loadimage_ps};

extern "C" {

gvplugin_installed_t gvloadimage_lasi_types[] = {
    {FORMAT_PS_PS, "eps:lasi", -5, &engine_ps, nullptr},
    {FORMAT_PS_PS, "ps:lasi", -5, &engine_ps, nullptr},
    {0, nullptr, 0, nullptr, nullptr}};
}
