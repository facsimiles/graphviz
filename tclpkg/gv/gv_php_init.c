/*************************************************************************
 * Copyright (c) 2011 AT&T Intellectual Property 
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * https://www.eclipse.org/legal/epl-v10.html
 *
 * Contributors: Details at https://graphviz.org
 *************************************************************************/

#include <php.h>
#undef PACKAGE_BUGREPORT
#undef PACKAGE_NAME
#undef PACKAGE_STRING
#undef PACKAGE_TARNAME
#undef PACKAGE_VERSION

// PHP ship their own config.h, so suppress this
#ifdef HAVE_MEMRCHR
#undef HAVE_MEMRCHR
#endif
#ifdef HAVE_SETENV
#undef HAVE_SETENV
#endif
#ifdef HAVE_SYS_IOCTL_H
#undef HAVE_SYS_IOCTL_H
#endif
#ifdef HAVE_SYS_MMAN_H
#undef HAVE_SYS_MMAN_H
#endif
#ifdef HAVE_SYS_SELECT_H
#undef HAVE_SYS_SELECT_H
#endif
#ifdef HAVE_SYS_TIME_H
#undef HAVE_SYS_TIME_H
#endif

#include "config.h"

#include <gvc/gvc.h>
#include <gvc/gvplugin.h>
#include <gvc/gvcjob.h>
#include <gvc/gvcint.h>
#include "gv_channel.h"

static size_t gv_string_writer (GVJ_t *job, const char *s, size_t len)
{
    (void)job;
    return PHPWRITE(s, len);
}

static size_t gv_channel_writer (GVJ_t *job, const char *s, size_t len)
{
    (void)job;
    return PHPWRITE(s, len);
}

void gv_string_writer_init(GVC_t *gvc)
{
    gvc->write_fn = gv_string_writer;
}

void gv_channel_writer_init(GVC_t *gvc)
{
    gvc->write_fn = gv_channel_writer;
}

void gv_writer_reset (GVC_t *gvc) {gvc->write_fn = NULL;}
