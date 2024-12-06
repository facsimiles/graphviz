## Process this file with automake to produce Makefile.in

GVPR_VERSION = "2:4:0"

AM_CPPFLAGS = \
	-I$(top_srcdir)/lib \
	-I$(top_builddir)/lib \
	-I$(top_srcdir)/lib/common \
	-I$(top_builddir)/lib/common \
	-I$(top_srcdir)/lib/cgraph \
	-I$(top_srcdir)/lib/cdt

pkginclude_HEADERS = gvpr.h
noinst_HEADERS = actions.h compile.h gdefs.h gprdata.inc gprstate.h parse.h \
	trieFA.h trie.c
noinst_LTLIBRARIES = libgvpr_C.la
lib_LTLIBRARIES = libgvpr.la
pkgconfig_DATA = libgvpr.pc
dist_man_MANS = gvpr.3
if ENABLE_MAN_PDFS
pdf_DATA = gvpr.3.pdf
endif

libgvpr_C_la_SOURCES = actions.c compile.c gvpr.c gprstate.c parse.c
libgvpr_C_la_DEPENDENCIES = \
	$(top_builddir)/lib/expr/libexpr_C.la \
	$(top_builddir)/lib/common/libcommon_C.la

.3.3.pdf:
	rm -f $@; pdffile=$@; psfile=$${pdffile%pdf}ps; \
	$(GROFF) -Tps -man $< > $$psfile || { rm -f $$psfile; exit 1; }; \
	$(PS2PDF) $$psfile && rm -f $$psfile || { rm -f $$psfile; exit 1; }
SUFFIXES = .3 .3.pdf

libgvpr_la_LDFLAGS = -version-info $(GVPR_VERSION) -no-undefined
libgvpr_la_SOURCES = $(libgvpr_C_la_SOURCES)
libgvpr_la_LIBADD = $(libgvpr_C_la_LIBADD) \
	$(top_builddir)/lib/expr/libexpr_C.la \
	$(top_builddir)/lib/cgraph/libcgraph.la \
	$(top_builddir)/lib/cdt/libcdt.la \
	$(MATH_LIBS)
EXTRA_libgvpr_la_DEPENDENCIES = $(libgvpr_C_la_DEPENDENCIES)

if WITH_WIN32
libgvpr_la_LDFLAGS += -export-symbols-regex gvpr
endif

EXTRA_DIST = trie.c

DISTCLEANFILES = $(pdf_DATA)
