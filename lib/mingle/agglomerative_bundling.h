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

#include <vector>

void agglomerative_ink_bundling(int dim, SparseMatrix A,
                                std::vector<pedge> &edges, int nneighbor,
                                int max_recursion, double angle_param,
                                double angle);
