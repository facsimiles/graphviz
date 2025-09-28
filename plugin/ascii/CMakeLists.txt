if(AA_FOUND)
  add_library(gvplugin_ascii gvplugin_ascii.c)

  target_include_directories(gvplugin_ascii PRIVATE
    ../../lib
    ../../lib/cdt
    ../../lib/cgraph
    ../../lib/common
    ../../lib/gvc
    ../../lib/pathplan
  )

  target_include_directories(gvplugin_ascii SYSTEM PRIVATE ${AA_INCLUDE_DIRS})

  target_link_libraries(gvplugin_ascii PRIVATE cgraph gvc xdot ${AA_LIBRARIES})

  if(BUILD_SHARED_LIBS)
    # Installation location of library files
    install(
      TARGETS gvplugin_ascii
      RUNTIME DESTINATION ${BINARY_INSTALL_DIR}
      LIBRARY DESTINATION ${PLUGIN_INSTALL_DIR}
      ARCHIVE DESTINATION ${LIBRARY_INSTALL_DIR}
    )
  endif()

  # Specify library version and soversion
  set_target_properties(gvplugin_ascii PROPERTIES
    VERSION ${GVPLUGIN_CURRENT}.0.0
    SOVERSION ${GVPLUGIN_CURRENT}
  )

  if(MINGW)
    # work around https://gitlab.kitware.com/cmake/cmake/-/issues/21716
    set_target_properties(gvplugin_ascii PROPERTIES
      RUNTIME_OUTPUT_NAME gvplugin_ascii-${GVPLUGIN_CURRENT}
    )
  endif()
endif()
