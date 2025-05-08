if(CMAKE_VERSION VERSION_GREATER_EQUAL "3.30")
  # is Boost available and new enough to ship its own config?
  cmake_policy(SET CMP0167 NEW)
  find_package(Boost CONFIG)

  # if not, fallback to CMake’s Boost config
  if(NOT Boost_FOUND)
    cmake_policy(SET CMP0167 OLD)
    find_package(Boost REQUIRED)
  endif()
else()
  find_package(Boost REQUIRED)
endif()

find_package(Catch2 REQUIRED)
find_package(RAPIDXML REQUIRED)
find_package(SVGPP REQUIRED)

enable_testing()

# for convenience, make a library that depends on everything so each
# separate test can be as small as possible
add_library(test_common SHARED
            graphviz_edge.cpp
            graphviz_edge.h
            graphviz_graph.cpp
            graphviz_graph.h
            graphviz_node.h
            graphviz_node.cpp
            test_utilities.cpp
            svgpp_context.cpp
            svgpp_context.h
            svgpp_document_traverser.cpp
            svgpp_document_traverser.h
            svg_analyzer.cpp
            svg_analyzer.h
            svg_analyzer_interface.h
            svg_element.cpp
            svg_element.h
            test_edge_node_overlap_utilities.cpp
            test_edge_node_overlap_utilities.h
            ../cmd/dot/dot_builtins.cpp
)
set_target_properties(test_common PROPERTIES CXX_STANDARD 20)
set_target_properties(test_common PROPERTIES CXX_STANDARD_REQUIRED ON)
target_include_directories(test_common PRIVATE
  ../lib
  ../lib/cgraph++
)
target_include_directories(test_common SYSTEM PRIVATE
  ${Boost_INCLUDE_DIRS}
  ${RAPIDXML_INCLUDE_DIRS}
  ${SVGPP_INCLUDE_DIRS}
)
target_link_libraries(test_common PUBLIC
  gvplugin_core
  gvplugin_dot_layout
  gvplugin_gd
  gvplugin_neato_layout
  gvplugin_pango
  gvplugin_vt
  gvplugin_webp
  cgraph
  cgraph++
  gvc
  gvc++
)

# common steps for creating a test case
macro(CREATE_TEST testname)
  add_executable(test_${testname} test_${testname}.cpp)
  set_target_properties(test_${testname} PROPERTIES CXX_STANDARD 20)
  set_target_properties(test_${testname} PROPERTIES CXX_STANDARD_REQUIRED ON)
  add_test(NAME test_${testname}
           COMMAND ./test_${testname} --reporter JUnit::out=test_${testname}.xml
             --reporter console::out=-::colour-mode=ansi)
  target_include_directories(test_${testname} PRIVATE
    ../lib
    ../lib/cdt
    ../lib/cgraph
    ../lib/cgraph++
    ../lib/common
    ../lib/gvc
    ../lib/pathplan
  )
  target_link_libraries(test_${testname} PRIVATE
    test_common
  )
  target_link_libraries(test_${testname} PRIVATE Catch2::Catch2WithMain)
endmacro()

CREATE_TEST(AGraph_construction)
CREATE_TEST(clusters)
CREATE_TEST(edge_color)
CREATE_TEST(edge_fillcolor)
CREATE_TEST(edge_penwidth)
CREATE_TEST(engines)
CREATE_TEST(GVContext_construction)
CREATE_TEST(GVContext_render_svg)
CREATE_TEST(GVLayout_construction)
CREATE_TEST(GVLayout_render)
CREATE_TEST(edge_node_overlap_all_edge_arrows)
CREATE_TEST(edge_node_overlap_all_node_shapes)
CREATE_TEST(edge_node_overlap_all_primitive_edge_arrows)
CREATE_TEST(edge_node_overlap_box_edge_arrow)
CREATE_TEST(edge_node_overlap_box_edge_arrow_various_arrowsizes)
CREATE_TEST(edge_node_overlap_crow_and_vee_edge_arrows)
CREATE_TEST(edge_node_overlap_curve_and_icurve_edge_arrows)
CREATE_TEST(edge_node_overlap_cylinder_node_shape)
CREATE_TEST(edge_node_overlap_diamond_edge_arrow)
CREATE_TEST(edge_node_overlap_diamond_edge_arrow_various_arrowsizes)
CREATE_TEST(edge_node_overlap_dot_edge_arrow)
CREATE_TEST(edge_node_overlap_dot_edge_arrow_various_arrowsizes)
CREATE_TEST(edge_node_overlap_ellipse_node_shapes_except_point)
CREATE_TEST(edge_node_overlap_none_edge_arrow)
CREATE_TEST(edge_node_overlap_none_edge_arrow_various_arrowsizes)
CREATE_TEST(edge_node_overlap_normal_and_inv_edge_arrows)
CREATE_TEST(edge_node_overlap_normal_and_inv_edge_arrows_all_modifiers)
CREATE_TEST(edge_node_overlap_normal_and_inv_edge_arrows_various_arrowsizes)
CREATE_TEST(edge_node_overlap_plain_node_shape)
CREATE_TEST(edge_node_overlap_point_node_shape)
CREATE_TEST(edge_node_overlap_polygon_node_shapes)
CREATE_TEST(edge_node_overlap_record_node_shape)
CREATE_TEST(edge_node_overlap_simple)
CREATE_TEST(edge_node_overlap_tee_edge_arrow)
CREATE_TEST(edge_node_overlap_tee_edge_arrow_various_arrowsizes)
CREATE_TEST(max_edge_node_overlap_simple)
CREATE_TEST(max_edge_node_overlap_polygon_node_shapes)
CREATE_TEST(min_edge_node_overlap_polygon_node_shapes)
CREATE_TEST(min_edge_node_overlap_simple)
CREATE_TEST(max_edge_stem_arrow_overlap_simple)
CREATE_TEST(min_edge_stem_arrow_overlap_simple)
CREATE_TEST(neatopack)
CREATE_TEST(node_color)
CREATE_TEST(node_fillcolor)
CREATE_TEST(node_penwidth)
CREATE_TEST(rankdir)
CREATE_TEST(simple)
CREATE_TEST(subgraph_layout)
CREATE_TEST(subgraphs)
CREATE_TEST(svg_analyzer)
CREATE_TEST(svg_analyzer_color)
CREATE_TEST(svg_analyzer_fillcolor)
CREATE_TEST(svg_analyzer_penwidth)
