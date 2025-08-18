add_library(dotgen STATIC
  # Header files
  aspect.h
  dot.h
  dotprocs.h

  # Source files
  aspect.c
  acyclic.c
  class1.c
  class2.c
  cluster.c
  compound.c
  conc.c
  decomp.c
  dotinit.c
  dotsplines.c
  fastgr.c
  flat.c
  mincross.c
  position.c
  rank.c
  sameport.c
)

target_include_directories(dotgen PRIVATE
  ..
  ../cdt
  ../cgraph
  ../common
  ../gvc
  ../pack
  ../pathplan
)

target_link_libraries(dotgen PRIVATE
  cgraph
  util
)
