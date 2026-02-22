if(with_sfdp)

  add_executable(cluster
    cluster.c
    country_graph_coloring.c
    make_map.c
    power.c
  )

  target_include_directories(cluster PRIVATE
    ../../lib
    ../../lib/cdt
    ../../lib/cgraph
    ../../lib/common
  )

  if(GETOPT_FOUND)
    target_include_directories(cluster SYSTEM PRIVATE
      ${GETOPT_INCLUDE_DIRS}
    )
  endif()

  target_link_libraries(cluster PRIVATE
    cgraph
    edgepaintlib
    gvc
    neatogen
    rbtree
    sfdpgen
    sparse
    util
  )

  if(NOT HAVE_GETOPT_H)
    target_link_libraries(cluster PRIVATE ${GETOPT_LINK_LIBRARIES})
  endif()

  find_program(GZIP gzip)
  if(GZIP)
    add_custom_target(man-cluster ALL DEPENDS cluster.1.gz
                      COMMENT "cluster man page")
    add_custom_command(
      OUTPUT cluster.1.gz
      COMMAND ${GZIP} -9 --no-name --to-stdout cluster.1
        >"${CMAKE_CURRENT_BINARY_DIR}/cluster.1.gz"
      MAIN_DEPENDENCY cluster.1
      WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
      COMMENT "compress cluster man page")
    install(
      FILES ${CMAKE_CURRENT_BINARY_DIR}/cluster.1.gz
      DESTINATION ${MAN_INSTALL_DIR}/man1)
  else()
    install(
      FILES cluster.1
      DESTINATION ${MAN_INSTALL_DIR}/man1
    )
  endif()

  install(
    TARGETS cluster
    RUNTIME DESTINATION ${BINARY_INSTALL_DIR}
    LIBRARY DESTINATION ${LIBRARY_INSTALL_DIR}
    ARCHIVE DESTINATION ${LIBRARY_INSTALL_DIR}
  )

  add_executable(gvmap
    country_graph_coloring.c
    country_graph_coloring.h
    gvmap.c
    make_map.c
    make_map.h
    power.c
    power.h
  )

  target_include_directories(gvmap PRIVATE
    ../../lib
    ../../lib/common
    ../../lib/cgraph
    ../../lib/cdt
  )

  if(GETOPT_FOUND)
    target_include_directories(gvmap SYSTEM PRIVATE
      ${GETOPT_INCLUDE_DIRS}
    )
  endif()

  target_link_libraries(gvmap PRIVATE
    cgraph
    edgepaintlib
    gvc
    neatogen
    rbtree
    sfdpgen
    sparse
    util
  )

  if(NOT HAVE_GETOPT_H)
    target_link_libraries(gvmap PRIVATE ${GETOPT_LINK_LIBRARIES})
  endif()

  if(GZIP)
    add_custom_target(man-gvmap ALL DEPENDS gvmap.1.gz
                      COMMENT "gvmap man page")
    add_custom_command(
      OUTPUT gvmap.1.gz
      COMMAND ${GZIP} -9 --no-name --to-stdout gvmap.1
        >"${CMAKE_CURRENT_BINARY_DIR}/gvmap.1.gz"
      MAIN_DEPENDENCY gvmap.1
      WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
      COMMENT "compress gvmap man page")
    install(
      FILES ${CMAKE_CURRENT_BINARY_DIR}/gvmap.1.gz
      DESTINATION ${MAN_INSTALL_DIR}/man1)
  else()
    install(
      FILES gvmap.1
      DESTINATION ${MAN_INSTALL_DIR}/man1
    )
  endif()

  install(
    TARGETS gvmap
    RUNTIME DESTINATION ${BINARY_INSTALL_DIR}
    LIBRARY DESTINATION ${LIBRARY_INSTALL_DIR}
    ARCHIVE DESTINATION ${LIBRARY_INSTALL_DIR}
  )

  install(
    FILES gvmap.sh
    DESTINATION ${BINARY_INSTALL_DIR}
    PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE
      WORLD_READ WORLD_EXECUTE
  )

  if(GZIP)
    add_custom_target(man-gvmap-sh ALL
      DEPENDS gvmap.sh.1.gz
      COMMENT "gvmap.sh man page"
    )
    add_custom_command(
      OUTPUT gvmap.sh.1.gz
      COMMAND ${GZIP} -9 --no-name --to-stdout gvmap.sh.1
        >"${CMAKE_CURRENT_BINARY_DIR}/gvmap.sh.1.gz"
      MAIN_DEPENDENCY gvmap.sh.1
      WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
      COMMENT "compress gvmap.sh man page"
    )
    install(
      FILES ${CMAKE_CURRENT_BINARY_DIR}/gvmap.sh.1.gz
      DESTINATION ${MAN_INSTALL_DIR}/man1
    )
  else()
    install(
      FILES gvmap.sh.1
      DESTINATION ${MAN_INSTALL_DIR}/man1
    )
  endif()
endif()
