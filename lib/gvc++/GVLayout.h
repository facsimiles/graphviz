#pragma once

#include <memory>

#include "cgraph++/AGraph.h"
#include "gvc++/GVContext.h"

#ifdef _WIN32
#if gvc___EXPORTS // CMake's substitution of gvc++_EXPORTS
#define GVLAYOUT_API __declspec(dllexport)
#else
#define GVLAYOUT_API __declspec(dllimport)
#endif
#else
#define GVLAYOUT_API /* nothing */
#endif

namespace GVC {

/**
 * @brief The GVLayout class represents a graph layout
 */

class GVLAYOUT_API GVLayout {
public:
  GVLayout(const std::shared_ptr<GVContext> &gvc,
           const std::shared_ptr<CGraph::AGraph> &g, const std::string &engine);
  ~GVLayout();

  // default copy since we manage resources through movable types
  GVLayout(const GVLayout &) = default;
  GVLayout &operator=(const GVLayout &) = default;

  // default move since we manage resources through movable types
  GVLayout(GVLayout &&) = default;
  GVLayout &operator=(GVLayout &&) = default;

private:
  std::shared_ptr<GVContext> m_gvc;
  std::shared_ptr<CGraph::AGraph> m_g;
};

} //  namespace GVC

#undef GVLAYOUT_API
