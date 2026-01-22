add_library(util STATIC
  arena.c
  base64.c
  gv_find_me.c
  gv_fopen.c
  list.c
  random.c
  xml.c
)

target_include_directories(util PRIVATE ..)

if(WITH_THREADS STREQUAL "c11")
  target_sources(util PRIVATE mutex-c11.c)
elseif(WITH_THREADS STREQUAL "cxx")
  target_sources(util PRIVATE mutex-cxx.cc)
elseif(WITH_THREADS STREQUAL "no")
  target_sources(util PRIVATE mutex-no.c)
elseif(WITH_THREADS STREQUAL "posix")
  target_sources(util PRIVATE mutex-posix.c)
  target_link_libraries(util PRIVATE Threads::Threads)
endif()

if(WIN32 AND NOT MINGW)
  target_include_directories(util PRIVATE ../../windows/include/unistd)
endif()

# From https://gitlab.com/graphviz/graphviz/-/issues/1613 @mdwies 20093010
if(APPLE)
  target_compile_options(util PRIVATE "-fno-common")
endif()
