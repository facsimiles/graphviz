BISON_TARGET(Grammar ${CMAKE_CURRENT_SOURCE_DIR}/grammar.y
                     ${CMAKE_CURRENT_BINARY_DIR}/grammar.c)
FLEX_TARGET(Scan
  ${CMAKE_CURRENT_SOURCE_DIR}/scan.l
  ${CMAKE_CURRENT_BINARY_DIR}/scan.c
  COMPILE_FLAGS --case-insensitive
)
ADD_FLEX_BISON_DEPENDENCY(Scan Grammar)

add_definitions(-DEXPORT_CGRAPH -DEXPORT_CGHDR)
if(WIN32 AND NOT MINGW)
  add_definitions(-DYY_NO_UNISTD_H)
endif()

add_library(cgraph
  # Header files
  agxbuf.h
  cghdr.h
  cgraph.h
  gv_ctype.h
  gv_math.h
  ingraphs.h
  list.h
  node_set.h
  rdr.h
  strview.h
  tokenize.h

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
  mem.c
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

target_include_directories(cgraph PRIVATE
  ..
  ../cdt
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
)

target_link_libraries(cgraph PRIVATE util)
target_link_libraries(cgraph PUBLIC cdt)

# Installation location of library files
install(
  TARGETS cgraph
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
  VERSION 6.0.0
  SOVERSION 6
)
