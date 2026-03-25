## Process this file with automake to produce Makefile.in

AM_CPPFLAGS = \
	-I$(top_srcdir)/lib \
	-I$(top_srcdir)/lib/cgraph \
	-I$(top_srcdir)/lib/cdt \
	-I$(top_srcdir)/lib/pathplan \
	-I$(top_srcdir)/lib/pack \
	-I$(top_srcdir)/lib/gvc \
	-I$(top_srcdir)/lib/common

noinst_HEADERS = colortbl.h colorxlate.h convert.h mmio.h matrix_market.h \
	graph_generator.h gml2gv.h gmlparse.h openFile.h
if ENABLE_STATIC
bin_PROGRAMS = gc gvcolor gxl2gv acyclic nop ccomps sccmap tred \
	unflatten gvpack gvpack_static dijkstra bcomps mm2gv gvgen gml2gv gv2gml graphml2gv
else
bin_PROGRAMS = gc gvcolor gxl2gv acyclic nop ccomps sccmap tred \
	unflatten gvpack dijkstra bcomps mm2gv gvgen gml2gv gv2gml graphml2gv
endif

dist_man_MANS = gc.1 gvcolor.1 gxl2gv.1 acyclic.1 nop.1 ccomps.1 sccmap.1 \
	tred.1 unflatten.1 gvpack.1 dijkstra.1 bcomps.1 mm2gv.1 gvgen.1 gml2gv.1 graphml2gv.1
if ENABLE_MAN_PDFS
pdf_DATA = gc.1.pdf gvcolor.1.pdf gxl2gv.1.pdf acyclic.1.pdf \
	nop.1.pdf ccomps.1.pdf sccmap.1.pdf tred.1.pdf \
	unflatten.1.pdf gvpack.1.pdf dijkstra.1.pdf \
	bcomps.1.pdf mm2gv.1.pdf gvgen.1.pdf gml2gv.1.pdf graphml2gv.1.pdf
endif

install-data-hook:
	(cd $(DESTDIR)$(man1dir); rm -f gv2gxl.1; $(LN_S) gxl2gv.1 gv2gxl.1;)
if ENABLE_MAN_PDFS
	(cd $(DESTDIR)$(pdfdir); rm -f gv2gxl.1.pdf; $(LN_S) gxl2gv.1.pdf gv2gxl.1.pdf;)
endif
	(cd $(DESTDIR)$(man1dir); rm -f gxl2dot.1; $(LN_S) gxl2gv.1 gxl2dot.1;)
if ENABLE_MAN_PDFS
	(cd $(DESTDIR)$(pdfdir); rm -f gxl2dot.1.pdf; $(LN_S) gxl2gv.1.pdf gxl2dot.1.pdf;)
endif
	(cd $(DESTDIR)$(man1dir); rm -f dot2gxl.1; $(LN_S) gxl2gv.1 dot2gxl.1;)
if ENABLE_MAN_PDFS
	(cd $(DESTDIR)$(pdfdir); rm -f dot2gxl.1.pdf; $(LN_S) gxl2gv.1.pdf dot2gxl.1.pdf;)
endif
	(cd $(DESTDIR)$(man1dir); rm -f gv2gml.1; $(LN_S) gml2gv.1 gv2gml.1;)
if ENABLE_MAN_PDFS
	(cd $(DESTDIR)$(pdfdir); rm -f gv2gml.1.pdf; $(LN_S) gml2gv.1.pdf gv2gml.1.pdf;)
endif

install-exec-hook:
	(cd $(DESTDIR)$(bindir); rm -f gv2gxl$(EXEEXT); $(LN_S) gxl2gv$(EXEEXT) gv2gxl$(EXEEXT);)
	(cd $(DESTDIR)$(bindir); rm -f dot2gxl$(EXEEXT); $(LN_S) gxl2gv$(EXEEXT) dot2gxl$(EXEEXT);)
	(cd $(DESTDIR)$(bindir); rm -f gxl2dot$(EXEEXT); $(LN_S) gxl2gv$(EXEEXT) gxl2dot$(EXEEXT);)

uninstall-hook:
	(cd $(DESTDIR)$(man1dir); rm -f gv2gxl.1;)
if ENABLE_MAN_PDFS
	(cd $(DESTDIR)$(pdfdir); rm -f gv2gxl.1.pdf;)
endif
	(cd $(DESTDIR)$(bindir); rm -f gv2gxl$(EXEEXT);)
	(cd $(DESTDIR)$(bindir); rm -f dot2gxl$(EXEEXT);)
	(cd $(DESTDIR)$(bindir); rm -f gxl2dot$(EXEEXT);)

gxl2gv_SOURCES = cvtgxl.c gv2gxl.c gxl2gv.c
gxl2gv_CPPFLAGS = $(AM_CPPFLAGS) $(EXPAT_CFLAGS)

gxl2gv_LDADD = \
	$(top_builddir)/lib/gvc/libgvc.la \
	$(top_builddir)/lib/cgraph/libcgraph.la \
	$(top_builddir)/lib/cdt/libcdt.la \
	$(top_builddir)/lib/util/libutil_C.la $(EXPAT_LIBS)

if WITH_THREADS_CXX
# add a non-existent C++ source to force the C++ compiler to be used for
# linking, so the C++ standard library is included for our C++ dependencies
nodist_EXTRA_gxl2gv_SOURCES = fake.cxx
endif

.1.1.pdf:
	rm -f $@; pdffile=$@; psfile=$${pdffile%pdf}ps; \
	$(GROFF) -Tps -man $< > $$psfile || { rm -f $$psfile; exit 1; }; \
	$(PS2PDF) $$psfile && rm -f $$psfile || { rm -f $$psfile; exit 1; }
SUFFIXES = .1 .1.pdf

sccmap_SOURCES = sccmap.c

sccmap_LDADD = \
	$(top_builddir)/lib/cgraph/libcgraph.la \
	$(top_builddir)/lib/util/libutil_C.la $(EXPAT_LIBS)

if WITH_THREADS_CXX
# add a non-existent C++ source to force the C++ compiler to be used for
# linking, so the C++ standard library is included for our C++ dependencies
nodist_EXTRA_sccmap_SOURCES = fake.cxx
endif

ccomps_SOURCES = ccomps.c

ccomps_LDADD = \
	$(top_builddir)/lib/common/libcommon_C.la \
	$(top_builddir)/lib/gvc/libgvc.la \
	$(top_builddir)/lib/pathplan/libpathplan.la \
	$(top_builddir)/lib/cgraph/libcgraph.la \
	$(top_builddir)/lib/cdt/libcdt.la \
	$(top_builddir)/lib/util/libutil_C.la \
	$(MATH_LIBS)

if WITH_THREADS_CXX
# add a non-existent C++ source to force the C++ compiler to be used for
# linking, so the C++ standard library is included for our C++ dependencies
nodist_EXTRA_ccomps_SOURCES = fake.cxx
endif

tred_SOURCES = tred.c

tred_LDADD = \
	$(top_builddir)/lib/gvc/libgvc.la \
	$(top_builddir)/lib/cgraph/libcgraph.la

if WITH_THREADS_CXX
# add a non-existent C++ source to force the C++ compiler to be used for
# linking, so the C++ standard library is included for our C++ dependencies
nodist_EXTRA_tred_SOURCES = fake.cxx
endif

unflatten_SOURCES = unflatten.c

unflatten_LDADD = \
	$(top_builddir)/lib/cgraph/libcgraph.la

if WITH_THREADS_CXX
# add a non-existent C++ source to force the C++ compiler to be used for
# linking, so the C++ standard library is included for our C++ dependencies
nodist_EXTRA_unflatten_SOURCES = fake.cxx
endif

acyclic_SOURCES = acyclic.c

acyclic_LDADD = \
	$(top_builddir)/lib/cgraph/libcgraph.la

if WITH_THREADS_CXX
# add a non-existent C++ source to force the C++ compiler to be used for
# linking, so the C++ standard library is included for our C++ dependencies
nodist_EXTRA_acyclic_SOURCES = fake.cxx
endif

nop_SOURCES = nop.c

nop_LDADD = \
	$(top_builddir)/lib/cgraph/libcgraph.la

if WITH_THREADS_CXX
# add a non-existent C++ source to force the C++ compiler to be used for
# linking, so the C++ standard library is included for our C++ dependencies
nodist_EXTRA_nop_SOURCES = fake.cxx
endif

gvcolor_SOURCES = gvcolor.c colxlate.c colortbl.h colorxlate.h

gvcolor_LDADD = \
	$(top_builddir)/lib/cgraph/libcgraph.la \
	$(top_builddir)/lib/util/libutil_C.la \
	$(MATH_LIBS)

if WITH_THREADS_CXX
# add a non-existent C++ source to force the C++ compiler to be used for
# linking, so the C++ standard library is included for our C++ dependencies
nodist_EXTRA_gvcolor_SOURCES = fake.cxx
endif

bcomps_SOURCES = bcomps.c

bcomps_LDADD = \
	$(top_builddir)/lib/cgraph/libcgraph.la \
	$(top_builddir)/lib/util/libutil_C.la

if WITH_THREADS_CXX
# add a non-existent C++ source to force the C++ compiler to be used for
# linking, so the C++ standard library is included for our C++ dependencies
nodist_EXTRA_bcomps_SOURCES = fake.cxx
endif

gc_SOURCES = gc.c

gc_LDADD = \
	$(top_builddir)/lib/common/libcommon_C.la \
	$(top_builddir)/lib/gvc/libgvc.la \
	$(top_builddir)/lib/pathplan/libpathplan.la \
	$(top_builddir)/lib/cgraph/libcgraph.la \
	$(top_builddir)/lib/cdt/libcdt.la \
	$(top_builddir)/lib/util/libutil_C.la \
	$(MATH_LIBS)

if WITH_THREADS_CXX
# add a non-existent C++ source to force the C++ compiler to be used for
# linking, so the C++ standard library is included for our C++ dependencies
nodist_EXTRA_gc_SOURCES = fake.cxx
endif

gvpack_SOURCES = gvpack.cpp
gvpack_CPPFLAGS = $(AM_CPPFLAGS) -DDEMAND_LOADING=0

gvpack_LDADD = \
	$(top_builddir)/plugin/neato_layout/libgvplugin_neato_layout.la \
	$(top_builddir)/lib/gvc/libgvc.la \
	$(top_builddir)/lib/cgraph/libcgraph.la \
	$(top_builddir)/lib/cdt/libcdt.la

if ENABLE_STATIC
gvpack_static_SOURCES = gvpack.cpp
gvpack_static_CPPFLAGS = $(AM_CPPFLAGS) -DDEMAND_LOADING=0
gvpack_static_LDADD = \
	$(top_builddir)/plugin/neato_layout/libgvplugin_neato_layout_C.la \
	$(top_builddir)/lib/gvc/libgvc_C.la \
	$(top_builddir)/lib/pathplan/libpathplan_C.la \
	$(top_builddir)/lib/cgraph/libcgraph_C.la \
	$(top_builddir)/lib/cdt/libcdt_C.la \
	$(top_builddir)/lib/util/libutil_C.la \
	$(EXPAT_LIBS) $(Z_LIBS) $(GTS_LIBS) $(IPSEPCOLA_LIBS) $(MATH_LIBS)
endif


mm2gv_SOURCES = mm2gv.c matrix_market.c mmio.c

mm2gv_LDADD = \
	$(top_builddir)/lib/sparse/libsparse_C.la \
	$(top_builddir)/lib/common/libcommon_C.la \
	$(top_builddir)/lib/gvc/libgvc_C.la \
	$(top_builddir)/lib/pathplan/libpathplan_C.la \
	$(top_builddir)/lib/cgraph/libcgraph.la \
	$(top_builddir)/lib/util/libutil_C.la \
	$(MATH_LIBS)

if WITH_THREADS_CXX
# add a non-existent C++ source to force the C++ compiler to be used for
# linking, so the C++ standard library is included for our C++ dependencies
nodist_EXTRA_mm2gv_SOURCES = fake.cxx
endif

gv2gml_SOURCES = gv2gml.c

gv2gml_LDADD = \
	$(top_builddir)/lib/gvc/libgvc.la \
	$(top_builddir)/lib/cgraph/libcgraph.la \
	$(top_builddir)/lib/util/libutil_C.la

if WITH_THREADS_CXX
# add a non-existent C++ source to force the C++ compiler to be used for
# linking, so the C++ standard library is included for our C++ dependencies
nodist_EXTRA_gv2gml_SOURCES = fake.cxx
endif

gml2gv_SOURCES = gml2gv.c gmlparse.y gmlscan.l

gmlparse.c gmlparse.h: $(top_srcdir)/cmd/tools/gmlparse.y
	$(YACC) -Wno-yacc -dv $(top_srcdir)/cmd/tools/gmlparse.y -o gmlparse.c

gmlparse.$(OBJEXT) gmlparse.lo gmlscan.$(OBJEXT) gmlscan.lo: \
  gmlscan.c gmlparse.h

gmlscan.c: $(top_srcdir)/cmd/tools/gmlscan.l
	$(LEX) --case-insensitive --outfile=$@ $(top_srcdir)/cmd/tools/gmlscan.l

gml2gv_LDADD = \
	$(top_builddir)/lib/cgraph/libcgraph.la \
	$(top_builddir)/lib/cdt/libcdt.la \
	$(top_builddir)/lib/util/libutil_C.la $(MATH_LIBS)

if WITH_THREADS_CXX
# add a non-existent C++ source to force the C++ compiler to be used for
# linking, so the C++ standard library is included for our C++ dependencies
nodist_EXTRA_gml2gv_SOURCES = fake.cxx
endif

graphml2gv_SOURCES = graphml2gv.c
graphml2gv_CPPFLAGS = $(AM_CPPFLAGS) $(EXPAT_CFLAGS)

graphml2gv_LDADD = \
	$(top_builddir)/lib/cgraph/libcgraph.la \
	$(top_builddir)/lib/cdt/libcdt.la \
	$(top_builddir)/lib/util/libutil_C.la $(EXPAT_LIBS)

if WITH_THREADS_CXX
# add a non-existent C++ source to force the C++ compiler to be used for
# linking, so the C++ standard library is included for our C++ dependencies
nodist_EXTRA_graphml2gv_SOURCES = fake.cxx
endif

dijkstra_SOURCES = dijkstra.c

dijkstra_LDADD = \
	$(top_builddir)/lib/cgraph/libcgraph.la \
	$(top_builddir)/lib/cdt/libcdt.la

if WITH_THREADS_CXX
# add a non-existent C++ source to force the C++ compiler to be used for
# linking, so the C++ standard library is included for our C++ dependencies
nodist_EXTRA_dijkstra_SOURCES = fake.cxx
endif

gvgen_SOURCES = gvgen.c graph_generator.c

gvgen_LDADD = \
	$(top_builddir)/lib/cgraph/libcgraph.la \
	$(top_builddir)/lib/util/libutil_C.la \
	$(MATH_LIBS)

EXTRA_DIST = gmlscan.c gmlparse.c gmlparse.h

CLEANFILES = stamp.h
DISTCLEANFILES = $(pdf_DATA) gmlparse.[ch] gmlscan.c
