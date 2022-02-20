// exit test for `dot -Vrandom`

#ifdef NDEBUG
#error this is not intended to be compiled with assertions off
#endif

#include <assert.h>
#include <graphviz/gvc.h>

static const lt_symlist_t lt_preloaded_symbols[] = {{0, 0}};
extern int GvExitOnUsage;

int main(void) {
  GVC_t *Gvc = gvContextPlugins(lt_preloaded_symbols, DEMAND_LOADING);
  GvExitOnUsage = 1;
  int argc = 2;
  char *argv[] = {"dot", "-Vrandom"};

  gvParseArgs(Gvc, argc, argv);

  // fail this test if the function above does not call exit
  assert(false);

  return 0;
}