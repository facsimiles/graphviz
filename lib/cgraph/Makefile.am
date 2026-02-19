## Process this file with automake to produce Makefile.in

CGRAPH_VERSION = "8:6:0"

AM_CPPFLAGS = -I$(top_srcdir)/lib -I$(top_srcdir)/lib/cdt

if WITH_WIN32
AM_CFLAGS = -DEXPORT_CGRAPH -DEXPORT_CGHDR
endif

pkginclude_HEADERS = cgraph.h
noinst_HEADERS = agstrcanon.h cghdr.h ingraphs.h node_set.h rdr.h
noinst_LTLIBRARIES = libcgraph_C.la
lib_LTLIBRARIES = libcgraph.la
pkgconfig_DATA = libcgraph.pc
dist_man_MANS = cgraph.3
if ENABLE_MAN_PDFS
pdf_DATA = cgraph.3.pdf
endif

libcgraph_C_la_SOURCES = acyclic.c agerror.c apply.c attr.c edge.c \
	graph.c grammar.y id.c imap.c ingraphs.c io.c node.c node_induce.c \
	obj.c rec.c refstr.c scan.l subg.c tred.c unflatten.c utils.c write.c

libcgraph_la_LDFLAGS = -version-info $(CGRAPH_VERSION) -no-undefined
libcgraph_la_SOURCES = $(libcgraph_C_la_SOURCES)
libcgraph_la_LIBADD = \
  $(top_builddir)/lib/cdt/libcdt.la \
  $(top_builddir)/lib/util/libutil_C.la

scan.$(OBJEXT) scan.lo: scan.c grammar.h

scan.c: $(top_srcdir)/lib/cgraph/scan.l
	$(LEX) --case-insensitive --outfile=$@ $(top_srcdir)/lib/cgraph/scan.l

grammar.c grammar.h: $(top_srcdir)/lib/cgraph/grammar.y
	$(YACC) -Wno-yacc -dv --output=grammar.c $(top_srcdir)/lib/cgraph/grammar.y

.3.3.pdf:
	rm -f $@; pdffile=$@; psfile=$${pdffile%pdf}ps; \
	$(GROFF) -Tps -man $< > $$psfile || { rm -f $$psfile; exit 1; }; \
	$(PS2PDF) $$psfile && rm -f $$psfile || { rm -f $$psfile; exit 1; }
SUFFIXES = .3 .3.pdf

EXTRA_DIST = \
	grammar.c grammar.h scan.c

DISTCLEANFILES = grammar.[ch] scan.c $(pdf_DATA)
