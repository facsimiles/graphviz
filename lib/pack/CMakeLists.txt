add_library(pack_obj OBJECT
  # Header files
  pack.h

  # Source files
  ccomps.c
  pack.c
)

target_compile_definitions(pack_obj PRIVATE GVC_EXPORTS)

target_include_directories(pack_obj PRIVATE
  ..
  ${CMAKE_CURRENT_SOURCE_DIR}
  ../cdt
  ../cgraph
  ../common
  ../gvc
  ../pathplan
)

target_link_libraries(pack_obj PRIVATE util)

add_library(pack STATIC
  $<TARGET_OBJECTS:pack_obj>
)

target_link_libraries(pack PRIVATE util)

# Specify headers to be installed
install(
  FILES pack.h
  DESTINATION ${HEADER_INSTALL_DIR}
)

# Specify man pages to be installed
if(GZIP)
  add_custom_target(man-pack ALL DEPENDS pack.3.gz
                    COMMENT "pack man page")
  add_custom_command(
    OUTPUT pack.3.gz
    COMMAND ${GZIP} -9 --no-name --to-stdout pack.3
      >"${CMAKE_CURRENT_BINARY_DIR}/pack.3.gz"
    MAIN_DEPENDENCY pack.3
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMENT "compress pack man page")
  install(
    FILES ${CMAKE_CURRENT_BINARY_DIR}/pack.3.gz
    DESTINATION ${MAN_INSTALL_DIR}/man3)
else()
  install(
    FILES pack.3
    DESTINATION ${MAN_INSTALL_DIR}/man3
  )
endif()
