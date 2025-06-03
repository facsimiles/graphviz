# This function sets some default values that every tools shares:
# - linking to getopt if REQUIRED
# - installation of executable
# - installation of manpage
function(tool_defaults name)
  if(NOT HAVE_GETOPT_H)
    target_link_libraries(${name} PRIVATE ${GETOPT_LINK_LIBRARIES})
  endif()

  install(
    TARGETS ${name}
    RUNTIME DESTINATION ${BINARY_INSTALL_DIR}
    LIBRARY DESTINATION ${LIBRARY_INSTALL_DIR}
    ARCHIVE DESTINATION ${LIBRARY_INSTALL_DIR}
  )

  if(GZIP)
    add_custom_target(man-${name} ALL DEPENDS ${name}.1.gz
                      COMMENT "${name} man page")
    add_custom_command(
      OUTPUT ${name}.1.gz
      COMMAND ${GZIP} -9 --no-name --to-stdout ${name}.1
        >"${CMAKE_CURRENT_BINARY_DIR}/${name}.1.gz"
      MAIN_DEPENDENCY ${name}.1
      WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
      COMMENT "compress ${name} man page")
    install(
      FILES ${CMAKE_CURRENT_BINARY_DIR}/${name}.1.gz
      DESTINATION ${MAN_INSTALL_DIR}/man1)
  else()
    install(
      FILES ${name}.1
      DESTINATION ${MAN_INSTALL_DIR}/man1
    )
  endif()
endfunction(tool_defaults)

# ================================ simple tools ================================
# These tools have a single source file and share linked libraries, etc.

# This function adds a simple tool using only its name
function(add_simple_tool name)
  add_executable(${name} ${name}.c)

  target_include_directories(${name} PRIVATE
    ../../lib
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
    ../../lib/cdt
    ../../lib/cgraph
    ../../lib/common
    ../../lib/gvc
    ../../lib/pack
    ../../lib/pathplan
  )

  if(GETOPT_FOUND)
    target_include_directories(${name} SYSTEM PRIVATE
      ${GETOPT_INCLUDE_DIRS}
    )
  endif()

  target_link_libraries(${name} PRIVATE
    cgraph
  )

  tool_defaults(${name})
endfunction(add_simple_tool)

add_simple_tool(acyclic)
add_simple_tool(bcomps)
add_simple_tool(ccomps)
target_link_libraries(ccomps PRIVATE gvc)
add_simple_tool(nop)
add_simple_tool(tred)
target_link_libraries(tred PRIVATE gvc) # e.g. for start_timer
add_simple_tool(unflatten)

# ================================ complex tools ===============================
# These tools have multiple source files, different dependencies or other
# differences from the simple tools above

# ================================== gc ========================================
add_executable(gc_bin gc.c)
set_target_properties(gc_bin PROPERTIES OUTPUT_NAME "gc")

target_include_directories(gc_bin PRIVATE
  ../../lib
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
  ../../lib/cdt
  ../../lib/cgraph
  ../../lib/common
  ../../lib/gvc
  ../../lib/pack
  ../../lib/pathplan
)

if(GETOPT_FOUND)
  target_include_directories(gc_bin SYSTEM PRIVATE ${GETOPT_INCLUDE_DIRS})
endif()
if(NOT HAVE_GETOPT_H)
  target_link_libraries(gc_bin PRIVATE ${GETOPT_LINK_LIBRARIES})
endif()

target_link_libraries(gc_bin PRIVATE
  cgraph
  gvc
)

install(
  TARGETS gc_bin
  RUNTIME DESTINATION ${BINARY_INSTALL_DIR}
  LIBRARY DESTINATION ${LIBRARY_INSTALL_DIR}
  ARCHIVE DESTINATION ${LIBRARY_INSTALL_DIR}
)

if(GZIP)
  add_custom_target(man-gc ALL DEPENDS gc.1.gz
                    COMMENT "gc man page")
  add_custom_command(
    OUTPUT gc.1.gz
    COMMAND ${GZIP} -9 --no-name --to-stdout gc.1
      >"${CMAKE_CURRENT_BINARY_DIR}/gc.1.gz"
    MAIN_DEPENDENCY gc.1
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMENT "compress gc man page")
  install(
    FILES ${CMAKE_CURRENT_BINARY_DIR}/gc.1.gz
    DESTINATION ${MAN_INSTALL_DIR}/man1)
else()
  install(
    FILES gc.1
    DESTINATION ${MAN_INSTALL_DIR}/man1
  )
endif()

# ================================== dijkstra ==================================
add_executable(dijkstra
  # Source files
  dijkstra.c
)

target_include_directories(dijkstra PRIVATE
  ../../lib
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
  ../../lib/cdt
  ../../lib/cgraph
  ../../lib/common
  ../../lib/gvc
  ../../lib/pack
  ../../lib/pathplan
)

if(GETOPT_FOUND)
  target_include_directories(dijkstra SYSTEM PRIVATE
    ${GETOPT_INCLUDE_DIRS}
  )
endif()

target_link_libraries(dijkstra PRIVATE
  cgraph
)

tool_defaults(dijkstra)

# =================================== gml2gv ===================================
BISON_TARGET(Gmlparse ${CMAKE_CURRENT_SOURCE_DIR}/gmlparse.y
                      ${CMAKE_CURRENT_BINARY_DIR}/gmlparse.c)
FLEX_TARGET(Gmlscan
  ${CMAKE_CURRENT_SOURCE_DIR}/gmlscan.l
  ${CMAKE_CURRENT_BINARY_DIR}/gmlscan.c
  COMPILE_FLAGS --case-insensitive
)
ADD_FLEX_BISON_DEPENDENCY(Gmlscan Gmlparse)

add_executable(gml2gv
  # Source files
  gml2gv.c

  # Generated files
  ${BISON_Gmlparse_OUTPUTS}
  ${FLEX_Gmlscan_OUTPUTS}
)

if(WIN32 AND NOT MINGW)
  target_compile_definitions(gml2gv PRIVATE -DYY_NO_UNISTD_H)
endif()

target_include_directories(gml2gv PRIVATE
  ../../lib
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
  ../../lib/cdt
  ../../lib/cgraph
  ../../lib/common
  ../../lib/gvc
  ../../lib/pack
  ../../lib/pathplan
)

if(GETOPT_FOUND)
  target_include_directories(gml2gv SYSTEM PRIVATE
    ${GETOPT_INCLUDE_DIRS}
  )
endif()

target_link_libraries(gml2gv PRIVATE
  cgraph
)

tool_defaults(gml2gv)

add_executable(gv2gml gv2gml.c)
target_include_directories(gv2gml PRIVATE
  ../../lib
  ../../lib/cdt
  ../../lib/cgraph
  ../../lib/common
  ../../lib/gvc
  ../../lib/pathplan
)
if(GETOPT_FOUND)
  target_include_directories(gv2gml SYSTEM PRIVATE
    ${GETOPT_INCLUDE_DIRS}
  )
endif()
target_link_libraries(gv2gml PRIVATE cgraph gvc util)
if(NOT HAVE_GETOPT_H)
  target_link_libraries(gv2gml PRIVATE ${GETOPT_LINK_LIBRARIES})
endif()
install(TARGETS gv2gml RUNTIME DESTINATION ${BINARY_INSTALL_DIR})

# ================================= graphml2gv =================================
if(EXPAT_FOUND)

  add_executable(graphml2gv
    # Source files
    graphml2gv.c
  )

  target_include_directories(graphml2gv PRIVATE
    ../../lib
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
    ../../lib/cdt
    ../../lib/cgraph
    ../../lib/common
    ../../lib/gvc
    ../../lib/pack
    ../../lib/pathplan
  )

  target_include_directories(graphml2gv SYSTEM PRIVATE
    ${EXPAT_INCLUDE_DIRS}
  )

  if(GETOPT_FOUND)
    target_include_directories(graphml2gv SYSTEM PRIVATE
      ${GETOPT_INCLUDE_DIRS}
    )
  endif()

  target_link_libraries(graphml2gv PRIVATE
    cgraph
    ${EXPAT_LIBRARIES}
  )

  tool_defaults(graphml2gv)

endif()

# ================================== gvcolor ===================================
add_executable(gvcolor
  # Header files
  colortbl.h

  # Source files
  colxlate.c
  gvcolor.c
)

target_include_directories(gvcolor PRIVATE
  ../../lib
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
  ../../lib/cdt
  ../../lib/cgraph
  ../../lib/common
  ../../lib/gvc
  ../../lib/pack
  ../../lib/pathplan
)

if(GETOPT_FOUND)
  target_include_directories(gvcolor SYSTEM PRIVATE
    ${GETOPT_INCLUDE_DIRS}
  )
endif()

target_link_libraries(gvcolor PRIVATE
  cgraph
)

tool_defaults(gvcolor)

# ==================================== gvgen ===================================
add_executable(gvgen
  # Source files
  graph_generator.c
  gvgen.c
)

target_include_directories(gvgen PRIVATE
  ../../lib
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
  ../../lib/cdt
  ../../lib/cgraph
  ../../lib/common
  ../../lib/gvc
  ../../lib/pack
  ../../lib/pathplan
)

if (GETOPT_FOUND)
  target_include_directories(gvgen SYSTEM PRIVATE
    ${GETOPT_INCLUDE_DIRS}
  )
endif()

target_link_libraries(gvgen PRIVATE cgraph util)

tool_defaults(gvgen)

# =================================== gvpack ===================================
# TODO add gvpack_static

add_executable(gvpack
  # Source files
  gvpack.cpp
)

target_compile_definitions(gvpack PRIVATE DEMAND_LOADING=0)

target_include_directories(gvpack PRIVATE
  ../../lib
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
  ../../lib/cdt
  ../../lib/cgraph
  ../../lib/common
  ../../lib/gvc
  ../../lib/pack
  ../../lib/pathplan
)

if(GETOPT_FOUND)
  target_include_directories(gvpack SYSTEM PRIVATE
    ${GETOPT_INCLUDE_DIRS}
  )
endif()

target_link_libraries(gvpack PRIVATE
  cgraph
  gvc
)
if(BUILD_SHARED_LIBS)
  target_link_libraries(gvpack PRIVATE gvplugin_neato_layout)
else()
  target_link_libraries(gvpack PRIVATE
    $<TARGET_OBJECTS:gvplugin_neato_layout>
    $<TARGET_PROPERTY:gvplugin_neato_layout,LINK_LIBRARIES>
  )
endif()

tool_defaults(gvpack)

# =================================== gxl2gv ===================================
if(EXPAT_FOUND)

  add_executable(gxl2gv
    # Source files
    cvtgxl.c
    gv2gxl.c
    gxl2gv.c
  )

  target_include_directories(gxl2gv PRIVATE
    ../../lib
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
    ../../lib/cdt
    ../../lib/cgraph
    ../../lib/common
    ../../lib/gvc
    ../../lib/pack
    ../../lib/pathplan
  )

  target_include_directories(gxl2gv SYSTEM PRIVATE
    ${EXPAT_INCLUDE_DIRS}
  )
  if(GETOPT_FOUND)
    target_include_directories(gxl2gv SYSTEM PRIVATE
      ${GETOPT_INCLUDE_DIRS}
    )
  endif()

  target_link_libraries(gxl2gv PRIVATE
    cgraph
    util
    ${EXPAT_LIBRARIES}
  )

  tool_defaults(gxl2gv)

  list(APPEND gxl2gv_aliases dot2gxl gv2gxl gxl2dot)
  foreach(cmd_alias IN LISTS gxl2gv_aliases)
    set(GXL2GV_COPY
        "${CMAKE_CURRENT_BINARY_DIR}/${cmd_alias}${CMAKE_EXECUTABLE_SUFFIX}")
    if(WIN32 OR CYGWIN)
      # copy instead of symlink to avoid
      # https://gitlab.com/graphviz/graphviz/-/issues/2123
      add_custom_command(
        TARGET gxl2gv
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:gxl2gv> ${GXL2GV_COPY}
        COMMENT "Copying gxl2gv to ${GXL2GV_COPY}"
      )
      install(
        PROGRAMS ${GXL2GV_COPY}
        DESTINATION ${BINARY_INSTALL_DIR}
      )
    else()
      add_custom_command(
        TARGET gxl2gv
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E create_symlink $<TARGET_FILE_NAME:gxl2gv>
          ${cmd_alias}
        COMMENT "Linking gxl2gv as ${cmd_alias}")
      install(
        FILES ${GXL2GV_COPY}
        DESTINATION ${BINARY_INSTALL_DIR}
      )
    endif()
  endforeach()

endif()

# ==================================== mm2gv ===================================
add_executable(mm2gv
  # Source files
  matrix_market.c
  mm2gv.c
  mmio.c
)

target_include_directories(mm2gv PRIVATE
  ../../lib
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
  ../../lib/cdt
  ../../lib/cgraph
  ../../lib/common
  ../../lib/gvc
  ../../lib/pack
  ../../lib/pathplan
)

if(GETOPT_FOUND)
  target_include_directories(mm2gv SYSTEM PRIVATE
    ${GETOPT_INCLUDE_DIRS}
  )
endif()

target_link_libraries(mm2gv PRIVATE
  sparse
  cgraph
  gvc
)

tool_defaults(mm2gv)

# =================================== sccmap ===================================
add_executable(sccmap
  # Source files
  sccmap.c
)

target_include_directories(sccmap PRIVATE
  ../../lib
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
  ../../lib/cdt
  ../../lib/cgraph
  ../../lib/common
  ../../lib/gvc
  ../../lib/pack
  ../../lib/pathplan
)

if(GETOPT_FOUND)
  target_include_directories(sccmap SYSTEM PRIVATE
    ${GETOPT_INCLUDE_DIRS}
  )
endif()

target_link_libraries(sccmap PRIVATE
  cgraph
)

tool_defaults(sccmap)

# ===================== Install third party DLLs on Windows ====================

if(WIN32 AND NOT MINGW AND install_win_dependency_dlls AND EXPAT_FOUND)
  install(
    FILES ${EXPAT_RUNTIME_LIBRARIES}
    DESTINATION ${BINARY_INSTALL_DIR}
  )
endif()

if(WIN32 AND NOT MINGW AND install_win_dependency_dlls)
  install(
    FILES ${GETOPT_RUNTIME_LIBRARIES}
    DESTINATION ${BINARY_INSTALL_DIR}
  )
endif()
