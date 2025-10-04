if(PANGOCAIRO_FOUND AND RSVG_FOUND)

  add_library(gvplugin_rsvg
    gvloadimage_rsvg.c
    gvplugin_rsvg.c
  )

  target_include_directories(gvplugin_rsvg PRIVATE
    ../../lib
    ../../lib/cdt
    ../../lib/cgraph
    ../../lib/common
    ../../lib/gvc
    ../../lib/pathplan
  )

  target_include_directories(gvplugin_rsvg SYSTEM PRIVATE
    ${PANGOCAIRO_INCLUDE_DIRS}
    ${RSVG_INCLUDE_DIRS}
  )

  target_link_libraries(gvplugin_rsvg PRIVATE
    gvc
    ${PANGOCAIRO_LINK_LIBRARIES}
    ${RSVG_LINK_LIBRARIES}
  )

  if(BUILD_SHARED_LIBS)
    install(
      TARGETS gvplugin_rsvg
      RUNTIME DESTINATION ${BINARY_INSTALL_DIR}
      LIBRARY DESTINATION ${PLUGIN_INSTALL_DIR}
      ARCHIVE DESTINATION ${LIBRARY_INSTALL_DIR}
    )
  endif()

  set_target_properties(gvplugin_rsvg PROPERTIES
    VERSION ${GVPLUGIN_CURRENT}.0.${GVPLUGIN_REVISION}
    SOVERSION ${GVPLUGIN_CURRENT}
  )

  if(MINGW)
    # work around https://gitlab.kitware.com/cmake/cmake/-/issues/21716
    set_target_properties(gvplugin_rsvg PROPERTIES
      RUNTIME_OUTPUT_NAME gvplugin_rsvg-${GVPLUGIN_CURRENT}
    )
  endif()

endif()
