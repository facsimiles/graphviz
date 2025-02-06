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

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include <common/types.h>
#include <gvc/gvio.h>
#include <gvc/gvplugin_device.h>
#include <util/alloc.h>
#include <util/base64.h>
#include <util/gv_math.h>

#ifdef HAVE_LIBZ
#include <zlib.h>
#endif

static void kitty_write(unsigned char *data, size_t data_size, unsigned width,
                        unsigned height, bool is_compressed) {
  const size_t chunk_size = 4096;
  char *output = gv_base64(data, data_size);
  size_t offset = 0;
  size_t size = gv_base64_size(data_size);

  while (offset < size) {
    int has_next_chunk = offset + chunk_size <= size;
    if (offset == 0) {
      printf("\033_Ga=T,f=32,s=%u,v=%u%s%s;", width, height,
             chunk_size < size ? ",m=1" : "", is_compressed ? ",o=z" : "");
    } else {
      printf("\033_Gm=%d;", has_next_chunk);
    }

    size_t this_chunk_size = has_next_chunk ? chunk_size : size - offset;
    fwrite(output + offset, this_chunk_size, 1, stdout);
    printf("\033\\");
    offset += chunk_size;
  }
  printf("\n");

  free(output);
}

static void kitty_format(GVJ_t *job) {
  unsigned char *imagedata = job->imagedata;
  size_t imagedata_size = job->width * job->height * BYTES_PER_PIXEL;
  argb2rgba(job->width, job->height, imagedata);

  kitty_write(imagedata, imagedata_size, job->width, job->height, false);
}

static gvdevice_features_t device_features_kitty = {
    GVDEVICE_DOES_TRUECOLOR, /* flags */
    {0., 0.},                /* default margin - points */
    {0., 0.},                /* default page width, height - points */
    {96., 96.},              /* dpi */
};

static gvdevice_engine_t device_engine_kitty = {.format = kitty_format};

#ifdef HAVE_LIBZ
static int zlib_compress(unsigned char *source, uLong source_len,
                         unsigned char **dest, size_t *dest_len) {
  uLong dest_cap = compressBound(source_len);
  *dest = gv_alloc(dest_cap);

  const int ret = compress(*dest, &dest_cap, source, source_len);
  *dest_len = dest_cap;
  return ret;
}

static void zkitty_format(GVJ_t *job) {
  unsigned char *imagedata = job->imagedata;
  const uLong imagedata_size = job->width * job->height * BYTES_PER_PIXEL;
  argb2rgba(job->width, job->height, imagedata);

  unsigned char *zbuf;
  size_t zsize;
  int ret = zlib_compress(imagedata, imagedata_size, &zbuf, &zsize);
  assert(ret == Z_OK);
  (void)ret;

  kitty_write(zbuf, zsize, job->width, job->height, true);

  free(zbuf);
}

static gvdevice_features_t device_features_zkitty = {
    GVDEVICE_DOES_TRUECOLOR, /* flags */
    {0., 0.},                /* default margin - points */
    {0., 0.},                /* default page width, height - points */
    {96., 96.},              /* dpi */
};

static gvdevice_engine_t device_engine_zkitty = {.format = zkitty_format};
#endif

gvplugin_installed_t gvdevice_types_kitty[] = {
    {0, "kitty:cairo", 0, &device_engine_kitty, &device_features_kitty},
#ifdef HAVE_LIBZ
    {1, "kittyz:cairo", 1, &device_engine_zkitty, &device_features_zkitty},
#endif
    {0, NULL, 0, NULL, NULL}};
