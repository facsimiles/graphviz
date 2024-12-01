include_directories(
  ../../lib
  ${CMAKE_CURRENT_BINARY_DIR}
  ../../lib/cdt
  ../../lib/cgraph
  ../../lib/common
  ../../lib/gvc
  ../../lib/pathplan
)

if(BUILD_SHARED_LIBS)
  add_executable(dot
    # Source files
    dot.c
    no_builtins.c
  )

  target_compile_definitions(dot PRIVATE DEMAND_LOADING=1)

  target_link_libraries(dot PRIVATE
    cgraph
    gvc
  )

  if(APPLE)
    set_target_properties(dot PROPERTIES LINK_FLAGS -Wl,-stack_size,0x2000000)
  elseif(MINGW)
    set_target_properties(dot PROPERTIES LINK_FLAGS -Wl,--stack,0x2000000)
  elseif(WIN32)
    set_target_properties(dot PROPERTIES LINK_FLAGS /STACK:"33554432")
  endif()

  # Installation location of executables
  install(
    TARGETS dot
    RUNTIME DESTINATION ${BINARY_INSTALL_DIR}
    LIBRARY DESTINATION ${LIBRARY_INSTALL_DIR}
    ARCHIVE DESTINATION ${LIBRARY_INSTALL_DIR}
  )

  add_executable(dot_builtins
    dot.c
    dot_builtins.cpp
  )

  target_compile_definitions(dot_builtins PRIVATE DEMAND_LOADING=1)

  target_link_libraries(dot_builtins PRIVATE
    cdt
    cgraph
    gvc
    gvplugin_core
    gvplugin_dot_layout
    gvplugin_neato_layout
    gvplugin_vt
    pathplan
    xdot
  )

  if(APPLE)
    set_target_properties(dot_builtins PROPERTIES
      LINK_FLAGS -Wl,-stack_size,0x2000000)
  elseif(MINGW)
    set_target_properties(dot_builtins PROPERTIES
      LINK_FLAGS -Wl,--stack,0x2000000)
  elseif(WIN32)
    set_target_properties(dot_builtins PROPERTIES LINK_FLAGS /STACK:"33554432")
  endif()

  if(EXPAT_FOUND)
    target_include_directories(dot_builtins SYSTEM PRIVATE
      ${EXPAT_INCLUDE_DIRS}
    )
    target_link_libraries(dot_builtins PRIVATE ${EXPAT_LIBRARIES})
  endif()

  if(GD_FOUND)
    target_link_libraries(dot_builtins PRIVATE gvplugin_gd)
  endif()

  if(GTS_FOUND)
    target_include_directories(dot_builtins SYSTEM PRIVATE
      ${GTS_INCLUDE_DIRS}
    )
    target_link_libraries(dot_builtins PRIVATE ${GTS_LINK_LIBRARIES})
  endif()

  if(LASI_FOUND)
    target_link_libraries(dot_builtins PRIVATE gvplugin_lasi)
  endif()

  if(PANGOCAIRO_FOUND)
    target_link_libraries(dot_builtins PRIVATE gvplugin_pango)
    if(WEBP_FOUND)
      target_link_libraries(dot_builtins PRIVATE gvplugin_webp)
    endif()
  endif()

  install(
    TARGETS dot_builtins
    RUNTIME DESTINATION ${BINARY_INSTALL_DIR}
    LIBRARY DESTINATION ${LIBRARY_INSTALL_DIR}
    ARCHIVE DESTINATION ${LIBRARY_INSTALL_DIR}
  )
else()
  add_executable(dot
    dot.c
    dot_builtins.cpp
  )

  target_compile_definitions(dot PRIVATE DEMAND_LOADING=0)

  target_link_libraries(dot PRIVATE
    cdt
    cgraph
    gvc
    gvplugin_core
    gvplugin_dot_layout
    gvplugin_neato_layout
    gvplugin_vt
    pathplan
    xdot
  )

  if(APPLE)
    set_target_properties(dot PROPERTIES LINK_FLAGS -Wl,-stack_size,0x2000000)
  elseif(MINGW)
    set_target_properties(dot PROPERTIES LINK_FLAGS -Wl,--stack,0x2000000)
  elseif(WIN32)
    set_target_properties(dot PROPERTIES LINK_FLAGS /STACK:"33554432")
  endif()

  if(GD_FOUND)
    target_link_libraries(dot PRIVATE gvplugin_gd)
  endif()

  if(Freetype_FOUND AND LASI_FOUND AND PANGOCAIRO_FOUND)
    target_link_libraries(dot PRIVATE gvplugin_lasi)
  endif()

  if(CAIRO_FOUND AND PANGOCAIRO_FOUND)
    target_link_libraries(dot PRIVATE gvplugin_pango)
  endif()

  if(WEBP_FOUND)
    target_link_libraries(dot PRIVATE gvplugin_webp)
  endif()

  install(
    TARGETS dot
    RUNTIME DESTINATION ${BINARY_INSTALL_DIR}
    LIBRARY DESTINATION ${LIBRARY_INSTALL_DIR}
    ARCHIVE DESTINATION ${LIBRARY_INSTALL_DIR}
  )
endif()

# Aliases to the dot executable (not including '.exe' suffix)
list(APPEND dot_aliases circo fdp neato osage patchwork sfdp twopi)

# We use copying instead of symlinking for Cygwin to avoid
# https://gitlab.com/graphviz/graphviz/-/issues/2123

foreach(cmd_alias IN LISTS dot_aliases)
  set(DOTCOPY
      "${CMAKE_CURRENT_BINARY_DIR}/${cmd_alias}${CMAKE_EXECUTABLE_SUFFIX}")
  if(WIN32 OR CYGWIN)
    # Copy dot executable to each alias name then install copies to bindir
    add_custom_command(
      TARGET dot
      POST_BUILD
      COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:dot> ${DOTCOPY}
      COMMENT "Copying dot to ${DOTCOPY}"
    )
    install(
      PROGRAMS ${DOTCOPY}
      DESTINATION ${BINARY_INSTALL_DIR}
    )
  else()
    # For not-WIN32, install symlinks from dot_aliases -> dot_executable in
    # bindir. Note: This may be brittle. This builds a symlink from
    # ./cmd_alias -> ./dot in ${CMAKE_CURRENT_BINARY_DIR}, then installs that
    # symlink into ${BINARY_INSTALL_DIR}. This presumes
    # ${CMAKE_CURRENT_BINARY_DIR}/dot is installed to ${BINARY_INSTALL_DIR}/dot.
    # There is a (small?) risk of dangling symlinks
    add_custom_command(
      TARGET dot
      POST_BUILD
      COMMAND ${CMAKE_COMMAND} -E create_symlink $<TARGET_FILE_NAME:dot>
        ${cmd_alias}
      WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
      COMMENT "Linking dot as ${cmd_alias}"
    )
    install(
      FILES ${DOTCOPY}
      DESTINATION ${BINARY_INSTALL_DIR}
    )
  endif()
endforeach()

install(
  FILES dot_sandbox
  DESTINATION ${BINARY_INSTALL_DIR}
  PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE
    WORLD_READ WORLD_EXECUTE
)

# Specify man pages to be installed
if(GZIP)
  add_custom_target(man-dot ALL DEPENDS dot.1.gz
                    COMMENT "dot man page")
  add_custom_command(
    OUTPUT dot.1.gz
    COMMAND ${GZIP} -9 --no-name --to-stdout dot.1
      >"${CMAKE_CURRENT_BINARY_DIR}/dot.1.gz"
    MAIN_DEPENDENCY dot.1
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMENT "compress dot man page")
  install(
    FILES ${CMAKE_CURRENT_BINARY_DIR}/dot.1.gz
    DESTINATION ${MAN_INSTALL_DIR}/man1)
  add_custom_target(man-dot_sandbox ALL DEPENDS dot_sandbox.1.gz
                    COMMENT "dot_sandbox man page")
  add_custom_command(
    OUTPUT dot_sandbox.1.gz
    COMMAND ${GZIP} -9 --no-name --to-stdout dot_sandbox.1
      >"${CMAKE_CURRENT_BINARY_DIR}/dot_sandbox.1.gz"
    MAIN_DEPENDENCY dot_sandbox.1
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMENT "compress dot_sandbox man page")
  install(
    FILES ${CMAKE_CURRENT_BINARY_DIR}/dot_sandbox.1.gz
    DESTINATION ${MAN_INSTALL_DIR}/man1)
  add_custom_target(man-osage ALL DEPENDS osage.1.gz
                    COMMENT "osage man page")
  add_custom_command(
    OUTPUT osage.1.gz
    COMMAND ${GZIP} -9 --no-name --to-stdout osage.1
      >"${CMAKE_CURRENT_BINARY_DIR}/osage.1.gz"
    MAIN_DEPENDENCY osage.1
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMENT "compress osage man page")
  install(
    FILES ${CMAKE_CURRENT_BINARY_DIR}/osage.1.gz
    DESTINATION ${MAN_INSTALL_DIR}/man1)
  add_custom_target(man-patchwork ALL DEPENDS patchwork.1.gz
                    COMMENT "patchwork man page")
  add_custom_command(
    OUTPUT patchwork.1.gz
    COMMAND ${GZIP} -9 --no-name --to-stdout patchwork.1
      >"${CMAKE_CURRENT_BINARY_DIR}/patchwork.1.gz"
    MAIN_DEPENDENCY patchwork.1
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMENT "compress patchwork man page")
  install(
    FILES ${CMAKE_CURRENT_BINARY_DIR}/patchwork.1.gz
    DESTINATION ${MAN_INSTALL_DIR}/man1)
else()
  install(
    FILES dot.1 dot_sandbox.1 osage.1 patchwork.1
    DESTINATION ${MAN_INSTALL_DIR}/man1
  )
endif()

if(NOT CMAKE_CROSSCOMPILING)
  configure_file(
    ../../cmake/configure_plugins.cmake.in configure_plugins.cmake @ONLY)

  install(SCRIPT ${CMAKE_CURRENT_BINARY_DIR}/configure_plugins.cmake)
endif()
