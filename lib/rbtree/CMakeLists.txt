add_library(rbtree STATIC
  red_black_tree.c
)

include_directories(rbtree PRIVATE
  ..
)
