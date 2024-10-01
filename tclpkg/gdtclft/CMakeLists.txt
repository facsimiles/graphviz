if(GD_FOUND)
  add_library(gdtclft SHARED
    gdtclft.c
  )

  target_include_directories(gdtclft PRIVATE ../../lib)

  target_include_directories(gdtclft SYSTEM PRIVATE
    ${GD_INCLUDE_DIRS}
  )
  target_link_libraries(gdtclft PRIVATE ${GD_LIBRARIES})

  target_include_directories(gdtclft SYSTEM PRIVATE ${TCL_INCLUDE_PATH})
  target_link_libraries(gdtclft PRIVATE ${TCL_LIBRARY})

  install(
    TARGETS gdtclft
    RUNTIME DESTINATION ${BINARY_INSTALL_DIR}
    LIBRARY DESTINATION ${LIBRARY_INSTALL_DIR}/graphviz/tcl
    ARCHIVE DESTINATION ${LIBRARY_INSTALL_DIR}
  )

  add_custom_target(gdtclft_pkgindex
    COMMAND
      ${Python3_EXECUTABLE} "${CMAKE_CURRENT_SOURCE_DIR}/../mkpkgindex.py"
      --file libgdtclft${CMAKE_SHARED_LIBRARY_SUFFIX}
      --name Gdtclft
      --version "${GRAPHVIZ_VERSION_STRING}"
    DEPENDS ../mkpkgindex.py
    BYPRODUCTS pkgIndex.tcl
    COMMENT "create gdtclft package index"
  )
  add_dependencies(gdtclft_pkgindex gdtclft)

  if(GZIP)
    add_custom_target(man-gdtclft ALL DEPENDS gdtclft.3tcl.gz
                      COMMENT "gdtclft man page")
    add_custom_command(
      OUTPUT gdtclft.3tcl.gz
      COMMAND ${GZIP} -9 --no-name --to-stdout gdtclft.3tcl
        >"${CMAKE_CURRENT_BINARY_DIR}/gdtclft.3tcl.gz"
      MAIN_DEPENDENCY gdtclft.3tcl
      WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
      COMMENT "compress gdtclft man page")
    install(
      FILES ${CMAKE_CURRENT_BINARY_DIR}/gdtclft.3tcl.gz
      DESTINATION ${MAN_INSTALL_DIR}/man3)
  else()
    install(
      FILES gdtclft.3tcl
      DESTINATION ${MAN_INSTALL_DIR}/man3
    )
  endif()
endif()
