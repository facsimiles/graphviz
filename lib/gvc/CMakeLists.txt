add_definitions(-D_BLD_gvc=1 -DGVC_EXPORTS -DGVLIBDIR="${LIBRARY_INSTALL_DIR}/graphviz")

add_library(gvc SHARED
    # Header files
    gvc.h
    gvcext.h
    gvcint.h
    gvcjob.h
    gvconfig.h
    gvcommon.h
    gvcproc.h
    gvio.h
    gvplugin.h
    gvplugin_device.h
    gvplugin_layout.h
    gvplugin_loadimage.h
    gvplugin_render.h
    gvplugin_textlayout.h

    # Source files
    gvc.c
    gvconfig.c
    gvcontext.c
    gvdevice.c
    gvevent.c
    gvjobs.c
    gvlayout.c
    gvloadimage.c
    gvplugin.c
    gvrender.c
    gvtextlayout.c
    gvtool_tred.c
    gvusershape.c

    # Link definition file for Windows
    gvc.def
)

target_include_directories(gvc
  PUBLIC
    $<BUILD_INTERFACE:${GRAPHVIZ_LIB_DIR}>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<BUILD_INTERFACE:${GRAPHVIZ_LIB_DIR}/cdt>
    $<BUILD_INTERFACE:${GRAPHVIZ_LIB_DIR}/cgraph>
    $<BUILD_INTERFACE:${GRAPHVIZ_LIB_DIR}/common>
    $<BUILD_INTERFACE:${GRAPHVIZ_LIB_DIR}/pathplan>
    $<INSTALL_INTERFACE:include/graphviz>
  PRIVATE
    ${TOP_SOURCE_DIR}
)

target_link_libraries(gvc PRIVATE
    $<TARGET_OBJECTS:common_obj>
    $<TARGET_OBJECTS:pack_obj>
)

target_link_libraries(gvc PRIVATE
    cdt
    cgraph
    common
    pack
)

target_link_libraries(gvc PUBLIC
    ${MATH_LIB}
)

if (LTDL_FOUND)
    target_include_directories(gvc PRIVATE ${LTDL_INCLUDE_DIRS})
    if (NOT WIN32 OR MINGW)
        target_link_libraries(gvc PUBLIC ${LTDL_LIBRARIES})
    endif()
endif()

if (ZLIB_FOUND)
   target_link_libraries(gvc PUBLIC ${ZLIB_LIBRARIES})
endif()

if (${with_ortho})
    target_link_libraries(gvc PRIVATE
        $<TARGET_OBJECTS:ortho_obj>
    )
    target_link_libraries(gvc PRIVATE ortho)
endif()

# Installation location of library files
install(
    TARGETS gvc
    RUNTIME DESTINATION ${BINARY_INSTALL_DIR}
    LIBRARY DESTINATION ${LIBRARY_INSTALL_DIR}
    ARCHIVE DESTINATION ${LIBRARY_INSTALL_DIR}
)

# Specify headers to be installed
install(
    FILES
        gvc.h
        gvcext.h
        gvcjob.h
        gvconfig.h
        gvcommon.h
        gvplugin.h
        gvplugin_device.h
        gvplugin_layout.h
        gvplugin_loadimage.h
        gvplugin_render.h
        gvplugin_textlayout.h
    DESTINATION ${HEADER_INSTALL_DIR}
)

# Specify man pages to be installed
if(GZIP)
  add_custom_target(man-gvc ALL DEPENDS gvc.3.gz)
  add_custom_command(
    OUTPUT gvc.3.gz
    COMMAND ${GZIP} -9 --no-name --to-stdout gvc.3
      >"${CMAKE_CURRENT_BINARY_DIR}/gvc.3.gz"
    MAIN_DEPENDENCY gvc.3
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
  install(
    FILES ${CMAKE_CURRENT_BINARY_DIR}/gvc.3.gz
    DESTINATION ${MAN_INSTALL_DIR}/man3)
else()
  install(
    FILES gvc.3
    DESTINATION ${MAN_INSTALL_DIR}/man3
  )
endif()

# Specify library version and soversion
set_target_properties(gvc PROPERTIES
    VERSION 6.0.0
    SOVERSION 6
)

# Include DLLs with this library on Windows
if (WIN32 AND EXPAT_FOUND AND install_win_dependency_dlls)
    install(
        FILES
            ${EXPAT_RUNTIME_LIBRARIES}
        DESTINATION ${BINARY_INSTALL_DIR}
    )
endif()
