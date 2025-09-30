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

	Breadth First Search
	Computes single-source distances for
	unweighted graphs

******************************************/

#include <neatogen/bfs.h>
#include <util/list.h>

void bfs(int vertex, vtx_data *graph, int n, DistType *dist)
 /* compute vector 'dist' of distances of all nodes from 'vertex' */
{
    int closestVertex, neighbor;
    DistType closestDist = INT_MAX;

    /* initial distances with edge weights: */
    for (int i = 0; i < n; i++)
	dist[i] = -1;
    dist[vertex] = 0;

    LIST(int) Q = {0};
    LIST_PUSH_BACK(&Q, vertex);

    while (!LIST_IS_EMPTY(&Q)) {
        closestVertex = LIST_POP_FRONT(&Q);
        closestDist = dist[closestVertex];
        for (size_t i = 1; i < graph[closestVertex].nedges; i++) {
            neighbor = graph[closestVertex].edges[i];
            if (dist[neighbor] < -0.5) { // first time to reach neighbor
                const DistType bump = graph[0].ewgts == NULL
                  ? 1
                  : (DistType)graph[closestVertex].ewgts[i];
                dist[neighbor] = closestDist + bump;
                LIST_PUSH_BACK(&Q, neighbor);
            }
        }
    }

    /* For dealing with disconnected graphs: */
    for (int i = 0; i < n; i++)
	if (dist[i] < -0.5)	/* 'i' is not connected to 'vertex' */
	    dist[i] = closestDist + 10;

    LIST_FREE(&Q);
}
