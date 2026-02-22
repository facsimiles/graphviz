## Process this file with automake to produce Makefile.in

AM_CPPFLAGS = \
	-I$(top_srcdir)/lib \
	-I$(top_srcdir)/lib/cdt

bin_PROGRAMS = prune
dist_man_MANS = prune.1
if ENABLE_MAN_PDFS
pdf_DATA = prune.1.pdf
endif

prune_SOURCES = prune.c

prune_LDADD = \
	$(top_builddir)/lib/cgraph/libcgraph.la \
	$(top_builddir)/lib/cdt/libcdt.la \
	$(top_builddir)/lib/util/libutil_C.la

if WITH_THREADS_CXX
# add a non-existent C++ source to force the C++ compiler to be used for
# linking, so the C++ standard library is included for our C++ dependencies
nodist_EXTRA_prune_SOURCES = fake.cxx
endif

.1.1.pdf:
	rm -f $@; pdffile=$@; psfile=$${pdffile%pdf}ps; \
	$(GROFF) -Tps -man $< > $$psfile || { rm -f $$psfile; exit 1; }; \
	$(PS2PDF) $$psfile && rm -f $$psfile || { rm -f $$psfile; exit 1; }
SUFFIXES = .1 .1.pdf

EXTRA_DIST = README test.gv

DISTCLEANFILES = $(pdf_DATA)
