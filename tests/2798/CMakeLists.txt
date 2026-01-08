# see test_regression.py::test_2798

cmake_minimum_required(VERSION 3.21 FATAL_ERROR)
project(test_2798 C)

set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)

if(WIN32)
  add_definitions(-DGVDLL=1)
endif()

find_package(graphviz REQUIRED)

add_executable(demo ../../dot.demo/demo.c)
target_link_libraries(demo PRIVATE graphviz::cgraph graphviz::gvc)

add_executable(dot ../../dot.demo/dot.c)
target_link_libraries(dot PRIVATE graphviz::cgraph graphviz::gvc)

add_executable(example ../../dot.demo/example.c)
target_link_libraries(example PRIVATE graphviz::cgraph graphviz::gvc)

add_executable(neatopack ../../dot.demo/neatopack.c)
target_link_libraries(neatopack PRIVATE graphviz::cgraph graphviz::gvc)

add_executable(simple ../../dot.demo/simple.c)
target_link_libraries(simple PRIVATE graphviz::cgraph graphviz::gvc)
