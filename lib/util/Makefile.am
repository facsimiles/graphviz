AM_CPPFLAGS = -I$(top_srcdir)/lib

noinst_HEADERS = \
  agxbuf.h \
  alloc.h \
  api.h \
  arena.h \
  asan.h \
  base64.h \
  bitarray.h \
  debug.h \
  exit.h \
  gv_ctype.h \
  gv_find_me.h \
  gv_fopen.h \
  gv_ftell.h \
  gv_math.h \
  itos.h \
  list.h \
  list-private.h \
  lockfile.h \
  mutex.h \
  optional.h \
  overflow.h \
  path.h \
  prisize_t.h \
  random.h \
  sort.h \
  startswith.h \
  strcasecmp.h \
  streq.h \
  strview.h \
  tokenize.h \
  unreachable.h \
  unused.h \
  xml.h
noinst_LTLIBRARIES = libutil_C.la

if WITH_THREADS_NO
MUTEX=mutex-no.c
endif
if WITH_THREADS_C11
MUTEX=mutex-c11.c
endif
if WITH_THREADS_CXX
MUTEX=mutex-cxx.cc
endif
if WITH_THREADS_POSIX
MUTEX=mutex-posix.c
endif

libutil_C_la_SOURCES = arena.c base64.c gv_find_me.c gv_fopen.c list.c \
	$(MUTEX) random.c xml.c
libutil_C_la_CPPFLAGS = $(AM_CPPFLAGS)

EXTRA_DIST = mutex-c11.c mutex-cxx.cc mutex-no.c mutex-posix.c README
