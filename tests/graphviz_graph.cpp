#include <stdexcept>

#include <fmt/format.h>

#include "graphviz_graph.h"

GraphvizGraph::GraphvizGraph(SVG::SVGElement &svg_g_element)
    : m_svg_g_element(svg_g_element) {}

void GraphvizGraph::add_node(SVG::SVGElement &svg_g_element) {
  m_nodes.emplace_back(svg_g_element);
}

const std::vector<GraphvizNode> &GraphvizGraph::nodes() const {
  return m_nodes;
}

const SVG::SVGElement &GraphvizGraph::svg_g_element() const {
  return m_svg_g_element;
}

const GraphvizNode &GraphvizGraph::node(std::string_view node_id) const {
  for (auto &node : m_nodes) {
    if (node.node_id() == node_id) {
      return node;
    }
  }
  throw std::runtime_error{
      fmt::format("Unknown node '{}' in graph '{}'", node_id, m_graph_id)};
}

void GraphvizGraph::add_edge(SVG::SVGElement &svg_g_element) {
  m_edges.emplace_back(svg_g_element);
}

const std::vector<GraphvizEdge> &GraphvizGraph::edges() const {
  return m_edges;
}
