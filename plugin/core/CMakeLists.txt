add_library(gvplugin_core
  # Header files
  core_loadimage_xdot.h
  ps.h
  tcl_context.h

  # Source files
  gvloadimage_core.c
  gvplugin_core.c
  gvrender_core_dot.c
  gvrender_core_json.c
  gvrender_core_fig.c
  gvrender_core_map.c
  gvrender_core_pic.c
  gvrender_core_pov.c
  gvrender_core_ps.c
  gvrender_core_svg.c
  gvrender_core_tk.c
)

target_include_directories(gvplugin_core PRIVATE
  ../../lib
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
  ../../lib/cdt
  ../../lib/cgraph
  ../../lib/common
  ../../lib/gvc
  ../../lib/gvpr
  ../../lib/pathplan
)

target_link_libraries(gvplugin_core PRIVATE
  cgraph
  gvc
  util
  xdot
)

if(BUILD_SHARED_LIBS)
  # Installation location of library files
  install(
    TARGETS gvplugin_core
    RUNTIME DESTINATION ${BINARY_INSTALL_DIR}
    LIBRARY DESTINATION ${PLUGIN_INSTALL_DIR}
    ARCHIVE DESTINATION ${LIBRARY_INSTALL_DIR}
  )
endif()

# Specify library version and soversion
set_target_properties(gvplugin_core PROPERTIES
  VERSION ${GRAPHVIZ_PLUGIN_VERSION}.0.0
  SOVERSION ${GRAPHVIZ_PLUGIN_VERSION}
)

if(MINGW)
  # work around https://gitlab.kitware.com/cmake/cmake/-/issues/21716
  set_target_properties(gvplugin_core PROPERTIES
    RUNTIME_OUTPUT_NAME gvplugin_core-${GRAPHVIZ_PLUGIN_VERSION}
  )
endif()
