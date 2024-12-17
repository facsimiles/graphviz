cmake_minimum_required(VERSION 3.0 FATAL_ERROR)
project(test_2598 C)

# find Graphviz libraries
find_package(cdt REQUIRED)
find_package(cgraph REQUIRED)
find_package(gvc REQUIRED)
find_package(pathplan REQUIRED)
find_package(xdot REQUIRED)

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
