if(WEBP_FOUND)

  add_library(gvplugin_webp
    gvplugin_webp.c
    gvloadimage_webp.c
    gvdevice_webp.c
  )

  target_include_directories(gvplugin_webp PRIVATE
    ../../lib
    ../../lib/cdt
    ../../lib/cgraph
    ../../lib/common
    ../../lib/gvc
    ../../lib/pathplan
  )

  target_include_directories(gvplugin_webp SYSTEM PRIVATE
    ${WEBP_INCLUDE_DIRS}
  )

  target_link_libraries(gvplugin_webp PRIVATE
    gvc
    ${WEBP_LINK_LIBRARIES}
  )

  if(CAIRO_FOUND)
    target_include_directories(gvplugin_webp SYSTEM PRIVATE
      ${CAIRO_INCLUDE_DIRS}
    )
    target_link_libraries(gvplugin_webp PRIVATE
      ${CAIRO_LINK_LIBRARIES}
    )
    if(PANGOCAIRO_FOUND)
      target_include_directories(gvplugin_webp SYSTEM PRIVATE
        ${PANGOCAIRO_INCLUDE_DIRS}
      )

      target_link_libraries(gvplugin_webp PRIVATE
        ${PANGOCAIRO_LINK_LIBRARIES}
      )
    endif()
  endif()

  if(BUILD_SHARED_LIBS)
    install(
      TARGETS gvplugin_webp
      RUNTIME DESTINATION ${BINARY_INSTALL_DIR}
      LIBRARY DESTINATION ${PLUGIN_INSTALL_DIR}
      ARCHIVE DESTINATION ${LIBRARY_INSTALL_DIR}
    )
  endif()

  if(WIN32 AND install_win_dependency_dlls)
    find_program(WEBP_RUNTIME_LIBRARY NAMES libwebp.dll)
    find_program(SHARPYUV_RUNTIME_LIBRARY NAMES libsharpyuv.dll)

    install(
      FILES
        ${WEBP_RUNTIME_LIBRARY}
        ${SHARPYUV_RUNTIME_LIBRARY}
      DESTINATION ${BINARY_INSTALL_DIR}
    )
  endif()

  set_target_properties(gvplugin_webp PROPERTIES
    VERSION ${GVPLUGIN_CURRENT}.0.${GVPLUGIN_REVISION}
    SOVERSION ${GVPLUGIN_CURRENT}
  )

  if(MINGW)
    # work around https://gitlab.kitware.com/cmake/cmake/-/issues/21716
    set_target_properties(gvplugin_webp PROPERTIES
      RUNTIME_OUTPUT_NAME gvplugin_webp-${GVPLUGIN_CURRENT}
    )
  endif()

endif()
