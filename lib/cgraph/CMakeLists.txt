BISON_TARGET(Grammar ${CMAKE_CURRENT_SOURCE_DIR}/grammar.y
                     ${CMAKE_CURRENT_BINARY_DIR}/grammar.c)
FLEX_TARGET(Scan
  ${CMAKE_CURRENT_SOURCE_DIR}/scan.l
  ${CMAKE_CURRENT_BINARY_DIR}/scan.c
  COMPILE_FLAGS --case-insensitive
)
ADD_FLEX_BISON_DEPENDENCY(Scan Grammar)

if(WIN32 AND NOT MINGW)
  add_definitions(-DYY_NO_UNISTD_H)
endif()

add_library(cgraph
  # Header files
  cghdr.h
  cgraph.h
  ingraphs.h
  node_set.h
  rdr.h

  # Source files
  acyclic.c
  agerror.c
  apply.c
  attr.c
  edge.c
  graph.c
  id.c
  imap.c
  ingraphs.c
  io.c
  node.c
  node_induce.c
  obj.c
  rec.c
  refstr.c
  subg.c
  tred.c
  unflatten.c
  utils.c
  write.c

  # Generated files
  ${BISON_Grammar_OUTPUTS}
  ${FLEX_Scan_OUTPUTS}
)

if(HAVE_C_WUNUSED_BUT_SET_VARIABLE)
  set_source_files_properties(${BISON_Grammar_OUTPUTS}
    PROPERTIES COMPILE_FLAGS -Wno-unused-but-set-variable)
endif()

target_compile_definitions(cgraph PRIVATE EXPORT_CGRAPH EXPORT_CGHDR)

target_include_directories(cgraph
  PUBLIC
    $<INSTALL_INTERFACE:include>
    $<INSTALL_INTERFACE:include/graphviz>
  PRIVATE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/..>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../cdt>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
)

target_link_libraries(cgraph PRIVATE util)
target_link_libraries(cgraph PUBLIC cdt)

# Installation location of library files
install(
  TARGETS cgraph
  EXPORT graphvizTargets
  RUNTIME DESTINATION ${BINARY_INSTALL_DIR}
  LIBRARY DESTINATION ${LIBRARY_INSTALL_DIR}
  ARCHIVE DESTINATION ${LIBRARY_INSTALL_DIR}
)

# Specify headers to be installed
install(
  FILES cgraph.h
  DESTINATION ${HEADER_INSTALL_DIR}
)

# Specify man pages to be installed
if(GZIP)
  add_custom_target(man-cgraph ALL DEPENDS cgraph.3.gz
                    COMMENT "cgraph man page")
  add_custom_command(
    OUTPUT cgraph.3.gz
    COMMAND ${GZIP} -9 --no-name --to-stdout cgraph.3
      >"${CMAKE_CURRENT_BINARY_DIR}/cgraph.3.gz"
    MAIN_DEPENDENCY cgraph.3
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMENT "compress cgraph man page")
  install(
    FILES ${CMAKE_CURRENT_BINARY_DIR}/cgraph.3.gz
    DESTINATION ${MAN_INSTALL_DIR}/man3)
else()
  install(
    FILES cgraph.3
    DESTINATION ${MAN_INSTALL_DIR}/man3
  )
endif()

# Specify library version and soversion
set_target_properties(cgraph PROPERTIES
  VERSION 8.0.6
  SOVERSION 8
)
