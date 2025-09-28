if(Freetype_FOUND AND LASI_FOUND AND PANGOCAIRO_FOUND)

  add_library(gvplugin_lasi
    gvloadimage_lasi.cpp
    gvplugin_lasi.c
    gvrender_lasi.cpp
  )

  target_include_directories(gvplugin_lasi PRIVATE
    ../../lib
    ../../lib/cdt
    ../../lib/cgraph
    ../../lib/common
    ../../lib/gvc
    ../../lib/pathplan
  )

  target_include_directories(gvplugin_lasi SYSTEM PRIVATE
    ${FREETYPE_INCLUDE_DIRS}
    ${LASI_INCLUDE_DIRS}
    ${PANGOCAIRO_INCLUDE_DIRS}
  )

  target_link_libraries(gvplugin_lasi PRIVATE
    ${FREETYPE_LIBRARIES}
    ${LASI_LIBRARIES}
    ${PANGOCAIRO_LINK_LIBRARIES}
  )

  if(BUILD_SHARED_LIBS)
    install(
      TARGETS gvplugin_lasi
      RUNTIME DESTINATION ${BINARY_INSTALL_DIR}
      LIBRARY DESTINATION ${PLUGIN_INSTALL_DIR}
      ARCHIVE DESTINATION ${LIBRARY_INSTALL_DIR}
    )
  endif()

  set_target_properties(gvplugin_lasi PROPERTIES
    VERSION ${GVPLUGIN_CURRENT}.0.${GVPLUGIN_REVISION}
    SOVERSION ${GVPLUGIN_CURRENT}
  )

  if(MINGW)
    # work around https://gitlab.kitware.com/cmake/cmake/-/issues/21716
    set_target_properties(gvplugin_lasi PROPERTIES
      RUNTIME_OUTPUT_NAME gvplugin_lasi-${GVPLUGIN_CURRENT}
    )
  endif()

endif()
