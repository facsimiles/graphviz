# teach libraries how to find plugins and libraries, even if there is another
# copy of Graphviz installed in the system default paths
if(APPLE)
  list(APPEND CMAKE_INSTALL_RPATH
    "@executable_path/../../../${PLUGIN_INSTALL_DIR}")
  list(APPEND CMAKE_INSTALL_RPATH "@executable_path/../..")
else()
  list(APPEND CMAKE_INSTALL_RPATH "\$ORIGIN/../../../${PLUGIN_INSTALL_DIR}")
  list(APPEND CMAKE_INSTALL_RPATH "\$ORIGIN/../..")
endif()

if(ENABLE_TCL)
  add_subdirectory(gdtclft)
  add_subdirectory(tcldot)
  add_subdirectory(tclpathplan)
  add_subdirectory(tclstubs)
endif()

add_subdirectory(gv)

if(ENABLE_TCL)
  if(GD_FOUND)
    set(GD_INDEX "gdtclft/pkgIndex.tcl")
  else()
    set(GD_INDEX "")
  endif()
  if(ENABLE_SWIG)
    set(SWIG_INDEX "gv/pkgIndex.tcl")
  else()
    set(SWIG_INDEX "")
  endif()
  add_custom_target(tcl_pkgindex
    ALL
    COMMAND ${CMAKE_COMMAND} -E cat tcldot/pkgIndex.tcl
      tclpathplan/pkgIndex.tcl ${GD_INDEX} ${SWIG_INDEX} >pkgIndex.tcl
    COMMAND ${CMAKE_COMMAND} -E echo "\"#" "end\"" >>pkgIndex.tcl
    BYPRODUCTS pkgIndex.tcl
    COMMENT "create TCL package index"
  )
  add_dependencies(tcl_pkgindex tcldot_pkgindex tclplan_pkgindex)
  if(GD_FOUND)
    add_dependencies(tcl_pkgindex gdtclft_pkgindex)
  endif()
  if(ENABLE_SWIG)
    add_dependencies(tcl_pkgindex gv_tcl_pkgindex)
  endif()

  install(
    FILES ${CMAKE_CURRENT_BINARY_DIR}/pkgIndex.tcl
    DESTINATION ${LIBRARY_INSTALL_DIR}/graphviz/tcl
  )
endif()
