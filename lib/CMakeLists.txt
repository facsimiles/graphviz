# teach libraries how to find other libraries, even if there is another copy of
# Graphviz installed in the system default paths
if(APPLE)
  list(APPEND CMAKE_INSTALL_RPATH "@executable_path")
else()
  list(APPEND CMAKE_INSTALL_RPATH "\$ORIGIN")
endif()

# No dependencies
add_subdirectory(ast)
add_subdirectory(cdt)
add_subdirectory(circogen)
add_subdirectory(common)
add_subdirectory(dotgen)
add_subdirectory(edgepaint)
add_subdirectory(expr)
add_subdirectory(fdpgen)
add_subdirectory(glcomp)
add_subdirectory(gvpr)
add_subdirectory(label)
add_subdirectory(mingle)
add_subdirectory(neatogen)
add_subdirectory(ortho)
add_subdirectory(osage)
add_subdirectory(pack)
add_subdirectory(patchwork)
add_subdirectory(pathplan)
add_subdirectory(sfio)
add_subdirectory(rbtree)
add_subdirectory(sparse)
add_subdirectory(sfdpgen)
add_subdirectory(topfish)
add_subdirectory(twopigen)
add_subdirectory(util)
add_subdirectory(vmalloc)
add_subdirectory(vpsc)
add_subdirectory(xdot)

# Dependent on: cdt
add_subdirectory(cgraph)

# Multiple dependencies
add_subdirectory(gvc)

# C++ API libraries
if(with_cxx_api)
  add_subdirectory(cgraph++)
  add_subdirectory(gvc++)
endif()

if(BUILD_SHARED_LIBS)
  install(
    FILES graphvizConfig.cmake
    DESTINATION ${LIBRARY_INSTALL_DIR}/cmake/graphviz
  )
endif()
