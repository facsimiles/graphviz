if(with_sfdp)

add_definitions(-DNEATOGEN_EXPORTS=1)

add_library(sfdpgen STATIC
  Multilevel.c
  post_process.c
  sfdpinit.c
  sparse_solve.c
  spring_electrical.c
  stress_model.c
)

target_include_directories(sfdpgen PRIVATE
  ..
  ../cdt
  ../cgraph
  ../common
  ../gvc
  ../pack
  ../pathplan
)

target_link_libraries(sfdpgen PRIVATE
  gvc
  neatogen
  sparse
  util
)

endif()
