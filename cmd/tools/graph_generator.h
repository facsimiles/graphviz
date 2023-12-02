/*************************************************************************
 * Copyright (c) 2011 AT&T Intellectual Property
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v2.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v20.html
 *
 * Contributors: Details at https://graphviz.org
 *************************************************************************/

#pragma once

typedef void (*edgefn)(unsigned, unsigned);

extern void makeBall(unsigned, unsigned, edgefn);
extern void makeCircle(unsigned, edgefn);
extern void makeComplete(unsigned, edgefn);
extern void makeCompleteB(unsigned, unsigned, edgefn);
extern void makePath(unsigned, edgefn);
extern void makeStar(unsigned, edgefn);
extern void makeWheel(unsigned, edgefn);
extern void makeTorus(unsigned, unsigned, edgefn);
extern void makeTwistedTorus(unsigned, unsigned, unsigned, unsigned, edgefn);
extern void makeCylinder(unsigned, unsigned, edgefn);
extern void makeRandom(unsigned, unsigned, edgefn);
extern void makeSquareGrid(unsigned, unsigned, int, int, edgefn);
extern void makeBinaryTree(unsigned, edgefn);
extern void makeSierpinski(unsigned, edgefn);
extern void makeTetrix(unsigned, edgefn);
extern void makeHypercube(unsigned, edgefn);
extern void makeTree(unsigned, unsigned, edgefn);
extern void makeTriMesh(unsigned, edgefn);
extern void makeMobius(unsigned, unsigned, edgefn);

typedef struct treegen_s treegen_t;
extern treegen_t *makeTreeGen(unsigned);
extern void makeRandomTree(treegen_t *, edgefn);
extern void freeTreeGen(treegen_t *);
