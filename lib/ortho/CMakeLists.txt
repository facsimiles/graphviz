if(with_ortho)

add_library(ortho_obj OBJECT
  fPQ.c
  maze.c
  ortho.c
  partition.c
  rawgraph.c
  sgraph.c
  trapezoid.c
)

target_compile_definitions(ortho_obj PRIVATE GVC_EXPORTS)

target_include_directories(ortho_obj PRIVATE
  ..
  ../cdt
  ../cgraph
  ../common
  ../gvc
  ../pack
  ../pathplan
)

target_link_libraries(ortho_obj PRIVATE util)

add_library(ortho STATIC
  $<TARGET_OBJECTS:ortho_obj>
)

target_link_libraries(ortho PRIVATE util)

endif()
