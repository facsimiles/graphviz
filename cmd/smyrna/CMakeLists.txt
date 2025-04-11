if(WITH_SMYRNA)

  if(WITH_EXPAT)
    find_package(EXPAT REQUIRED)
  endif()
  find_package(Freetype REQUIRED)
  find_package(GLUT REQUIRED)
  find_package(GTK2 REQUIRED COMPONENTS gtk glade)
  find_package(PkgConfig REQUIRED)
  find_package(Fontconfig REQUIRED)
  pkg_check_modules(GTKGLEXT REQUIRED gtkglext-1.0)
  pkg_check_modules(XRENDER REQUIRED xrender)

  add_executable(smyrna
    arcball.h
    draw.h
    glexpose.h
    glmotion.h
    gltemplate.h
    glutrender.h
    gui/appmouse.h
    gui/callbacks.h
    gui/datalistcallbacks.h
    gui/frmobjectui.h
    gui/glcompui.h
    gui/gui.h
    gui/menucallbacks.h
    gui/toolboxcallbacks.h
    gui/topviewsettings.h
    gvprpipe.h
    hier.h
    hotkeymap.h
    polytess.h
    selectionfuncs.h
    smyrna_utils.h
    smyrnadefs.h
    topfisheyeview.h
    topviewfuncs.h
    tvnodes.h
    viewport.h
    viewportcamera.h

    arcball.c
    draw.c
    glexpose.c
    glmotion.c
    gltemplate.c
    gui/appmouse.c
    gui/callbacks.c
    gui/datalistcallbacks.c
    gui/frmobjectui.c
    gui/glcompui.c
    gui/gui.c
    gui/menucallbacks.c
    gui/toolboxcallbacks.c
    gui/topviewsettings.c
    glutrender.c
    gvprpipe.c
    hier.c
    hotkeymap.c
    main.c
    polytess.c
    selectionfuncs.c
    smyrna_utils.c
    topfisheyeview.c
    topviewfuncs.c
    tvnodes.c
    viewport.c
    viewportcamera.c
  )

  target_include_directories(smyrna PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ../../lib
    ../../lib/cdt
    ../../lib/cgraph
    ../../lib/common
    ../../lib/gvc
    ../../lib/gvpr
    ../../lib/pathplan
    gui
  )

  target_include_directories(smyrna SYSTEM PRIVATE
    ${EXPAT_INCLUDE_DIRS}
    ${Fontconfig_INCLUDE_DIRS}
    ${FREETYPE_INCLUDE_DIRS}
    ${GLUT_INCLUDE_DIR}
    ${GTK2_INCLUDE_DIRS}
    ${GTKGLEXT_INCLUDE_DIRS}
    ${XRENDER_INCLUDE_DIRS}
  )

  target_link_libraries(smyrna PRIVATE
    cdt
    cgraph
    expr
    $<TARGET_OBJECTS:glcomp>
    gvc
    gvpr
    neatogen
    pathplan
    topfish
    util
    xdot
    ${EXPAT_LIBRARIES}
    ${Fontconfig_LIBRARIES}
    ${FREETYPE_LIBRARIES}
    ${GLUT_LIBRARIES}
    ${GTK2_LIBRARIES}
    ${GTKGLEXT_LIBRARIES}
    ${XRENDER_LIBRARIES}
  )

  install(
    TARGETS smyrna
    RUNTIME DESTINATION ${BINARY_INSTALL_DIR}
    LIBRARY DESTINATION ${LIBRARY_INSTALL_DIR}
    ARCHIVE DESTINATION ${LIBRARY_INSTALL_DIR}
  )

  if(GZIP)
    add_custom_target(man-smyrna ALL DEPENDS smyrna.1.gz
                      COMMENT "smyrna man page")
    add_custom_command(
      OUTPUT smyrna.1.gz
      COMMAND ${GZIP} -9 --no-name --to-stdout smyrna.1
        >"${CMAKE_CURRENT_BINARY_DIR}/smyrna.1.gz"
      MAIN_DEPENDENCY smyrna.1
      WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
      COMMENT "compress smyrna man page")
    install(
      FILES ${CMAKE_CURRENT_BINARY_DIR}/smyrna.1.gz
      DESTINATION ${MAN_INSTALL_DIR}/man1)
  else()
    install(
      FILES smyrna.1
      DESTINATION ${MAN_INSTALL_DIR}/man1
    )
  endif()

endif()
