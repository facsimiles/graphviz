## Process this file with automake to produce Makefile.in

AM_CPPFLAGS = \
	-I$(top_srcdir)/lib \
	-I$(top_srcdir)/lib/cdt \
	-I$(top_srcdir)/lib/cgraph \
	-I$(top_srcdir)/lib/common \
	-I$(top_srcdir)/lib/gvc \
	-I$(top_srcdir)/lib/pathplan \
	$(Z_INCLUDES)

noinst_LTLIBRARIES = libgvplugin_kitty_C.la
if WITH_WIN32
lib_LTLIBRARIES = libgvplugin_kitty.la
else
pkglib_LTLIBRARIES = libgvplugin_kitty.la
endif

libgvplugin_kitty_C_la_SOURCES = \
	gvplugin_kitty.c \
	gvdevice_kitty.c

libgvplugin_kitty_la_LDFLAGS = -version-info $(GVPLUGIN_VERSION_INFO)
libgvplugin_kitty_la_SOURCES = $(libgvplugin_kitty_C_la_SOURCES)
libgvplugin_kitty_la_LIBADD = \
	$(top_builddir)/lib/util/libutil_C.la \
	$(Z_LIBS)

if WITH_WIN32
libgvplugin_kitty_la_LDFLAGS += -no-undefined
endif
