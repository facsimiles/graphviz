## Process this file with automake to produce Makefile.in

AM_CPPFLAGS = \
	-I$(top_srcdir)/lib \
	-I$(top_srcdir)/lib/common \
	-I$(top_srcdir)/lib/pathplan \
	-I$(top_srcdir)/lib/gvc \
	-I$(top_srcdir)/lib/cgraph \
	-I$(top_srcdir)/lib/cdt \
	$(PANGOCAIRO_CFLAGS) $(GDLIB_CFLAGS)

if WITH_LIBGD
noinst_LTLIBRARIES = libgvplugin_gd_C.la

if WITH_WIN32
lib_LTLIBRARIES = libgvplugin_gd.la
else
pkglib_LTLIBRARIES = libgvplugin_gd.la
endif
endif

noinst_HEADERS = gdioctx_wrapper.h gdgen_text.h gd_psfontResolve.h
libgvplugin_gd_C_la_SOURCES = \
	gvplugin_gd.c \
	gvrender_gd.c \
	gvrender_gd_vrml.c \
	gvtextlayout_gd.c \
	gvloadimage_gd.c \
	gvdevice_gd.c

libgvplugin_gd_la_LDFLAGS = -version-info $(GVPLUGIN_VERSION_INFO)
libgvplugin_gd_la_SOURCES = $(libgvplugin_gd_C_la_SOURCES)

if WITH_LIBGD
libgvplugin_gd_la_LIBADD = \
	$(top_builddir)/lib/gvc/libgvc.la \
	$(top_builddir)/lib/cgraph/libcgraph.la \
	$(top_builddir)/lib/cdt/libcdt.la $(PANGOCAIRO_LIBS) $(GDLIB_LIBS) $(MATH_LIBS)

if WITH_WIN32
libgvplugin_gd_la_LDFLAGS += -no-undefined
libgvplugin_gd_la_LIBADD += $(top_builddir)/lib/pathplan/libpathplan.la
endif
endif
