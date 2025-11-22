/*************************************************************************
 * Copyright (c) 2011 AT&T Intellectual Property 
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * https://www.eclipse.org/legal/epl-v10.html
 *
 * Contributors: Details at https://graphviz.org
 *************************************************************************/


/******************************************

	Dijkstra algorithm
	Computes single-source distances for
	weighted graphs

******************************************/

#include <assert.h>
#include <float.h>
#include <neatogen/bfs.h>
#include <neatogen/dijkstra.h>
#include <limits.h>
#include <stdbool.h>
#include <stdlib.h>
#include <util/alloc.h>
#include <util/gv_math.h>
#include <util/bitarray.h>

typedef DistType Word;

#define MAX_DIST ((DistType)INT_MAX)

/* This heap class is suited to the Dijkstra alg.
   data[i]=vertexNum <==> index[vertexNum]=i
*/

static int left(int i) { return 2 * i; }

static int right(int i) { return 2 * i + 1; }

static int parent(int i) { return i / 2; }

typedef struct {
    int *data;
    int heapSize;
    int *index;
} heap;

static bool insideHeap(const heap *h, int i) { return i < h->heapSize; }

static bool greaterPriority(const heap *h, int i, int j, const Word *dist) {
  return dist[h->data[i]] < dist[h->data[j]];
}

static bool greaterPriority_f(const heap *h, int i, int j, const float *dist) {
  return dist[h->data[i]] < dist[h->data[j]];
}

static void assign(heap *h, int i, int j) {
  h->data[i] = h->data[j];
  h->index[h->data[i]] = i;
}

static void exchange(heap *h, int i, int j) {
  SWAP(&h->data[i], &h->data[j]);
  h->index[h->data[i]] = i;
  h->index[h->data[j]] = j;
}

static void heapify(heap *h, int i, Word dist[]) {
    int l, r, largest;
    while (1) {
	l = left(i);
	r = right(i);
	if (insideHeap(h, l) && greaterPriority(h, l, i, dist))
	    largest = l;
	else
	    largest = i;
	if (insideHeap(h, r) && greaterPriority(h, r, largest, dist))
	    largest = r;

	if (largest == i)
	    break;

	exchange(h, largest, i);
	i = largest;
    }
}

static void freeHeap(heap * h)
{
    free(h->index);
    free(h->data);
}

static heap initHeap(int startVertex, Word dist[], int n) {
    int i, count;
    int j;    /* We cannot use an unsigned value in this loop */
    heap h = {
      .data = gv_calloc(n - 1, sizeof(int)),
      .heapSize = n - 1,
      .index = gv_calloc(n, sizeof(int))
    };

    for (count = 0, i = 0; i < n; i++)
	if (i != startVertex) {
	    h.data[count] = i;
	    h.index[i] = count;
	    count++;
	}

    for (j = (n - 1) / 2; j >= 0; j--)
	heapify(&h, j, dist);

    return h;
}

static bool extractMax(heap *h, int *max, Word dist[]) {
    if (h->heapSize == 0)
	return false;

    *max = h->data[0];
    h->data[0] = h->data[h->heapSize - 1];
    h->index[h->data[0]] = 0;
    h->heapSize--;
    heapify(h, 0, dist);

    return true;
}

static void increaseKey(heap *h, int increasedVertex, Word newDist, Word dist[]) {
    int placeInHeap;
    int i;

    if (dist[increasedVertex] <= newDist)
	return;

    placeInHeap = h->index[increasedVertex];

    dist[increasedVertex] = newDist;

    i = placeInHeap;
    while (i > 0 && dist[h->data[parent(i)]] > newDist) {	/* can write here: greaterPriority(i,parent(i),dist) */
	assign(h, i, parent(i));
	i = parent(i);
    }
    h->data[i] = increasedVertex;
    h->index[increasedVertex] = i;
}

void ngdijkstra(int vertex, vtx_data * graph, int n, DistType * dist)
{
    int closestVertex, neighbor;
    DistType closestDist, prevClosestDist = MAX_DIST;

    /* initial distances with edge weights: */
    for (int i = 0; i < n; i++)
	dist[i] = MAX_DIST;
    dist[vertex] = 0;
    for (size_t i = 1; i < graph[vertex].nedges; i++)
	dist[graph[vertex].edges[i]] = (DistType) graph[vertex].ewgts[i];

    heap H = initHeap(vertex, dist, n);

    while (extractMax(&H, &closestVertex, dist)) {
	closestDist = dist[closestVertex];
	if (closestDist == MAX_DIST)
	    break;
	for (size_t i = 1; i < graph[closestVertex].nedges; i++) {
	    neighbor = graph[closestVertex].edges[i];
	    increaseKey(&H, neighbor, closestDist +
			(DistType)graph[closestVertex].ewgts[i], dist);
	}
	prevClosestDist = closestDist;
    }

    /* For dealing with disconnected graphs: */
    for (int i = 0; i < n; i++)
	if (dist[i] == MAX_DIST)	/* 'i' is not connected to 'vertex' */
	    dist[i] = prevClosestDist + 10;
    freeHeap(&H);
}

static void heapify_f(heap *h, int i, float dist[]) {
    int l, r, largest;
    while (1) {
	l = left(i);
	r = right(i);
	if (insideHeap(h, l) && greaterPriority_f(h, l, i, dist))
	    largest = l;
	else
	    largest = i;
	if (insideHeap(h, r) && greaterPriority_f(h, r, largest, dist))
	    largest = r;

	if (largest == i)
	    break;

	exchange(h, largest, i);
	i = largest;
    }
}

static heap initHeap_f(int startVertex, float dist[], int n) {
    int i, count;
    int j;			/* We cannot use an unsigned value in this loop */
    heap h = {
      .data = gv_calloc(n - 1, sizeof(int)),
      .heapSize = n - 1,
      .index = gv_calloc(n, sizeof(int))
    };

    for (count = 0, i = 0; i < n; i++)
	if (i != startVertex) {
	    h.data[count] = i;
	    h.index[i] = count;
	    count++;
	}

    for (j = (n - 1) / 2; j >= 0; j--)
	heapify_f(&h, j, dist);

    return h;
}

static bool extractMax_f(heap *h, int *max, float dist[]) {
    if (h->heapSize == 0)
	return false;

    *max = h->data[0];
    h->data[0] = h->data[h->heapSize - 1];
    h->index[h->data[0]] = 0;
    h->heapSize--;
    heapify_f(h, 0, dist);

    return true;
}

static void increaseKey_f(heap *h, int increasedVertex, float newDist,
                          float dist[]) {
    int placeInHeap;
    int i;

    if (dist[increasedVertex] <= newDist)
	return;

    placeInHeap = h->index[increasedVertex];

    dist[increasedVertex] = newDist;

    i = placeInHeap;
    while (i > 0 && dist[h->data[parent(i)]] > newDist) {	/* can write here: greaterPriority(i,parent(i),dist) */
	assign(h, i, parent(i));
	i = parent(i);
    }
    h->data[i] = increasedVertex;
    h->index[increasedVertex] = i;
}

/* Weighted shortest paths from vertex.
 * Assume graph is connected.
 */
void dijkstra_f(int vertex, vtx_data * graph, int n, float *dist)
{
    int closestVertex = 0, neighbor;
    float closestDist;

    /* initial distances with edge weights: */
    for (int i = 0; i < n; i++)
	dist[i] = FLT_MAX;
    dist[vertex] = 0;
    for (size_t i = 1; i < graph[vertex].nedges; i++)
	dist[graph[vertex].edges[i]] = graph[vertex].ewgts[i];

    heap H = initHeap_f(vertex, dist, n);

    while (extractMax_f(&H, &closestVertex, dist)) {
	closestDist = dist[closestVertex];
	if (closestDist == FLT_MAX)
	    break;
	for (size_t i = 1; i < graph[closestVertex].nedges; i++) {
	    neighbor = graph[closestVertex].edges[i];
	    increaseKey_f(&H, neighbor, closestDist + graph[closestVertex].ewgts[i],
			  dist);
	}
    }

    freeHeap(&H);
}

// single source shortest paths that also builds terms as it goes
// mostly copied from dijkstra_f above
// returns the number of terms built
int dijkstra_sgd(graph_sgd *graph, int source, term_sgd *terms) {
    float *dists = gv_calloc(graph->n, sizeof(float));
    for (size_t i= 0; i < graph->n; i++) {
        dists[i] = FLT_MAX;
    }
    dists[source] = 0;
    for (size_t i = graph->sources[source]; i < graph->sources[source + 1];
         i++) {
        size_t target = graph->targets[i];
        dists[target] = graph->weights[i];
    }
    assert(graph->n <= INT_MAX);
    heap h = initHeap_f(source, dists, (int)graph->n);

    int closest = 0, offset = 0;
    while (extractMax_f(&h, &closest, dists)) {
        float d = dists[closest];
        if (d == FLT_MAX) {
            break;
        }
        // if the target is fixed then always create a term as shortest paths are not calculated from there
        // if not fixed then only create a term if the target index is lower
        if (bitarray_get(graph->pinneds, closest) || closest<source) {
            terms[offset].i = source;
            terms[offset].j = closest;
            terms[offset].d = d;
            terms[offset].w = 1 / (d*d);
            offset++;
        }
        for (size_t i = graph->sources[closest]; i < graph->sources[closest + 1];
             i++) {
            size_t target = graph->targets[i];
            float weight = graph->weights[i];
            assert(target <= INT_MAX);
            increaseKey_f(&h, (int)target, d+weight, dists);
        }
    }
    freeHeap(&h);
    free(dists);
    return offset;
}
