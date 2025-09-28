add_library(gvplugin_neato_layout
  # Source files
  gvplugin_neato_layout.c
  gvlayout_neato_layout.c
)

target_include_directories(gvplugin_neato_layout PRIVATE
  ../../lib
  ${CMAKE_CURRENT_SOURCE_DIR}
  ../../lib/cdt
  ../../lib/cgraph
  ../../lib/common
  ../../lib/gvc
  ../../lib/pathplan
)

target_link_libraries(gvplugin_neato_layout PRIVATE
  circogen
  fdpgen
  gvc
  neatogen
  osage
  patchwork
  rbtree
  sparse
  twopigen
)

if(with_ipsepcola)
  target_link_libraries(gvplugin_neato_layout PRIVATE vpsc)
endif()

if(with_sfdp)
  target_link_libraries(gvplugin_neato_layout PRIVATE sfdpgen)
endif()

if(BUILD_SHARED_LIBS)
  # Installation location of library files
  install(
    TARGETS gvplugin_neato_layout
    RUNTIME DESTINATION ${BINARY_INSTALL_DIR}
    LIBRARY DESTINATION ${PLUGIN_INSTALL_DIR}
    ARCHIVE DESTINATION ${LIBRARY_INSTALL_DIR}
  )
endif()

# Specify library version and soversion
set_target_properties(gvplugin_neato_layout PROPERTIES
  VERSION ${GVPLUGIN_CURRENT}.0.0
  SOVERSION ${GVPLUGIN_CURRENT}
)

if(MINGW)
  # work around https://gitlab.kitware.com/cmake/cmake/-/issues/21716
  set_target_properties(gvplugin_neato_layout PROPERTIES
    RUNTIME_OUTPUT_NAME gvplugin_neato_layout-${GVPLUGIN_CURRENT}
  )
endif()
