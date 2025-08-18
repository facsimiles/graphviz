add_definitions(-DNEATOGEN_EXPORTS=1)

add_library(fdpgen STATIC
  # Header files
  clusteredges.h
  comp.h
  dbg.h
  fdp.h
  grid.h
  tlayout.h
  xlayout.h

  # Source files
  clusteredges.c
  comp.c
  dbg.c
  fdpinit.c
  grid.c
  layout.c
  tlayout.c
  xlayout.c
)

target_include_directories(fdpgen PRIVATE
  ..
  ../cdt
  ../cgraph
  ../common
  ../gvc
  ../pack
  ../pathplan
)

target_link_libraries(fdpgen PRIVATE
  cgraph
  gvc
  pathplan
  util
)
