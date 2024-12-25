cmake_minimum_required(VERSION 3.12 FATAL_ERROR)
project(test_2598 C)

# listen to `graphviz_ROOT` environment variable during `find_package(graphviz)`
cmake_policy(SET CMP0074 NEW)

# find Graphviz libraries
find_package(graphviz REQUIRED)

# define an application that uses libcgraph
add_executable(cgraph_app cgraph_app.c)
target_link_libraries(cgraph_app PRIVATE graphviz::cgraph)

# define an application that uses libgvc
add_executable(gvc_app gvc_app.c)
target_link_libraries(gvc_app PRIVATE graphviz::gvc)

# define an application that uses libpathplan
add_executable(pathplan_app pathplan_app.c)
target_link_libraries(pathplan_app PRIVATE graphviz::pathplan)

# define an application that uses libxdot
add_executable(xdot_app xdot_app.c)
target_link_libraries(xdot_app PRIVATE graphviz::xdot)
