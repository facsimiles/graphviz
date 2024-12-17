cmake_minimum_required(VERSION 3.12 FATAL_ERROR)
project(test_2598_1 C)

# listen to `graphviz_ROOT` environment variable during `find_package(graphviz)`
cmake_policy(SET CMP0074 NEW)

# find Graphviz libraries
find_package(graphviz REQUIRED)

# define an application that uses libcgraph
add_executable(cgraph_app cgraph_app.c)
target_link_libraries(cgraph_app PRIVATE graphviz::cgraph)
