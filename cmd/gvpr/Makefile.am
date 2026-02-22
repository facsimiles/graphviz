## Process this file with automake to produce Makefile.in

SUBDIRS = lib

AM_CPPFLAGS = \
	-I$(top_srcdir)/lib \
	-I$(top_srcdir)/lib/gvpr \
	-I$(top_srcdir)/lib/cgraph \
	-I$(top_srcdir)/lib/cdt

if ENABLE_STATIC
bin_PROGRAMS = gvpr gvpr_static
else
bin_PROGRAMS = gvpr
endif

dist_man_MANS = gvpr.1
if ENABLE_MAN_PDFS
pdf_DATA = gvpr.1.pdf
endif

gvpr_SOURCES = gvprmain.c
gvpr_CPPFLAGS = $(AM_CPPFLAGS)
gvpr_LDADD = $(top_builddir)/lib/gvpr/libgvpr.la \
	$(top_builddir)/lib/cgraph/libcgraph.la \
	$(top_builddir)/lib/cdt/libcdt.la

if WITH_THREADS_CXX
# add a non-existent C++ source to force the C++ compiler to be used for
# linking, so the C++ standard library is included for our C++ dependencies
nodist_EXTRA_gvpr_SOURCES = fake.cxx
endif

.1.1.pdf:
	rm -f $@; pdffile=$@; psfile=$${pdffile%pdf}ps; \
	$(GROFF) -Tps -man $< > $$psfile || { rm -f $$psfile; exit 1; }; \
	$(PS2PDF) $$psfile && rm -f $$psfile || { rm -f $$psfile; exit 1; }
SUFFIXES = .1 .1.pdf

gvpr_static_SOURCES = gvprmain.c
gvpr_static_CPPFLAGS = $(AM_CPPFLAGS)
gvpr_static_LDADD = \
	$(top_builddir)/lib/gvpr/libgvpr_C.la \
	$(top_builddir)/lib/expr/libexpr_C.la \
	$(top_builddir)/lib/ast/libast_C.la \
	$(top_builddir)/lib/sfio/libsfio_C.la \
	$(top_builddir)/lib/sfio/Sfio_f/libsfiof_C.la \
	$(top_builddir)/lib/cgraph/libcgraph_C.la \
	$(top_builddir)/lib/cdt/libcdt_C.la \
	$(top_builddir)/lib/util/libutil_C.la \
	$(MATH_LIBS)

EXTRA_DIST = lib

DISTCLEANFILES = $(pdf_DATA)
