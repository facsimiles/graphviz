## Process this file with automake to produce Makefile.in

SUBDIRS = demo

pkgtcldir = $(pkglibdir)/tcl

AM_CPPFLAGS = \
	-I$(top_srcdir) \
	-I$(top_srcdir)/lib \
	$(GDLIB_CFLAGS) $(TCL_INCLUDES)

LIBS = @LIBS@ -lc

AM_CFLAGS = $(TCL_CFLAGS)

man = gdtclft.3tcl

if WITH_LIBGD
if WITH_TCL
pkgtcl_LTLIBRARIES = libgdtclft.la
man_MANS = $(man)
if ENABLE_MAN_PDFS
pdf_DATA = gdtclft.3tcl.pdf
endif
noinst_LTLIBRARIES = libgdtclft_C.la
endif
endif

libgdtclft_la_SOURCES = gdtclft.c
libgdtclft_C_la_SOURCES = gdtclft.c

libgdtclft_la_LDFLAGS = -no-undefined -module -avoid-version

if WITH_LIBGD
libgdtclft_la_LIBADD = \
	$(top_builddir)/tclpkg/tclstubs/libtclstubs_C.la \
	$(GDLIB_LIBS)

if WITH_TCL
BUILT_SOURCES = pkgIndex.tcl
all-local: pkgIndex.tcl

pkgIndex.tcl:
	$(PYTHON3) $(top_srcdir)/tclpkg/mkpkgindex.py \
	  --file libgdtclft$(SHARED_LIBRARY_SUFFIX) \
	  --name Gdtclft \
	  --version $(VERSION)

test:
	echo 'load .libs/libgdtclft$(TCL_SHLIB_SUFFIX) Gdtclft ; cd tests ; source all' | tclsh8.3
endif
endif

.3tcl.3tcl.pdf:
	rm -f $@; pdffile=$@; psfile=$${pdffile%pdf}ps; \
	$(GROFF) -Tps -man $< > $$psfile || { rm -f $$psfile; exit 1; }; \
	$(PS2PDF) $$psfile && rm -f $$psfile || { rm -f $$psfile; exit 1; }
SUFFIXES = .3tcl .3tcl.pdf

EXTRA_DIST = $(man)

DISTCLEANFILES = $(pdf_DATA) pkgIndex.tcl
