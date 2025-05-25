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

#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <util/gv_math.h>

#include <gvc/gvplugin_loadimage.h>
#include "gvplugin_gdiplus.h"
#include <stringapiset.h>
#include <windows.h>
#include <gdiplus.h>
#include <vector>

using namespace Gdiplus;

static void gdiplus_freeimage(usershape_t *us) {
  delete reinterpret_cast<Image*>(us->data);
}

// convert a UTF-8 string to UTF-16
static std::vector<wchar_t> utf8_to_utf16(const char *s) {

  // how much space do we need for the UTF-16 string?
  const int wide_count = MultiByteToWideChar(CP_UTF8, 0, s, -1, nullptr, 0);

  // translate it
  std::vector<wchar_t> utf16(wide_count);
  if (wide_count > 0) {
    (void)MultiByteToWideChar(CP_UTF8, 0, s, -1, utf16.data(), wide_count);
  } else {
    utf16.push_back(0);
  }

  return utf16;
}

static Image *gdiplus_loadimage(usershape_t *us) {
    assert(us);
    assert(us->name);

    if (us->data && us->datafree != gdiplus_freeimage) {
	     us->datafree(us);        /* free incompatible cache data */
	     us->data = nullptr;
	     us->datafree = nullptr;
	}
    
    if (!us->data) { /* read file into cache */
		if (!gvusershape_file_access(us))
			return nullptr;

		/* create image from the usershape file */
		const std::vector<wchar_t> filename = utf8_to_utf16(us->name);
		us->data = Image::FromFile(filename.data());
		
		/* clean up */
		if (us->data)
			us->datafree = gdiplus_freeimage;
			
		gvusershape_file_release(us);
    }
    return reinterpret_cast<Image*>(us->data);
}

static void gdiplus_loadimage_gdiplus(GVJ_t * job, usershape_t *us, boxf b, bool)
{
	/* get the image from usershape details, then blit it to the context */
	if (Image *image = gdiplus_loadimage(us)) {
		assert(job != NULL);
		auto g = reinterpret_cast<Graphics*>(job->context);
		g->DrawImage(image, RectF(d2f(b.LL.x), d2f(b.LL.y), d2f(b.UR.x - b.LL.x),
		                          d2f(b.UR.y - b.LL.y)));
	}
}

static gvloadimage_engine_t engine = {
    gdiplus_loadimage_gdiplus
};

gvplugin_installed_t gvloadimage_gdiplus_types[] = {
	{FORMAT_BMP, "bmp:gdiplus", 8, &engine, nullptr},
	{FORMAT_GIF, "gif:gdiplus", 8, &engine, nullptr},
	{FORMAT_JPEG, "jpe:gdiplus", 8, &engine, nullptr},
	{FORMAT_JPEG, "jpeg:gdiplus", 8, &engine, nullptr},
	{FORMAT_JPEG, "jpg:gdiplus", 8, &engine, nullptr},
	{FORMAT_PNG, "png:gdiplus", 8, &engine, nullptr},
	{0, nullptr, 0, nullptr, nullptr}
};
