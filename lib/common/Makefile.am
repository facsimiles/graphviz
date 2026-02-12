## Process this file with automake to produce Makefile.in

AM_CPPFLAGS = \
	-I$(top_srcdir)/lib \
	-I$(top_builddir)/lib \
	-I$(top_srcdir)/libltdl \
	-I$(top_srcdir)/lib/gvc \
	-I$(top_srcdir)/lib/pack \
	-I$(top_srcdir)/lib/pathplan \
	-I$(top_srcdir)/lib/cgraph \
	-I$(top_srcdir)/lib/cdt $(GD_INCLUDES) $(Z_INCLUDES)

if WITH_WIN32
AM_CFLAGS = -DGVC_EXPORTS
endif

BUILT_SOURCES = colortbl.h entities.h htmlparse.h

pkginclude_HEADERS = arith.h geom.h color.h types.h textspan.h usershape.h
noinst_HEADERS = boxes.h render.h utils.h \
	geomprocs.h colorprocs.h globals.h \
	const.h macros.h htmllex.h htmltable.h pointset.h \
	textspan_lut.h ps_font_equiv.h
noinst_LTLIBRARIES = libcommon_C.la

libcommon_C_la_SOURCES = arrows.c colxlate.c ellipse.c textspan.c textspan_lut.c \
	args.c globals.c htmllex.c htmlparse.y htmltable.c input.c \
	pointset.c postproc.c routespl.c splines.c psusershape.c \
	timing.c labels.c ns.c shapes.c utils.c geom.c taper.c \
	output.c emit.c \
	color_names
libcommon_C_la_CPPFLAGS = $(AM_CPPFLAGS) $(EXPAT_CFLAGS)
libcommon_C_la_LIBADD = \
	$(top_builddir)/lib/xdot/libxdot.la $(EXPAT_LIBS)

colxlate.$(OBJEXT) colxlate.lo: colortbl.h

colortbl.h: $(top_srcdir)/lib/common/brewer_colors \
            $(top_srcdir)/lib/common/svgcolor_names \
            $(top_srcdir)/lib/common/color_names \
            $(top_srcdir)/lib/common/make_colortbl.py
	$(PYTHON3) $(top_srcdir)/lib/common/make_colortbl.py \
	  $(top_srcdir)/lib/common/brewer_colors \
	  $(top_srcdir)/lib/common/svgcolor_names \
	  $(top_srcdir)/lib/common/color_names colortbl.h

utils.$(OBJEXT) utils.lo: entities.h

entities.h: $(top_srcdir)/lib/common/entities.html \
            $(top_srcdir)/lib/common/entities.py
	$(PYTHON3) $(top_srcdir)/lib/common/entities.py "$@"

htmllex.$(OBJEXT) htmllex.lo: htmllex.c htmllex.h htmlparse.h
htmlparse.$(OBJEXT) htmlparse.lo: htmlparse.c htmlparse.h

htmlparse.c htmlparse.h: $(top_srcdir)/lib/common/htmlparse.y
	$(YACC) -Wno-yacc -dv $(top_srcdir)/lib/common/htmlparse.y -o htmlparse.c

DISTCLEANFILES = colortbl.h entities.h htmlparse.[ch]

EXTRA_DIST = README.imap \
	htmlparse.c htmlparse.h \
	entities.h entities.html entities.py \
	brewer_colors svgcolor_names \
	color_names colortbl.h \
	make_colortbl.py
