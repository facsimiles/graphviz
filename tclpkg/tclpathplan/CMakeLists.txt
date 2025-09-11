add_library(tclplan SHARED
  find_ints.c
  intersect.c
  makecw.c
  tclpathplan.c
  wrapper.c
)

target_include_directories(tclplan PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}
  ../../lib
  ../../lib/pathplan
)

target_link_libraries(tclplan PRIVATE
  pathplan
  tclstubs
  util
)

target_include_directories(tclplan SYSTEM PRIVATE ${TCL_INCLUDE_PATH})
target_link_libraries(tclplan PRIVATE ${TCL_LIBRARY})

install(
  TARGETS tclplan
  RUNTIME DESTINATION ${BINARY_INSTALL_DIR}
  LIBRARY DESTINATION ${LIBRARY_INSTALL_DIR}/graphviz/tcl
  ARCHIVE DESTINATION ${LIBRARY_INSTALL_DIR}
)

add_custom_target(tclplan_pkgindex
  COMMAND
    ${Python3_EXECUTABLE} "${CMAKE_CURRENT_SOURCE_DIR}/../mkpkgindex.py"
    --file libtclplan${CMAKE_SHARED_LIBRARY_SUFFIX}
    --name Tclpathplan
    --version "${GRAPHVIZ_VERSION}"
  DEPENDS ../mkpkgindex.py
  BYPRODUCTS pkgIndex.tcl
  COMMENT "create tclplan package index"
)
add_dependencies(tclplan_pkgindex tclplan)
