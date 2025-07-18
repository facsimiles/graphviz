#pragma once

#include <stddef.h>
#include <util/bitarray.h>

#ifdef __cplusplus
extern "C" {
#endif

#if !defined(__CYGWIN__) && defined(__GNUC__) && !defined(__MINGW32__)
#define INTERNAL __attribute__((visibility("hidden")))
#else
#define INTERNAL /* nothing */
#endif

typedef struct {
    int i, j;
    float d, w;
} term_sgd;

typedef struct graph_sgd {
    size_t n; // number of nodes
    size_t *sources; // index of first edge in *targets for each node (length n+1)
    bitarray_t pinneds; // whether a node is fixed or not

    size_t *targets; // index of targets for each node (length sources[n])
    float *weights; // weights of edges (length sources[n])
} graph_sgd;

INTERNAL void sgd(graph_t *, int);

#undef INTERNAL

#ifdef __cplusplus
}
#endif
