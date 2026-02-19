## Process this file with automake to produce Makefile.in

SUBDIRS = demo

pkgtcldir = $(pkglibdir)/tcl

noinst_HEADERS = makecw.h Plegal_arrangement.h simple.h
if WITH_TCL
pkgtcl_LTLIBRARIES = libtclplan.la
endif

libtclplan_la_SOURCES = find_ints.c intersect.c makecw.c tclpathplan.c wrapper.c
libtclplan_la_CPPFLAGS = \
	-I$(top_srcdir)/lib \
	-I$(top_srcdir)/lib/pathplan $(TCL_INCLUDES)
libtclplan_la_CFLAGS = $(TCL_CFLAGS)
libtclplan_la_LDFLAGS = -no-undefined -module -avoid-version
libtclplan_la_LIBADD = \
	$(top_builddir)/tclpkg/tclstubs/libtclstubs_C.la \
	$(top_builddir)/lib/pathplan/libpathplan.la \
	$(top_builddir)/lib/util/libutil_C.la $(MATH_LIBS) \
	-lc $(TCL_STUB_LIB_SPEC)

if WITH_TCL
BUILT_SOURCES = pkgIndex.tcl
all-local: pkgIndex.tcl

pkgIndex.tcl:
	$(PYTHON3) $(top_srcdir)/tclpkg/mkpkgindex.py \
	  --file libtclplan$(SHARED_LIBRARY_SUFFIX) \
	  --name Tclpathplan \
	  --version $(VERSION)
endif

DISTCLEANFILES = pkgIndex.tcl

EXTRA_DIST = $(libtclplan_la_SOURCES)
