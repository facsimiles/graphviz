if(PANGOCAIRO_FOUND AND POPPLER_FOUND)

  add_library(gvplugin_poppler
    gvloadimage_poppler.c
    gvplugin_poppler.c
  )

  target_include_directories(gvplugin_poppler PRIVATE
    ../../lib
    ../../lib/cdt
    ../../lib/cgraph
    ../../lib/common
    ../../lib/gvc
    ../../lib/pathplan
  )

  target_include_directories(gvplugin_poppler SYSTEM PRIVATE
    ${PANGOCAIRO_INCLUDE_DIRS}
    ${POPPLER_INCLUDE_DIRS}
  )

  target_link_libraries(gvplugin_poppler PRIVATE
    gvc
    ${PANGOCAIRO_LINK_LIBRARIES}
    ${POPPLER_LIBRARIES}
  )

  if(BUILD_SHARED_LIBS)
    install(
      TARGETS gvplugin_poppler
      RUNTIME DESTINATION ${BINARY_INSTALL_DIR}
      LIBRARY DESTINATION ${PLUGIN_INSTALL_DIR}
      ARCHIVE DESTINATION ${LIBRARY_INSTALL_DIR}
    )
  endif()

  set_target_properties(gvplugin_poppler PROPERTIES
    VERSION ${GRAPHVIZ_PLUGIN_VERSION}.0.0
    SOVERSION ${GRAPHVIZ_PLUGIN_VERSION}
  )

  if(MINGW)
    # work around https://gitlab.kitware.com/cmake/cmake/-/issues/21716
    set_target_properties(gvplugin_poppler PROPERTIES
      RUNTIME_OUTPUT_NAME gvplugin_poppler-${GRAPHVIZ_PLUGIN_VERSION}
    )
  endif()

endif()
