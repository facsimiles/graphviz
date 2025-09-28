if(GD_FOUND)

  add_library(gvplugin_gd
    # Source files
    gdioctx_wrapper.h
    gdgen_text.h
    gd_psfontResolve.h
    gvdevice_gd.c
    gvloadimage_gd.c
    gvplugin_gd.c
    gvrender_gd.c
    gvrender_gd_vrml.c
    gvtextlayout_gd.c
  )

  target_include_directories(gvplugin_gd PRIVATE
    ../../lib
    ${CMAKE_CURRENT_SOURCE_DIR}
    ../../lib/cdt
    ../../lib/cgraph
    ../../lib/common
    ../../lib/gvc
    ../../lib/pathplan
  )

  target_include_directories(gvplugin_gd SYSTEM PRIVATE
    ${GD_INCLUDE_DIRS}
  )

  target_link_libraries(gvplugin_gd PRIVATE
    cgraph
    gvc
    pathplan
    ${GD_LIBRARIES}
  )

  if(CAIRO_FOUND)
    target_include_directories(gvplugin_gd SYSTEM PRIVATE
      ${CAIRO_INCLUDE_DIRS}
    )
    target_link_libraries(gvplugin_gd PRIVATE
      ${CAIRO_LINK_LIBRARIES}
    )
    if(PANGOCAIRO_FOUND)
      target_include_directories(gvplugin_gd SYSTEM PRIVATE
        ${PANGOCAIRO_INCLUDE_DIRS}
      )

      target_link_libraries(gvplugin_gd PRIVATE
        ${PANGOCAIRO_LINK_LIBRARIES}
      )
    endif()
  endif()

  if(BUILD_SHARED_LIBS)
    # Installation location of library files
    install(
      TARGETS gvplugin_gd
      RUNTIME DESTINATION ${BINARY_INSTALL_DIR}
      LIBRARY DESTINATION ${PLUGIN_INSTALL_DIR}
      ARCHIVE DESTINATION ${LIBRARY_INSTALL_DIR}
    )
  endif()

  # Include DLLs with this library on Windows
  if(WIN32 AND install_win_dependency_dlls)
    install(
      FILES
        ${GD_RUNTIME_LIBRARIES}
      DESTINATION ${BINARY_INSTALL_DIR}
    )
  endif()

  # Specify library version and soversion
  set_target_properties(gvplugin_gd PROPERTIES
    VERSION ${GVPLUGIN_CURRENT}.0.${GVPLUGIN_REVISION}
    SOVERSION ${GVPLUGIN_CURRENT}
  )

  if(MINGW)
    # work around https://gitlab.kitware.com/cmake/cmake/-/issues/21716
    set_target_properties(gvplugin_gd PROPERTIES
      RUNTIME_OUTPUT_NAME gvplugin_gd-${GVPLUGIN_CURRENT}
    )
  endif()

endif()
