/**
 * @file
 * @brief import <a href=https://math.nist.gov/MatrixMarket/>Matrix Market</a>
 */

/*************************************************************************
 * Copyright (c) 2011 AT&T Intellectual Property 
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v2.0
 * which accompanies this distribution, and is available at
 * https://www.eclipse.org/org/documents/epl-2.0/EPL-2.0.html
 *
 * Contributors: Details at https://graphviz.org
 *************************************************************************/

#pragma once

#include "mmio.h"
#include <sparse/SparseMatrix.h>
SparseMatrix SparseMatrix_import_matrix_market(FILE * f);
