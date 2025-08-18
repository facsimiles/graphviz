# teach `prune` how to find plugins and libraries, even if there is another copy
# of Graphviz installed in the system default paths
if(APPLE)
  list(APPEND CMAKE_INSTALL_RPATH "@executable_path/../${PLUGIN_INSTALL_DIR}")
  list(APPEND CMAKE_INSTALL_RPATH "@executable_path/../${LIBRARY_INSTALL_DIR}")
else()
  list(APPEND CMAKE_INSTALL_RPATH "\$ORIGIN/../${PLUGIN_INSTALL_DIR}")
  list(APPEND CMAKE_INSTALL_RPATH "\$ORIGIN/../${LIBRARY_INSTALL_DIR}")
endif()

add_executable(prune prune.c)
target_include_directories(prune PRIVATE
  ../../lib
  ../../lib/cdt)
target_link_libraries(prune PRIVATE cdt cgraph util)

if(GETOPT_FOUND)
  target_include_directories(prune SYSTEM PRIVATE
    ${GETOPT_INCLUDE_DIRS}
  )
endif()

if(NOT HAVE_GETOPT_H)
  target_link_libraries(prune PRIVATE ${GETOPT_LINK_LIBRARIES})
endif()

install(
  TARGETS prune
  RUNTIME DESTINATION ${BINARY_INSTALL_DIR}
  LIBRARY DESTINATION ${LIBRARY_INSTALL_DIR}
  ARCHIVE DESTINATION ${LIBRARY_INSTALL_DIR}
)

if(GZIP)
  add_custom_target(man-prune ALL DEPENDS prune.1.gz
                    COMMENT "prune man page")
  add_custom_command(
    OUTPUT prune.1.gz
    COMMAND ${GZIP} -9 --no-name --to-stdout prune.1
      >"${CMAKE_CURRENT_BINARY_DIR}/prune.1.gz"
    MAIN_DEPENDENCY prune.1
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMENT "compress prune man page")
  install(
    FILES ${CMAKE_CURRENT_BINARY_DIR}/prune.1.gz
    DESTINATION ${MAN_INSTALL_DIR}/man1)
else()
  install(
    FILES prune.1
    DESTINATION ${MAN_INSTALL_DIR}/man1
  )
endif()
