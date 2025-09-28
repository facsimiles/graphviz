if(PANGOCAIRO_FOUND AND DevIL_FOUND)

  add_library(gvplugin_devil
    gvdevice_devil.c
    gvplugin_devil.c
  )

  target_include_directories(gvplugin_devil PRIVATE
    ../../lib
    ../../lib/cdt
    ../../lib/cgraph
    ../../lib/common
    ../../lib/gvc
    ../../lib/pathplan
  )

  target_include_directories(gvplugin_devil SYSTEM PRIVATE
    ${PANGOCAIRO_INCLUDE_DIRS}
    ${IL_INCLUDE_DIR}/..
  )

  target_link_libraries(gvplugin_devil PRIVATE
    ${PANGOCAIRO_LINK_LIBRARIES}
    ${IL_LIBRARIES}
    ${ILU_LIBRARIES}
  )

  if(BUILD_SHARED_LIBS)
    install(
      TARGETS gvplugin_devil
      RUNTIME DESTINATION ${BINARY_INSTALL_DIR}
      LIBRARY DESTINATION ${PLUGIN_INSTALL_DIR}
      ARCHIVE DESTINATION ${LIBRARY_INSTALL_DIR}
    )
  endif()

  set_target_properties(gvplugin_devil PROPERTIES
    VERSION ${GVPLUGIN_CURRENT}.0.0
    SOVERSION ${GVPLUGIN_CURRENT}
  )

  if(MINGW)
    # work around https://gitlab.kitware.com/cmake/cmake/-/issues/21716
    set_target_properties(gvplugin_devil PROPERTIES
      RUNTIME_OUTPUT_NAME gvplugin_devil-${GVPLUGIN_CURRENT}
    )
  endif()

endif()
