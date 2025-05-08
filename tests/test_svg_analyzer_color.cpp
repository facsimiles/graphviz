#include <format>
#include <string_view>

#include <catch2/catch_all.hpp>

#include "svg_analyzer.h"
#include "test_utilities.h"

TEST_CASE("SvgAnalyzer color",
          "Test that the SvgAnalyzer can recreate the original "
          "SVG with the correct `stroke` and `stroke-opacity` attributes when "
          "the Graphviz `color` attribute is used for nodes and edges ") {

  const auto shape = GENERATE(from_range(all_node_shapes));
  INFO("Shape: " << shape);

  const std::string_view color =
      GENERATE("", "\"#10204000\"", "\"#10204080\"", "\"#102040ff\"");
  INFO("Color: " << color);
  const auto color_attr = color.empty() ? "" : std::format(" color={}", color);

  auto dot = std::format("digraph g1 {{node [shape={}{}]; edge [{}]; a -> b}}",
                         shape, color_attr, color_attr);

  SVGAnalyzer::make_from_dot(dot).re_create_and_verify_svg();
}
