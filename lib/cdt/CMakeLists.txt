add_definitions(-DEXPORT_CDT)

add_library(cdt
  # Header files
  cdt.h
  dthdr.h

  # Source files
  dtclose.c
  dtdisc.c
  dtextract.c
  dtflatten.c
  dthash.c
  dtmethod.c
  dtopen.c
  dtrenew.c
  dtrestore.c
  dtsize.c
  dtstat.c
  dtstrhash.c
  dttree.c
  dtview.c
  dtwalk.c
)

target_include_directories(cdt PRIVATE
  ..
  ${CMAKE_CURRENT_SOURCE_DIR})

# Installation location of library files
install(
  TARGETS cdt
  EXPORT cdtConfig
  RUNTIME DESTINATION ${BINARY_INSTALL_DIR}
  LIBRARY DESTINATION ${LIBRARY_INSTALL_DIR}
  ARCHIVE DESTINATION ${LIBRARY_INSTALL_DIR}
)

# Specify headers to be installed
install(
  FILES cdt.h
  DESTINATION ${HEADER_INSTALL_DIR}
)

# Specify man pages to be installed
if(GZIP)
  add_custom_target(man-cdt ALL DEPENDS cdt.3.gz
                    COMMENT "cdt man page")
  add_custom_command(
    OUTPUT cdt.3.gz
    COMMAND ${GZIP} -9 --no-name --to-stdout cdt.3
      >"${CMAKE_CURRENT_BINARY_DIR}/cdt.3.gz"
    MAIN_DEPENDENCY cdt.3
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMENT "compress cdt man page")
  install(
    FILES ${CMAKE_CURRENT_BINARY_DIR}/cdt.3.gz
    DESTINATION ${MAN_INSTALL_DIR}/man3)
else()
  install(
    FILES cdt.3
    DESTINATION ${MAN_INSTALL_DIR}/man3
  )
endif()

# Specify library version and soversion
set_target_properties(cdt PROPERTIES
  VERSION 6.0.0
  SOVERSION 6
)

if(BUILD_SHARED_LIBS)
  install(
    EXPORT cdtConfig
    FILE cdtConfig.cmake
    NAMESPACE graphviz::
    DESTINATION ${LIBRARY_INSTALL_DIR}/cmake/graphviz
  )
endif()
