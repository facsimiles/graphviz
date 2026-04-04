add_library(patchwork STATIC
  patchwork.c
  patchworkinit.c
  tree_map.c
)

target_include_directories(patchwork PRIVATE
  ..
  ../cdt
  ../cgraph
  ../common
  ../gvc
  ../pack
  ../pathplan
)

target_link_libraries(patchwork PRIVATE
  cgraph
  gvc
  util
)
