if(with_ipsepcola)

add_library(vpsc STATIC
  block.cpp
  blocks.cpp
  constraint.cpp
  csolve_VPSC.cpp
  generate-constraints.cpp
  solve_VPSC.cpp
  variable.cpp
)

target_include_directories(vpsc PRIVATE
  ..
)

endif()
