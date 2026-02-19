## Process this file with automake to produce Makefile.in

SUBDIRS = demo

if WITH_LIBGD
GDTCLFT_LIBS = $(top_builddir)/tclpkg/gdtclft/libgdtclft_C.la
endif

pkgtcldir = $(pkglibdir)/tcl

AM_CPPFLAGS = \
	-I$(top_srcdir)/lib \
	-I$(top_srcdir)/lib/gvc \
	-I$(top_srcdir)/lib/common \
	-I$(top_srcdir)/lib/cgraph \
	-I$(top_srcdir)/lib/cdt \
	-I$(top_srcdir)/lib/pathplan $(GDLIB_CFLAGS) $(TCL_INCLUDES)

LIBS = -lc $(TCL_STUB_LIB_SPEC)

AM_CFLAGS = $(TCL_CFLAGS)

man = tcldot.3tcl

if WITH_TCL
noinst_HEADERS = tcldot.h
pkgtcl_LTLIBRARIES = libtcldot.la libtcldot_builtin.la

man_MANS = $(man)
if ENABLE_MAN_PDFS
pdf_DATA = tcldot.3tcl.pdf
endif
dist_html_DATA = tcldot.html
endif

libtcldot_la_SOURCES = \
	tcldot.c \
	tcldot-graphcmd.c \
	tcldot-nodecmd.c \
	tcldot-edgecmd.c \
	tcldot-util.c \
	tcldot-id.c \
	tcldot-io.c \
	no_builtins.c $(GDTCLFT)
libtcldot_la_CPPFLAGS = $(AM_CPPFLAGS) -DDEMAND_LOADING=1
libtcldot_la_LDFLAGS = -no-undefined -module -avoid-version

libtcldot_la_LIBADD = $(GDTCLFT_LIBS) \
	$(top_builddir)/tclpkg/tclstubs/libtclstubs_C.la \
	$(top_builddir)/lib/gvc/libgvc.la \
	$(top_builddir)/lib/pathplan/libpathplan_C.la \
	$(top_builddir)/lib/cgraph/libcgraph.la \
	$(top_builddir)/lib/cdt/libcdt.la \
	$(top_builddir)/lib/util/libutil_C.la

# GDTCLFT requires gd libs to be builtin
if WITH_LIBGD
libtcldot_la_LIBADD += $(GDLIB_LIBS)
endif

libtcldot_la_LIBADD += $(MATH_LIBS)

libtcldot_builtin_la_SOURCES = \
	tcldot.c \
	tcldot-graphcmd.c \
	tcldot-nodecmd.c \
	tcldot-edgecmd.c \
	tcldot-util.c \
	tcldot-id.c \
	tcldot-io.c \
	tcldot_builtins.cpp $(GDTCLFT)
libtcldot_builtin_la_CPPFLAGS = $(AM_CPPFLAGS) -DDEMAND_LOADING=1
libtcldot_builtin_la_LDFLAGS = -no-undefined -module -avoid-version

libtcldot_builtin_la_LIBADD = $(GDTCLFT_LIBS) \
	$(top_builddir)/plugin/dot_layout/libgvplugin_dot_layout_C.la \
	$(top_builddir)/plugin/neato_layout/libgvplugin_neato_layout_C.la \
	$(top_builddir)/plugin/core/libgvplugin_core_C.la \
	$(top_builddir)/tclpkg/tclstubs/libtclstubs_C.la \
	$(top_builddir)/lib/gvc/libgvc_C.la \
	$(top_builddir)/lib/pathplan/libpathplan_C.la \
	$(top_builddir)/lib/cgraph/libcgraph_C.la \
	$(top_builddir)/lib/cdt/libcdt_C.la \
	$(GTS_LIBS)

if WITH_PANGOCAIRO
libtcldot_builtin_la_LIBADD += $(top_builddir)/plugin/pango/libgvplugin_pango_C.la $(PANGOCAIRO_LIBS) $(PANGOFT2_LIBS)
endif

if WITH_LIBGD
libtcldot_builtin_la_LIBADD += $(top_builddir)/plugin/gd/libgvplugin_gd_C.la
libtcldot_builtin_la_LIBADD += $(GDLIB_LIBS)
endif

libtcldot_builtin_la_LIBADD += $(EXPAT_LIBS) $(IPSEPCOLA_LIBS) $(MATH_LIBS)

if WITH_TCL
BUILT_SOURCES = pkgIndex.tcl
all-local: pkgIndex.tcl

pkgIndex.tcl:
	$(PYTHON3) $(top_srcdir)/tclpkg/mkpkgindex.py \
	  --file libtcldot$(SHARED_LIBRARY_SUFFIX) \
	  --name Tcldot \
	  --version $(VERSION)
endif

.3tcl.3tcl.pdf:
	rm -f $@; pdffile=$@; psfile=$${pdffile%pdf}ps; \
	$(GROFF) -Tps -man $< > $$psfile || { rm -f $$psfile; exit 1; }; \
	$(PS2PDF) $$psfile && rm -f $$psfile || { rm -f $$psfile; exit 1; }
SUFFIXES = .3tcl .3tcl.pdf

EXTRA_DIST = $(man) README README.Tkspline

DISTCLEANFILES = $(pdf_DATA) pkgIndex.tcl
