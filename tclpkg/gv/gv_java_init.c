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

#include <stddef.h>
#include <gvc/gvc.h>
#include <gvc/gvplugin.h>
#include <gvc/gvcjob.h>
#include <gvc/gvcint.h>
#include "gv_channel.h"
#include <util/agxbuf.h>

static size_t gv_string_writer(GVJ_t *job, const char *s, size_t len)
{
    agxbuf *const buffer = (agxbuf *)job->output_file;
    return agxbput_n(buffer, s, len);
}

void gv_string_writer_init(GVC_t *gvc)
{
    gvc->write_fn = gv_string_writer;
}

static size_t gv_channel_writer(GVJ_t *job, const char *s, size_t len)
{
    (void)job;
    (void)s;
    return len;
}

void gv_channel_writer_init(GVC_t *gvc)
{
    gvc->write_fn = gv_channel_writer;
}

void gv_writer_reset (GVC_t *gvc) {gvc->write_fn = NULL;}
