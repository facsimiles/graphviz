cmake_minimum_required(VERSION 3.0 FATAL_ERROR)
project(test_2598_1 C)

# find Graphviz libraries
find_package(cgraph REQUIRED)

# define an application that uses libcgraph
add_executable(cgraph_app cgraph_app.c)
target_link_libraries(cgraph_app PRIVATE graphviz::cgraph)
