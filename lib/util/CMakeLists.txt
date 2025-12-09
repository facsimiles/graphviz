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

if(${CMAKE_CXX_COMPILER_ID} STREQUAL MSVC)
  target_sources(util PRIVATE mutex-cxx.cc)
else()
  if(CMAKE_USE_PTHREADS_INIT)
    target_sources(util PRIVATE mutex-pthread.c)
  else()
    target_sources(util PRIVATE mutex-c11.c)
  endif()
  target_link_libraries(util PRIVATE Threads::Threads)
endif()

if(WIN32 AND NOT MINGW)
  target_include_directories(util PRIVATE ../../windows/include/unistd)
endif()

# From https://gitlab.com/graphviz/graphviz/-/issues/1613 @mdwies 20093010
if(APPLE)
  target_compile_options(util PRIVATE "-fno-common")
endif()
