add_definitions(-DNEATOGEN_EXPORTS=1)

add_library(osage STATIC
  # Header files
  osage.h

  # Source files
  osageinit.c
)

target_include_directories(osage PRIVATE
  ..
  ../cdt
  ../cgraph
  ../common
  ../gvc
  ../pack
  ../pathplan
)

target_link_libraries(osage PRIVATE
  cgraph
)
