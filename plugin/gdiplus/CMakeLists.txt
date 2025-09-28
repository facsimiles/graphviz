if(WIN32)

  add_library(gvplugin_gdiplus
    # Header files
    gvplugin_gdiplus.h

    # Source files
    gvdevice_gdiplus.cpp
    gvloadimage_gdiplus.cpp
    gvplugin_gdiplus.cpp
    gvrender_gdiplus.cpp
    gvtextlayout_gdiplus.cpp
  )

  target_include_directories(gvplugin_gdiplus PRIVATE
    ../../lib
    ${CMAKE_CURRENT_SOURCE_DIR}
    ../../lib/cdt
    ../../lib/cgraph
    ../../lib/common
    ../../lib/gvc
    ../../lib/pathplan
  )

  target_link_libraries(gvplugin_gdiplus PRIVATE
    gvc
    gdiplus
  )

  if(BUILD_SHARED_LIBS)
    # Installation location of library files
    install(
      TARGETS gvplugin_gdiplus
      RUNTIME DESTINATION ${BINARY_INSTALL_DIR}
      LIBRARY DESTINATION ${PLUGIN_INSTALL_DIR}
      ARCHIVE DESTINATION ${LIBRARY_INSTALL_DIR}
    )
  endif()

  # Specify library version and soversion
  set_target_properties(gvplugin_gdiplus PROPERTIES
    VERSION ${GVPLUGIN_CURRENT}.0.0
    SOVERSION ${GVPLUGIN_CURRENT}
  )

  if(MINGW)
    # work around https://gitlab.kitware.com/cmake/cmake/-/issues/21716
    set_target_properties(gvplugin_gdiplus PROPERTIES
        RUNTIME_OUTPUT_NAME gvplugin_gdiplus-${GVPLUGIN_CURRENT}
    )
  endif()

endif()
