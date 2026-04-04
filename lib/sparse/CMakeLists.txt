add_library(sparse STATIC
  clustering.c
  color_palette.c
  colorutil.c
  DotIO.c
  general.c
  mq.c
  QuadTree.c
  SparseMatrix.c
)

target_include_directories(sparse PRIVATE
  ..
  ../cdt
  ../cgraph
  ../common
)

target_link_libraries(sparse PRIVATE util)
