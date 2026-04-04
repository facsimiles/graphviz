if(WITH_SMYRNA)

  add_library(topfish STATIC
    hierarchy.c
    rescale_layout.c
  )

  target_include_directories(topfish PRIVATE
    ..
    ../common
  )

  target_link_libraries(topfish PRIVATE
    gvc
    neatogen
  )

endif()
