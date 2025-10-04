add_library(gvplugin_dot_layout
  # Source files
  gvplugin_dot_layout.c
  gvlayout_dot_layout.c
)

target_include_directories(gvplugin_dot_layout PRIVATE
  ../../lib
  ${CMAKE_CURRENT_SOURCE_DIR}
  ../../lib/cdt
  ../../lib/cgraph
  ../../lib/common
  ../../lib/gvc
  ../../lib/pathplan
)

target_link_libraries(gvplugin_dot_layout PRIVATE
  dotgen
  gvc
)

if(BUILD_SHARED_LIBS)
  # Installation location of library files
  install(
    TARGETS gvplugin_dot_layout
    RUNTIME DESTINATION ${BINARY_INSTALL_DIR}
    LIBRARY DESTINATION ${PLUGIN_INSTALL_DIR}
    ARCHIVE DESTINATION ${LIBRARY_INSTALL_DIR}
  )
endif()

# Specify library version and soversion
set_target_properties(gvplugin_dot_layout PROPERTIES
  VERSION ${GVPLUGIN_CURRENT}.0.${GVPLUGIN_REVISION}
  SOVERSION ${GVPLUGIN_CURRENT}
)

if(MINGW)
  # work around https://gitlab.kitware.com/cmake/cmake/-/issues/21716
  set_target_properties(gvplugin_dot_layout PROPERTIES
    RUNTIME_OUTPUT_NAME gvplugin_dot_layout-${GVPLUGIN_CURRENT}
  )
endif()
