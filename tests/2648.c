/// @file
/// @brief accompanying test case for test_regression.py::test_2648

#include <assert.h>
#include <graphviz/cgraph.h>
#include <graphviz/gvc.h>
#include <graphviz/gvcext.h>
#include <stddef.h>
#include <stdio.h>

static GVC_t *gv_global_context_;

#ifdef _WIN32
#define IMPORT __declspec(dllimport)
#else
#define IMPORT /* nothing */
#endif

IMPORT extern gvplugin_library_t gvplugin_dot_layout_LTX_library;
IMPORT extern gvplugin_library_t gvplugin_core_LTX_library;

lt_symlist_t lt_preloaded_symbols[3];

static void RenderDot(char *dot_input, char *format) {
  Agraph_t *const gv_graph = agmemread(dot_input);
  assert(gv_graph != NULL);

  const int layout_result = gvLayout(gv_global_context_, gv_graph, "dot");

  assert(layout_result == 0 && "layout error");

#ifdef _WIN32
  const char devnull[] = "nul";
#else
  const char devnull[] = "/dev/null";
#endif
  FILE *const f = fopen(devnull, "w");
  assert(f != NULL);

  (void)gvRender(gv_global_context_, gv_graph, format, f);
  (void)fclose(f);

  gvFreeLayout(gv_global_context_, gv_graph);
  agclose(gv_graph);
}

int main(void) {

  // Define plugins. This is done here rather than globally in a static
  // initializer because MSVC insists on strict const rules and claims the
  // addresses of the plugins cannot be used at compile time.
  lt_preloaded_symbols[0] =
      (lt_symlist_t){.name = "gvplugin_dot_layout_LTX_library",
                     .address = &gvplugin_dot_layout_LTX_library};
  lt_preloaded_symbols[1] =
      (lt_symlist_t){.name = "gvplugin_core_LTX_library",
                     .address = &gvplugin_core_LTX_library};

  gv_global_context_ =
      gvContextPlugins(lt_preloaded_symbols, 0 /* DEMAND_LOADING */);

  char *const dot = "digraph \"Root\" { \n"
                    "  label=\"Root\" \n"
                    "  graph [rankdir=LR] \n"
                    "  subgraph child { \n"
                    "    label=child \n"
                    "  } \n"
                    "}";
  RenderDot(dot, "svg");
  RenderDot(dot, "png");
  RenderDot(dot, "dot");
  return 0;
}
