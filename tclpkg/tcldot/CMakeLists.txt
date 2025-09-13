set(SOURCES
  tcldot.c
  tcldot.h
  tcldot-graphcmd.c
  tcldot-nodecmd.c
  tcldot-edgecmd.c
  tcldot-util.c
  tcldot-id.c
  tcldot-io.c
)

if(GD_FOUND)
  list(APPEND SOURCES $<TARGET_OBJECTS:gdtclft>)
endif()

add_library(tcldot SHARED ${SOURCES} no_builtins.c)

target_compile_definitions(tcldot PRIVATE -DDEMAND_LOADING=1)

target_link_libraries(tcldot PRIVATE
  tclstubs
  cdt
  cgraph
  pathplan
  gvc
)
target_include_directories(tcldot SYSTEM PRIVATE ${TCL_INCLUDE_PATH})
target_link_libraries(tcldot PRIVATE ${TCL_LIBRARY})
if(GD_FOUND)
  target_include_directories(tcldot SYSTEM PRIVATE ${GD_INCLUDE_DIRS})
  target_link_libraries(tcldot PRIVATE ${GD_LIBRARIES})
endif()

install(
  TARGETS tcldot
  RUNTIME DESTINATION ${BINARY_INSTALL_DIR}
  LIBRARY DESTINATION ${LIBRARY_INSTALL_DIR}/graphviz/tcl
  ARCHIVE DESTINATION ${LIBRARY_INSTALL_DIR}
)

# Include DLLs with this library on Windows
if(WIN32 AND NOT MINGW AND install_win_dependency_dlls)
  install(
    FILES
      ${TCL_RUNTIME_LIBRARIES}
    DESTINATION ${BINARY_INSTALL_DIR}
  )
endif()

add_custom_target(tcldot_pkgindex
  COMMAND
    ${Python3_EXECUTABLE} "${CMAKE_CURRENT_SOURCE_DIR}/../mkpkgindex.py"
    --file libtcldot${CMAKE_SHARED_LIBRARY_SUFFIX}
    --name Tcldot
    --version "${GRAPHVIZ_VERSION}"
  DEPENDS ../mkpkgindex.py
  BYPRODUCTS pkgIndex.tcl
  COMMENT "create tcldot package index"
)
add_dependencies(tcldot_pkgindex tcldot)

if(GZIP)
  add_custom_target(man-tcldot ALL DEPENDS tcldot.3tcl.gz
                    COMMENT "tcldot man page")
  add_custom_command(
    OUTPUT tcldot.3tcl.gz
    COMMAND ${GZIP} -9 --no-name --to-stdout tcldot.3tcl
      >"${CMAKE_CURRENT_BINARY_DIR}/tcldot.3tcl.gz"
    MAIN_DEPENDENCY tcldot.3tcl
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMENT "compress tcldot man page")
  install(
    FILES ${CMAKE_CURRENT_BINARY_DIR}/tcldot.3tcl.gz
    DESTINATION ${MAN_INSTALL_DIR}/man3)
else()
  install(
    FILES tcldot.3tcl
    DESTINATION ${MAN_INSTALL_DIR}/man3
  )
endif()

add_library(tcldot_builtin SHARED ${SOURCES} tcldot_builtins.cpp)

target_compile_definitions(tcldot_builtin PRIVATE -DDEMAND_LOADING=1)

target_link_libraries(tcldot_builtin PRIVATE
  gvplugin_dot_layout
  gvplugin_neato_layout
  gvplugin_core
  gvc
  cgraph
  cdt
  pathplan
  tclstubs
)
target_include_directories(tcldot_builtin SYSTEM PRIVATE ${TCL_INCLUDE_PATH})
target_link_libraries(tcldot_builtin PRIVATE ${TCL_LIBRARY})
if(GD_FOUND)
  target_include_directories(tcldot_builtin SYSTEM PRIVATE ${GD_INCLUDE_DIRS})
  target_link_libraries(tcldot_builtin PRIVATE ${GD_LIBRARIES})
endif()
if(PANGOCAIRO_FOUND)
  target_link_libraries(tcldot_builtin PRIVATE gvplugin_pango)
endif()
if(GD_FOUND)
  target_link_libraries(tcldot_builtin PRIVATE gvplugin_gd)
endif()

install(
  TARGETS tcldot_builtin
  RUNTIME DESTINATION ${BINARY_INSTALL_DIR}
  LIBRARY DESTINATION ${LIBRARY_INSTALL_DIR}/graphviz/tcl
  ARCHIVE DESTINATION ${LIBRARY_INSTALL_DIR}
)
