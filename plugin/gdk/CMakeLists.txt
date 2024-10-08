if(GDK_FOUND AND GDK_PIXBUF_FOUND AND PANGOCAIRO_FOUND)

  add_library(gvplugin_gdk
    gvdevice_gdk.c
    gvloadimage_gdk.c
    gvplugin_gdk.c
  )

  target_include_directories(gvplugin_gdk PRIVATE
    ../../lib
    ../../lib/cdt
    ../../lib/cgraph
    ../../lib/common
    ../../lib/gvc
    ../../lib/pathplan
  )

  target_include_directories(gvplugin_gdk SYSTEM PRIVATE
    ${GDK_INCLUDE_DIRS}
    ${GDK_PIXBUF_INCLUDE_DIRS}
    ${PANGOCAIRO_INCLUDE_DIRS}
  )

  target_link_libraries(gvplugin_gdk PRIVATE
    gvc
    ${GDK_LINK_LIBRARIES}
    ${GDK_PIXBUF_LINK_LIBRARIES}
    ${PANGOCAIRO_LINK_LIBRARIES}
  )

  if(BUILD_SHARED_LIBS)
    install(
      TARGETS gvplugin_gdk
      RUNTIME DESTINATION ${BINARY_INSTALL_DIR}
      LIBRARY DESTINATION ${PLUGIN_INSTALL_DIR}
      ARCHIVE DESTINATION ${LIBRARY_INSTALL_DIR}
    )
  endif()

  set_target_properties(gvplugin_gdk PROPERTIES
    VERSION ${GRAPHVIZ_PLUGIN_VERSION}.0.0
    SOVERSION ${GRAPHVIZ_PLUGIN_VERSION}
  )

  if(MINGW)
    # work around https://gitlab.kitware.com/cmake/cmake/-/issues/21716
    set_target_properties(gvplugin_gdk PROPERTIES
      RUNTIME_OUTPUT_NAME gvplugin_gdk-${GRAPHVIZ_PLUGIN_VERSION}
    )
  endif()

endif()
