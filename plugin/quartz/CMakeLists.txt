if(IS_DIRECTORY "/System/Library/Frameworks/ApplicationServices.framework")

  add_library(gvplugin_quartz
    gvplugin_quartz.h
    gvdevice_quartz.c
    gvloadimage_quartz.c
    gvplugin_quartz.c
    gvrender_quartz.c
    gvtextlayout_quartz.c
  )

  target_include_directories(gvplugin_quartz PRIVATE
    ../../lib
    ../../lib/cdt
    ../../lib/cgraph
    ../../lib/common
    ../../lib/gvc
    ../../lib/pathplan
  )

  target_link_libraries(gvplugin_quartz PRIVATE
    cgraph
    gvc
  )

  target_link_options(gvplugin_quartz PUBLIC
    -framework ApplicationServices
  )

  if(BUILD_SHARED_LIBS)
    install(
      TARGETS gvplugin_quartz
      RUNTIME DESTINATION ${BINARY_INSTALL_DIR}
      LIBRARY DESTINATION ${PLUGIN_INSTALL_DIR}
      ARCHIVE DESTINATION ${LIBRARY_INSTALL_DIR}
    )
  endif()

  set_target_properties(gvplugin_quartz PROPERTIES
    VERSION ${GVPLUGIN_CURRENT}.0.0
    SOVERSION ${GVPLUGIN_CURRENT}
  )

  if(MINGW)
    # work around https://gitlab.kitware.com/cmake/cmake/-/issues/21716
    set_target_properties(gvplugin_quartz PROPERTIES
      RUNTIME_OUTPUT_NAME gvplugin_quartz-${GVPLUGIN_CURRENT}
    )
  endif()

endif()
