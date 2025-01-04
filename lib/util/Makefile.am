AM_CPPFLAGS = -I$(top_srcdir)/lib

noinst_HEADERS = \
  agxbuf.h \
  alloc.h \
  bitarray.h \
  exit.h \
  gv_ctype.h \
  gv_fopen.h \
  gv_math.h \
  overflow.h \
  prisize_t.h \
  sort.h \
  startswith.h \
  strcasecmp.h \
  streq.h \
  strview.h \
  tokenize.h \
  unreachable.h \
  unused.h
noinst_LTLIBRARIES = libutil_C.la

libutil_C_la_SOURCES = gv_fopen.c
libutil_C_la_CPPFLAGS = $(AM_CPPFLAGS)

EXTRA_DIST = README
