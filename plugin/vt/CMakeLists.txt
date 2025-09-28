add_library(gvplugin_vt gvplugin_vt.c)

target_include_directories(gvplugin_vt PRIVATE
  ../../lib
  ../../lib/cdt
  ../../lib/cgraph
  ../../lib/common
  ../../lib/gvc
  ../../lib/pathplan
)

target_link_libraries(gvplugin_vt PRIVATE cgraph gvc xdot)

if(BUILD_SHARED_LIBS)
  # Installation location of library files
  install(
    TARGETS gvplugin_vt
    RUNTIME DESTINATION ${BINARY_INSTALL_DIR}
    LIBRARY DESTINATION ${PLUGIN_INSTALL_DIR}
    ARCHIVE DESTINATION ${LIBRARY_INSTALL_DIR}
  )
endif()

# Specify library version and soversion
set_target_properties(gvplugin_vt PROPERTIES
  VERSION ${GVPLUGIN_CURRENT}.0.0
  SOVERSION ${GVPLUGIN_CURRENT}
)

if(MINGW)
  # work around https://gitlab.kitware.com/cmake/cmake/-/issues/21716
  set_target_properties(gvplugin_vt PROPERTIES
    RUNTIME_OUTPUT_NAME gvplugin_vt-${GVPLUGIN_CURRENT}
  )
endif()
