## Process this file with automake to produce Makefile.in

AM_CPPFLAGS = \
	-I$(top_srcdir)/lib \
        -I$(top_srcdir)/libltdl \
	-I$(top_srcdir)/lib/gvc \
	-I$(top_srcdir)/lib/pack \
	-I$(top_srcdir)/lib/xdot \
	-I$(top_srcdir)/lib/pathplan \
	-I$(top_srcdir)/lib/cgraph \
	-I$(top_srcdir)/lib/cdt $(GD_INCLUDES) $(EXPAT_INCLUDES) $(Z_INCLUDES)

if WITH_WIN32
AM_CFLAGS = -D_BLD_gvc=1
AM_CPPFLAGS += -O0
endif

BUILT_SOURCES = colortbl.h htmlparse.h

pkginclude_HEADERS = arith.h geom.h color.h types.h textspan.h usershape.h
noinst_HEADERS = render.h utils.h memory.h \
	geomprocs.h colorprocs.h colortbl.h entities.h globals.h \
	logic.h const.h macros.h htmllex.h htmltable.h pointset.h intset.h \
	timing.h ps_font_equiv.h
noinst_LTLIBRARIES = libcommon_C.la

libcommon_C_la_SOURCES = arrows.c colxlate.c ellipse.c textspan.c \
	args.c memory.c globals.c htmllex.c htmlparse.y htmltable.c input.c \
	pointset.c intset.c postproc.c routespl.c splines.c psusershape.c \
	timing.c labels.c ns.c shapes.c utils.c geom.c taper.c \
	output.c emit.c xml.c \
	color_names

colxlate.o colxlate.lo : colortbl.h

colortbl.h : color_lib
	$(AWK) -f $(top_srcdir)/awk/colortbl.awk color_lib > colortbl.h

# ensure color names are properly sorted for bsearch operation 
color_lib : brewer_lib svgcolor_lib $(top_srcdir)/lib/common/color_names
	cat brewer_lib svgcolor_lib $(top_srcdir)/lib/common/color_names | LC_ALL=C $(SORT) > color_lib

brewer_lib : $(top_srcdir)/lib/common/brewer_colors  $(top_srcdir)/awk/brewer.awk
	$(AWK) -f $(top_srcdir)/awk/brewer.awk $(top_srcdir)/lib/common/brewer_colors > brewer_lib

if WITH_CYGWIN
AWK_OPTIONS=-v RS="\r*\n"
endif

svgcolor_lib : $(top_srcdir)/lib/common/svgcolor_names  $(top_srcdir)/awk/svgcolor.awk
	 $(AWK) $(AWK_OPTIONS) -f $(top_srcdir)/awk/svgcolor.awk $(top_srcdir)/lib/common/svgcolor_names > svgcolor_lib

htmllex.o htmllex.lo: htmllex.c htmllex.h htmlparse.h
htmlparse.o htmlparse.lo: htmlparse.c htmlparse.h
                                                                                
htmlparse.c htmlparse.h: $(top_srcdir)/lib/common/htmlparse.y
	@YACC@ -Wno-yacc -dv $(top_srcdir)/lib/common/htmlparse.y -o htmlparse.c

DISTCLEANFILES = brewer_lib color_lib colortbl.h \
	htmlparse.[ch]

EXTRA_DIST = README.imap \
	htmlparse.c htmlparse.h \
	entities.html entities.tcl \
	brewer_colors brewer_lib svgcolor_names svgcolor_lib \
	color_names color_lib colortbl.h
