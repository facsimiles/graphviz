if(CAIRO_FOUND AND PANGOCAIRO_FOUND)

  add_library(gvplugin_pango
    # Header files
    gvgetfontlist.h
    gvplugin_pango.h

    # Source files
    gvgetfontlist_pango.c
    gvloadimage_pango.c
    gvplugin_pango.c
    gvrender_pango.c
    gvtextlayout_pango.c
  )

  target_include_directories(gvplugin_pango PRIVATE
    ../../lib
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_BINARY_DIR}/lib/common
    ../../lib/cdt
    ../../lib/cgraph
    ../../lib/common
    ../../lib/gvc
    ../../lib/pathplan
  )

  target_include_directories(gvplugin_pango SYSTEM PRIVATE
    ${CAIRO_INCLUDE_DIRS}
    ${PANGOCAIRO_INCLUDE_DIRS}
  )

  target_link_libraries(gvplugin_pango PRIVATE
    cgraph
    gvc
    util
    ${CAIRO_LINK_LIBRARIES}
    ${PANGOCAIRO_LINK_LIBRARIES}
  )

  if(BUILD_SHARED_LIBS)
    # Installation location of library files
    install(
      TARGETS gvplugin_pango
      RUNTIME DESTINATION ${BINARY_INSTALL_DIR}
      LIBRARY DESTINATION ${PLUGIN_INSTALL_DIR}
      ARCHIVE DESTINATION ${LIBRARY_INSTALL_DIR}
    )
  endif()

  # Include DLLs with this library on Windows
  if(WIN32 AND install_win_dependency_dlls)
    install(
      FILES
          ${CAIRO_RUNTIME_LIBRARIES}
          ${PANGOCAIRO_RUNTIME_LIBRARIES}
      DESTINATION ${BINARY_INSTALL_DIR}
    )
  endif()

  # Specify library version and soversion
  set_target_properties(gvplugin_pango PROPERTIES
    VERSION ${GRAPHVIZ_PLUGIN_VERSION}.0.0
    SOVERSION ${GRAPHVIZ_PLUGIN_VERSION}
  )

  if(MINGW)
    # work around https://gitlab.kitware.com/cmake/cmake/-/issues/21716
    set_target_properties(gvplugin_pango PROPERTIES
      RUNTIME_OUTPUT_NAME gvplugin_pango-${GRAPHVIZ_PLUGIN_VERSION}
    )
  endif()

endif()
