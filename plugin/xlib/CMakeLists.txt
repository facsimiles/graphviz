if(PANGOCAIRO_FOUND AND X11_FOUND AND XRENDER_FOUND)

  add_library(gvplugin_xlib
    gvdevice_xlib.c
    gvplugin_xlib.c
  )

  target_include_directories(gvplugin_xlib PRIVATE
    ../../lib
    ../../lib/cdt
    ../../lib/cgraph
    ../../lib/common
    ../../lib/gvc
    ../../lib/pathplan
  )

  target_include_directories(gvplugin_xlib PRIVATE SYSTEM
    ${PANGOCAIRO_INCLUDE_DIRS}
    ${X11_INCLUDE_DIRS}
    ${XRENDER_INCLUDE_DIRS}
  )

  target_link_libraries(gvplugin_xlib PRIVATE
    ${PANGOCAIRO_LINK_LIBRARIES}
    ${X11_LINK_LIBRARIES}
    ${XRENDER_LINK_LIBRARIES}
  )

  if(BUILD_SHARED_LIBS)
    install(
      TARGETS gvplugin_xlib
      RUNTIME DESTINATION ${BINARY_INSTALL_DIR}
      LIBRARY DESTINATION ${PLUGIN_INSTALL_DIR}
      ARCHIVE DESTINATION ${LIBRARY_INSTALL_DIR}
    )
  endif()

  set_target_properties(gvplugin_xlib PROPERTIES
    VERSION ${GVPLUGIN_CURRENT}.0.0
    SOVERSION ${GVPLUGIN_CURRENT}
  )

  if(MINGW)
    # work around https://gitlab.kitware.com/cmake/cmake/-/issues/21716
    set_target_properties(gvplugin_xlib PROPERTIES
      RUNTIME_OUTPUT_NAME gvplugin_xlib-${GVPLUGIN_CURRENT}
    )
  endif()

  install(
    FILES vimdot.sh
    DESTINATION ${BINARY_INSTALL_DIR}
    PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE
      WORLD_READ WORLD_EXECUTE
    RENAME vimdot
  )

  if(GZIP)
    add_custom_target(man-vimdot ALL DEPENDS vimdot.1.gz
                      COMMENT "vimdot man page")
    add_custom_command(
      OUTPUT vimdot.1.gz
      COMMAND ${GZIP} -9 --no-name --to-stdout vimdot.1
        >"${CMAKE_CURRENT_BINARY_DIR}/vimdot.1.gz"
      MAIN_DEPENDENCY vimdot.1
      WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
      COMMENT "compress vimdot man page")
    install(
      FILES ${CMAKE_CURRENT_BINARY_DIR}/vimdot.1.gz
      DESTINATION ${MAN_INSTALL_DIR}/man1)
  else()
    install(
      FILES vimdot.1
      DESTINATION ${MAN_INSTALL_DIR}/man1
    )
  endif()

endif()
