if(WITH_SMYRNA)

  find_package(Freetype REQUIRED)
  find_package(GLUT REQUIRED)
  find_package(GTK2 REQUIRED COMPONENTS gtk glade)
  find_package(PANGOCAIRO REQUIRED)
  find_package(PkgConfig REQUIRED)
  pkg_check_modules(XRENDER REQUIRED xrender)

  add_library(glcomp OBJECT
    glcompbutton.c
    glcompfont.c
    glcompimage.c
    glcomplabel.c
    glcompmouse.c
    glcomppanel.c
    glcompset.c
    glcomptextpng.c
    glcomptexture.c
    glpangofont.c
    glutils.c
  )

  target_include_directories(glcomp PRIVATE
    ..
    ../common
  )

  target_include_directories(glcomp SYSTEM PRIVATE
    ${FREETYPE_INCLUDE_DIRS}
    ${GLUT_INCLUDE_DIR}
    ${GTK2_INCLUDE_DIRS}
    ${PANGOCAIRO_INCLUDE_DIRS}
    ${XRENDER_INCLUDE_DIRS}
  )

  if(WIN32 AND NOT MINGW)
    target_include_directories(glcomp PRIVATE ../../windows/include/unistd)
  endif()

  target_link_libraries(glcomp PRIVATE
    ${FREETYPE_LIBRARIES}
    ${GLUT_LIBRARIES}
    ${GTK2_LIBRARIES}
    ${PANGOCAIRO_LINK_LIBRARIES}
    ${XRENDER_LIBRARIES}
  )

endif()
