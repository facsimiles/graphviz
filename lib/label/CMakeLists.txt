add_library(label STATIC
  # Header files
  index.h
  node.h
  rectangle.h
  split.q.h
  xlabels.h

  # Source files
  index.c
  node.c
  rectangle.c
  split.q.c
  xlabels.c
)

target_include_directories(label PRIVATE
  ..
  ../cdt
  ../cgraph
  ../common
)

target_link_libraries(label PRIVATE
  cdt
)
