add_definitions(-DNEATOGEN_EXPORTS=1)

add_library(circogen STATIC
  block.c
  blockpath.c
  blocktree.c
  circpos.c
  circular.c
  circularinit.c
  edgelist.c
  nodelist.c
)

target_include_directories(circogen PRIVATE
  ..
  ../cdt
  ../cgraph
  ../common
  ../gvc
  ../pack
  ../pathplan)

target_link_libraries(circogen PRIVATE
  cgraph
)
