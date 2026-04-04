add_definitions(-DNEATOGEN_EXPORTS=1)

add_library(osage STATIC
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
  util
)
