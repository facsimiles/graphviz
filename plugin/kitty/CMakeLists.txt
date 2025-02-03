if(PANGOCAIRO_FOUND)
  add_library(gvplugin_kitty gvdevice_kitty.c gvplugin_kitty.c)

  target_include_directories(gvplugin_kitty PRIVATE
    ../../lib
    ../../lib/cdt
    ../../lib/cgraph
    ../../lib/common
    ../../lib/gvc
    ../../lib/pathplan
  )

  target_link_libraries(gvplugin_kitty PRIVATE util)

  if(ZLIB_FOUND)
    target_include_directories(gvplugin_kitty SYSTEM PRIVATE
      ${ZLIB_INCLUDE_DIRS}
    )
    target_link_libraries(gvplugin_kitty PUBLIC
      ${ZLIB_LIBRARIES}
    )
  endif()

  if(BUILD_SHARED_LIBS)
    install(
      TARGETS gvplugin_kitty
      RUNTIME DESTINATION ${BINARY_INSTALL_DIR}
      LIBRARY DESTINATION ${PLUGIN_INSTALL_DIR}
      ARCHIVE DESTINATION ${LIBRARY_INSTALL_DIR}
    )
  endif()

  set_target_properties(gvplugin_kitty PROPERTIES
    VERSION ${GRAPHVIZ_PLUGIN_VERSION}.0.0
    SOVERSION ${GRAPHVIZ_PLUGIN_VERSION}
  )

  if(MINGW)
    # work around https://gitlab.kitware.com/cmake/cmake/-/issues/21716
    set_target_properties(gvplugin_kitty PROPERTIES
      RUNTIME_OUTPUT_NAME gvplugin_kitty-${GRAPHVIZ_PLUGIN_VERSION}
    )
  endif()
endif()
