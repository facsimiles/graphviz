add_definitions(-DEXPORT_GVPR)

add_library(gvpr
  # Header files
  actions.h
  compile.h
  gdefs.h
  gprdata.inc
  gprstate.h
  gvpr.h
  parse.h
  trieFA.h

  # Source files
  actions.c
  compile.c
  gvpr.c
  gprstate.c
  parse.c
)

target_compile_definitions(gvpr PRIVATE
  DFLT_GVPRPATH="${DATA_INSTALL_DIR}/graphviz/gvpr")

target_include_directories(gvpr
  PUBLIC
    $<INSTALL_INTERFACE:include/graphviz>
  PRIVATE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/..>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/..>
      # needed to find generated expr/exparse.h
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/../common>
      # needed to find generated common/colortbl.h
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../cdt>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../cgraph>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../common>
)

if(GETOPT_FOUND)
  target_include_directories(gvpr SYSTEM PRIVATE
    ${GETOPT_INCLUDE_DIRS}
  )
endif()

if(WIN32 AND NOT MINGW)
  target_include_directories(gvpr PRIVATE ../../windows/include/unistd)
endif()

target_link_libraries(gvpr PRIVATE
  $<TARGET_OBJECTS:ast>
  cgraph
  gvc
  $<TARGET_OBJECTS:expr>
  $<TARGET_OBJECTS:sfio>
  $<TARGET_OBJECTS:vmalloc>
)

if(NOT HAVE_GETOPT_H)
  target_link_libraries(gvpr PRIVATE ${GETOPT_LINK_LIBRARIES})
endif()

# Installation location of library files
install(
  TARGETS gvpr
  EXPORT gvprConfig
  RUNTIME DESTINATION ${BINARY_INSTALL_DIR}
  LIBRARY DESTINATION ${LIBRARY_INSTALL_DIR}
  ARCHIVE DESTINATION ${LIBRARY_INSTALL_DIR}
)

# Specify headers to be installed
install(
  FILES gvpr.h
  DESTINATION ${HEADER_INSTALL_DIR}
)

# Specify man pages to be installed
if(GZIP)
  add_custom_target(man-gvprlib ALL DEPENDS gvpr.3.gz
                    COMMENT "gvpr library man page")
  add_custom_command(
    OUTPUT gvpr.3.gz
    COMMAND ${GZIP} -9 --no-name --to-stdout gvpr.3
      >"${CMAKE_CURRENT_BINARY_DIR}/gvpr.3.gz"
    MAIN_DEPENDENCY gvpr.3
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMENT "compress gvpr library man page")
  install(
    FILES ${CMAKE_CURRENT_BINARY_DIR}/gvpr.3.gz
    DESTINATION ${MAN_INSTALL_DIR}/man3)
else()
  install(
    FILES gvpr.3
    DESTINATION ${MAN_INSTALL_DIR}/man3
  )
endif()

# Specify library version and soversion
set_target_properties(gvpr PROPERTIES
  VERSION 2.0.0
  SOVERSION 2
)

if(BUILD_SHARED_LIBS)
  install(
    EXPORT gvprConfig
    FILE gvprConfig.cmake
    NAMESPACE graphviz::
    DESTINATION ${LIBRARY_INSTALL_DIR}/cmake/graphviz
  )
endif()
