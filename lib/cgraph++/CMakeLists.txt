add_library(cgraph++
  AGraph.cpp
)
set_property(TARGET cgraph++ PROPERTY CXX_STANDARD 20)
set_property(TARGET cgraph++ PROPERTY CXX_STANDARD_REQUIRED ON)

target_include_directories(cgraph++ PRIVATE
  ..
  ../cdt
  ../cgraph
  ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(cgraph++ PUBLIC cgraph)

install(
  TARGETS cgraph++
  RUNTIME DESTINATION ${BINARY_INSTALL_DIR}
  LIBRARY DESTINATION ${LIBRARY_INSTALL_DIR}
  ARCHIVE DESTINATION ${LIBRARY_INSTALL_DIR}
)

install(
  FILES AGraph.h
  DESTINATION ${HEADER_INSTALL_DIR}
)

set_target_properties(cgraph++ PROPERTIES
  VERSION 0.0.1
  SOVERSION 0
)
