add_definitions(-DLAB_GAMUT_EXPORTS)

add_library(edgepaintlib STATIC
  edge_distinct_coloring.h
  furtherest_point.h
  intersection.h
  lab.h
  lab_gamut.h
  node_distinct_coloring.h

  edge_distinct_coloring.c
  furtherest_point.c
  intersection.c
  lab.c
  lab_gamut.c
  node_distinct_coloring.c
)

target_include_directories(edgepaintlib PRIVATE
  ..
  ../cdt
  ../cgraph
  ../common
  ../gvc
  ../pathplan
)

target_link_libraries(edgepaintlib PRIVATE util)

# work around needed for an object library
# https://gitlab.com/graphviz/graphviz/-/issues/1613
if(APPLE)
  target_compile_options(edgepaintlib PRIVATE "-fno-common")
endif()
