# This function sets some default values that every tools shares:
# - linking to getopt if REQUIRED
# - installation of executable
# - installation of manpage
function(tool_defaults name)
    if(NOT HAVE_GETOPT_H)
        target_link_libraries(${name} ${Getopt_LINK_LIBRARIES})
    endif(NOT HAVE_GETOPT_H)

    install(
        TARGETS ${name}
        RUNTIME DESTINATION ${BINARY_INSTALL_DIR}
    )

    if(GZIP)
      add_custom_target(man-${name} ALL DEPENDS ${name}.1.gz)
      add_custom_command(
        OUTPUT ${name}.1.gz
        COMMAND ${GZIP} -9 --no-name --to-stdout ${name}.1
          >"${CMAKE_CURRENT_BINARY_DIR}/${name}.1.gz"
        MAIN_DEPENDENCY ${name}.1
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
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

# This function adds a simple tool using only it's name
function(add_simple_tool name)
    add_executable(${name} ${name}.c)

    target_include_directories(${name} PRIVATE
        ${GRAPHVIZ_LIB_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_BINARY_DIR}
        ${GRAPHVIZ_LIB_DIR}/cdt
        ${GRAPHVIZ_LIB_DIR}/cgraph
        ${GRAPHVIZ_LIB_DIR}/common
        ${GRAPHVIZ_LIB_DIR}/gvc
        ${GRAPHVIZ_LIB_DIR}/pack
        ${GRAPHVIZ_LIB_DIR}/pathplan
        ${Getopt_INCLUDE_DIRS}
    )

    target_link_libraries(${name}
        cgraph
        ingraphs
    )

    tool_defaults(${name})
endfunction(add_simple_tool)

add_simple_tool(acyclic)
add_simple_tool(bcomps)
add_simple_tool(ccomps)
add_simple_tool(gc)
add_simple_tool(nop)
add_simple_tool(tred)
target_link_libraries(tred common) # e.g. for start_timer
add_simple_tool(unflatten)

# ================================ complex tools ===============================
# These tools have multiple source files, different dependencies or other
# differences from the simple tools above

# ================================== dijkstra ==================================
add_executable(dijkstra
    # Source files
    dijkstra.c
)

target_include_directories(dijkstra PRIVATE
    ${GRAPHVIZ_LIB_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
    ${GRAPHVIZ_LIB_DIR}/cdt
    ${GRAPHVIZ_LIB_DIR}/cgraph
    ${GRAPHVIZ_LIB_DIR}/common
    ${GRAPHVIZ_LIB_DIR}/gvc
    ${GRAPHVIZ_LIB_DIR}/pack
    ${GRAPHVIZ_LIB_DIR}/pathplan
    ${Getopt_INCLUDE_DIRS}
)

target_link_libraries(dijkstra
    cgraph
    ingraphs
)

tool_defaults(dijkstra)

# =================================== gml2gv ===================================
BISON_TARGET(Gmlparse ${CMAKE_CURRENT_SOURCE_DIR}/gmlparse.y ${CMAKE_CURRENT_BINARY_DIR}/gmlparse.c)
FLEX_TARGET(Gmlscan ${CMAKE_CURRENT_SOURCE_DIR}/gmlscan.l  ${CMAKE_CURRENT_BINARY_DIR}/gmlscan.c)
ADD_FLEX_BISON_DEPENDENCY(Gmlscan Gmlparse)

add_executable(gml2gv
    # Source files
    gml2gv.c

    # Generated files
    ${BISON_Gmlparse_OUTPUTS}
    ${FLEX_Gmlscan_OUTPUTS}
)

target_include_directories(gml2gv PRIVATE
    ${GRAPHVIZ_LIB_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
    ${GRAPHVIZ_LIB_DIR}/cdt
    ${GRAPHVIZ_LIB_DIR}/cgraph
    ${GRAPHVIZ_LIB_DIR}/common
    ${GRAPHVIZ_LIB_DIR}/gvc
    ${GRAPHVIZ_LIB_DIR}/pack
    ${GRAPHVIZ_LIB_DIR}/pathplan
    ${Getopt_INCLUDE_DIRS}
)

target_link_libraries(gml2gv
    cgraph
)

tool_defaults(gml2gv)

add_executable(gv2gml gv2gml.c)
target_include_directories(gv2gml PRIVATE
    ${GRAPHVIZ_LIB_DIR}
    ${GRAPHVIZ_LIB_DIR}/cdt
    ${GRAPHVIZ_LIB_DIR}/cgraph
    ${GRAPHVIZ_LIB_DIR}/common
    ${GRAPHVIZ_LIB_DIR}/gvc
    ${GRAPHVIZ_LIB_DIR}/pathplan
    ${Getopt_INCLUDE_DIRS})
target_link_libraries(gv2gml cgraph gvc ingraphs)
if(NOT HAVE_GETOPT_H)
    target_link_libraries(gv2gml ${Getopt_LINK_LIBRARIES})
endif(NOT HAVE_GETOPT_H)
install(TARGETS gv2gml RUNTIME DESTINATION ${BINARY_INSTALL_DIR})

# ================================= graphml2gv =================================
if(EXPAT_FOUND)

add_executable(graphml2gv
    # Source files
    graphml2gv.c
)

target_include_directories(graphml2gv PRIVATE
    ${GRAPHVIZ_LIB_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
    ${GRAPHVIZ_LIB_DIR}/cdt
    ${GRAPHVIZ_LIB_DIR}/cgraph
    ${GRAPHVIZ_LIB_DIR}/common
    ${GRAPHVIZ_LIB_DIR}/gvc
    ${GRAPHVIZ_LIB_DIR}/pack
    ${GRAPHVIZ_LIB_DIR}/pathplan
    ${EXPAT_INCLUDE_DIRS}
    ${Getopt_INCLUDE_DIRS}
)

target_link_libraries(graphml2gv
    cgraph
    ${EXPAT_LIBRARIES}
)

tool_defaults(graphml2gv)

endif(EXPAT_FOUND)

# ================================== gvcolor ===================================
add_executable(gvcolor
    # Header files
    colortbl.h

    # Source files
    colxlate.c
    gvcolor.c
)

target_include_directories(gvcolor PRIVATE
    ${GRAPHVIZ_LIB_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
    ${GRAPHVIZ_LIB_DIR}/cdt
    ${GRAPHVIZ_LIB_DIR}/cgraph
    ${GRAPHVIZ_LIB_DIR}/common
    ${GRAPHVIZ_LIB_DIR}/gvc
    ${GRAPHVIZ_LIB_DIR}/pack
    ${GRAPHVIZ_LIB_DIR}/pathplan
    ${Getopt_INCLUDE_DIRS}
)

target_link_libraries(gvcolor
    cgraph
    ingraphs
)

tool_defaults(gvcolor)

# ==================================== gvgen ===================================
add_executable(gvgen
    # Source files
    graph_generator.c
    gvgen.c
)

target_include_directories(gvgen PRIVATE
    ${GRAPHVIZ_LIB_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
    ${GRAPHVIZ_LIB_DIR}/cdt
    ${GRAPHVIZ_LIB_DIR}/cgraph
    ${GRAPHVIZ_LIB_DIR}/common
    ${GRAPHVIZ_LIB_DIR}/gvc
    ${GRAPHVIZ_LIB_DIR}/pack
    ${GRAPHVIZ_LIB_DIR}/pathplan
    ${Getopt_INCLUDE_DIRS}
)

target_link_libraries(gvgen cgraph)

# Link to math library
if (UNIX)
    target_link_libraries(gvgen m)
endif (UNIX)

tool_defaults(gvgen)

# =================================== gvpack ===================================
# TODO add gvpack_static

add_definitions(-DDEMAND_LOADING=0)

add_executable(gvpack
    # Source files
    gvpack.c
)

target_include_directories(gvpack PRIVATE
    ${GRAPHVIZ_LIB_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
    ${GRAPHVIZ_LIB_DIR}/cdt
    ${GRAPHVIZ_LIB_DIR}/cgraph
    ${GRAPHVIZ_LIB_DIR}/common
    ${GRAPHVIZ_LIB_DIR}/gvc
    ${GRAPHVIZ_LIB_DIR}/pack
    ${GRAPHVIZ_LIB_DIR}/pathplan
    ${Getopt_INCLUDE_DIRS}
)

target_link_libraries(gvpack
    cgraph
    gvc
    gvplugin_neato_layout
    ingraphs
)

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
    ${GRAPHVIZ_LIB_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
    ${GRAPHVIZ_LIB_DIR}/cdt
    ${GRAPHVIZ_LIB_DIR}/cgraph
    ${GRAPHVIZ_LIB_DIR}/common
    ${GRAPHVIZ_LIB_DIR}/gvc
    ${GRAPHVIZ_LIB_DIR}/pack
    ${GRAPHVIZ_LIB_DIR}/pathplan
    ${EXPAT_INCLUDE_DIRS}
    ${Getopt_INCLUDE_DIRS}
)

target_link_libraries(gxl2gv
    cgraph
    gvc
    ingraphs
    ${EXPAT_LIBRARIES}
)

tool_defaults(gxl2gv)

endif(EXPAT_FOUND)

# ==================================== mm2gv ===================================
add_executable(mm2gv
    # Source files
    matrix_market.c
    mm2gv.c
    mmio.c
)

target_include_directories(mm2gv PRIVATE
    ${GRAPHVIZ_LIB_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
    ${GRAPHVIZ_LIB_DIR}/cdt
    ${GRAPHVIZ_LIB_DIR}/cgraph
    ${GRAPHVIZ_LIB_DIR}/common
    ${GRAPHVIZ_LIB_DIR}/gvc
    ${GRAPHVIZ_LIB_DIR}/pack
    ${GRAPHVIZ_LIB_DIR}/pathplan
    ${Getopt_INCLUDE_DIRS}
)

target_link_libraries(mm2gv
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
    ${GRAPHVIZ_LIB_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
    ${GRAPHVIZ_LIB_DIR}/cdt
    ${GRAPHVIZ_LIB_DIR}/cgraph
    ${GRAPHVIZ_LIB_DIR}/common
    ${GRAPHVIZ_LIB_DIR}/gvc
    ${GRAPHVIZ_LIB_DIR}/pack
    ${GRAPHVIZ_LIB_DIR}/pathplan
    ${Getopt_INCLUDE_DIRS}
)

target_link_libraries(sccmap
    cgraph
    ingraphs
)

tool_defaults(sccmap)

# ===================== Install third party DLLs on Windows ====================

if (WIN32 AND EXPAT_FOUND)
    install(
        FILES ${EXPAT_RUNTIME_LIBRARIES}
        DESTINATION ${BINARY_INSTALL_DIR}
    )
endif(WIN32 AND EXPAT_FOUND)

if (WIN32)
    install(
        FILES ${Getopt_RUNTIME_LIBRARIES}
        DESTINATION ${BINARY_INSTALL_DIR}
    )
endif()
