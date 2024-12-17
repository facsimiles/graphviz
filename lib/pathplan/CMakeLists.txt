add_definitions(-DPATHPLAN_EXPORTS)

add_library(pathplan
  # Header files
  pathgeom.h
  pathplan.h
  pathutil.h
  solvers.h
  tri.h
  vis.h
  vispath.h

  # Source files
  cvt.c
  inpoly.c
  route.c
  shortest.c
  shortestpth.c
  solvers.c
  triang.c
  util.c
  visibility.c
)

target_include_directories(pathplan
  PUBLIC
    $<INSTALL_INTERFACE:include/graphviz>
  PRIVATE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/..>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
)

# Installation location of library files
install(
  TARGETS pathplan
  EXPORT pathplanConfig
  RUNTIME DESTINATION ${BINARY_INSTALL_DIR}
  LIBRARY DESTINATION ${LIBRARY_INSTALL_DIR}
  ARCHIVE DESTINATION ${LIBRARY_INSTALL_DIR}
)

# Specify headers to be installed
install(
  FILES pathgeom.h pathplan.h
  DESTINATION ${HEADER_INSTALL_DIR}
)

# Specify man pages to be installed
if(GZIP)
  add_custom_target(man-pathplan ALL DEPENDS pathplan.3.gz
                    COMMENT "pathplan man page")
  add_custom_command(
    OUTPUT pathplan.3.gz
    COMMAND ${GZIP} -9 --no-name --to-stdout pathplan.3
      >"${CMAKE_CURRENT_BINARY_DIR}/pathplan.3.gz"
    MAIN_DEPENDENCY pathplan.3
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMENT "compress pathplan man page")
  install(
    FILES ${CMAKE_CURRENT_BINARY_DIR}/pathplan.3.gz
    DESTINATION ${MAN_INSTALL_DIR}/man3)
else()
  install(
    FILES pathplan.3
    DESTINATION ${MAN_INSTALL_DIR}/man3
  )
endif()

# Specify library version and soversion
set_target_properties(pathplan PROPERTIES
  VERSION 4.0.0
  SOVERSION 4
)

if(BUILD_SHARED_LIBS)
  install(
    EXPORT pathplanConfig
    FILE pathplanConfig.cmake
    NAMESPACE graphviz::
    DESTINATION ${LIBRARY_INSTALL_DIR}/cmake/graphviz
  )
endif()
