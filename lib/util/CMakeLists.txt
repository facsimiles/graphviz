add_library(util STATIC
  arena.c
  base64.c
  debug.c
  gv_find_me.c
  gv_fopen.c
  list.c
  random.c
  xml.c
)

target_compile_definitions(common_obj PRIVATE UTIL_EXPORTS)

target_include_directories(util PRIVATE ..)

if(WIN32 AND NOT MINGW)
  target_include_directories(util PRIVATE ../../windows/include/unistd)
endif()

# From https://gitlab.com/graphviz/graphviz/-/issues/1613 @mdwies 20093010
if(APPLE)
  target_compile_options(util PRIVATE "-fno-common")
endif()
