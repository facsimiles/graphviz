## Process this file with automake to produce Makefile.in

AM_CPPFLAGS = \
	-I$(top_srcdir)/lib \
	-I$(top_srcdir)/lib/common \
	-I$(top_srcdir)/lib/pathplan \
	-I$(top_srcdir)/lib/gvc \
	-I$(top_srcdir)/lib/cgraph \
	-I$(top_srcdir)/lib/cdt \
	$(LIBGVC_CFLAGS)

noinst_LTLIBRARIES = libgvplugin_dot_layout_C.la

if WITH_WIN32
lib_LTLIBRARIES = libgvplugin_dot_layout.la
else
pkglib_LTLIBRARIES = libgvplugin_dot_layout.la
endif

libgvplugin_dot_layout_C_la_SOURCES = \
	gvplugin_dot_layout.c \
	gvlayout_dot_layout.c
libgvplugin_dot_layout_C_la_LIBADD = \
	$(top_builddir)/lib/dotgen/libdotgen_C.la

libgvplugin_dot_layout_la_LDFLAGS = -version-info $(GVPLUGIN_VERSION_INFO)
libgvplugin_dot_layout_la_SOURCES = $(libgvplugin_dot_layout_C_la_SOURCES)
libgvplugin_dot_layout_la_LIBADD = $(libgvplugin_dot_layout_C_la_LIBADD) \
	$(top_builddir)/lib/gvc/libgvc.la \
	$(top_builddir)/lib/pathplan/libpathplan.la \
	$(top_builddir)/lib/cgraph/libcgraph.la \
	$(top_builddir)/lib/cdt/libcdt.la \
	$(MATH_LIBS)

if WITH_WIN32
libgvplugin_dot_layout_la_LDFLAGS += -no-undefined
endif
