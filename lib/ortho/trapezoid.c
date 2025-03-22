/**
 * @file
 * @brief trapezoidation
 *
 * See [Fast polygon triangulation based on Seidel's algorithm](http://gamma.cs.unc.edu/SEIDEL/)
 *
 */

/*************************************************************************
 * Copyright (c) 2011 AT&T Intellectual Property
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * https://www.eclipse.org/legal/epl-v10.html
 *
 * Contributors: Details at https://graphviz.org
 *************************************************************************/


#include "config.h"
#include <string.h>
#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <math.h>
#include <common/geom.h>
#include <common/types.h>
#include <ortho/trap.h>
#include <util/alloc.h>
#include <util/gv_math.h>
#include <util/list.h>
#include <util/unreachable.h>

/* Node types */

#define T_X     1
#define T_Y     2
#define T_SINK  3

#define FIRSTPT 1       /* checking whether pt. is inserted */
#define LASTPT  2

#define S_LEFT 1        /* for merge-direction */
#define S_RIGHT 2

#define INF 1<<30

#define CROSS(v0, v1, v2) (((v1).x - (v0).x)*((v2).y - (v0).y) - \
               ((v1).y - (v0).y)*((v2).x - (v0).x))

typedef struct {
  int nodetype;         /* Y-node or S-node */
  int segnum;
  pointf yval;
  size_t trnum;
  size_t parent;        ///< doubly linked DAG
  size_t left, right;   ///< children
} qnode_t;

/// an array of qnodes
DEFINE_LIST(qnodes, qnode_t)

/* Return a new node to be added into the query tree */
static size_t newnode(qnodes_t *qs) {
  qnodes_append(qs, (qnode_t){0});
  return qnodes_size(qs) - 1;
}

/* Return a free trapezoid */
static size_t newtrap(traps_t *tr) {
  tr->data = gv_recalloc(tr->data, tr->length, tr->length + 1, sizeof(trap_t));
  ++tr->length;
  return tr->length - 1;
}

/* Return the maximum of the two points into the yval structure */
static void _max (pointf *yval, pointf *v0, pointf *v1)
{
  if (v0->y > v1->y + C_EPS)
    *yval = *v0;
  else if (FP_EQUAL(v0->y, v1->y))
    {
      if (v0->x > v1->x + C_EPS)
	*yval = *v0;
      else
	*yval = *v1;
    }
  else
    *yval = *v1;
}

/* Return the minimum of the two points into the yval structure */
static void _min (pointf *yval, pointf *v0, pointf *v1)
{
  if (v0->y < v1->y - C_EPS)
    *yval = *v0;
  else if (FP_EQUAL(v0->y, v1->y))
    {
      if (v0->x < v1->x)
	*yval = *v0;
      else
	*yval = *v1;
    }
  else
    *yval = *v1;
}

static bool _greater_than_equal_to (pointf *v0, pointf *v1)
{
  if (v0->y > v1->y + C_EPS)
    return true;
  else if (v0->y < v1->y - C_EPS)
    return false;
  else
    return v0->x >= v1->x;
}

static bool _less_than (pointf *v0, pointf *v1)
{
  return !_greater_than_equal_to(v0, v1);
}

/* Initialize the query structure (Q) and the trapezoid table (T)
 * when the first segment is added to start the trapezoidation. The
 * query-tree starts out with 4 trapezoids, one S-node and 2 Y-nodes
 *
 *                4
 *   -----------------------------------
 *  		  \
 *  	1	   \        2
 *  		    \
 *   -----------------------------------
 *                3
 */

static size_t init_query_structure(int segnum, segment_t *seg, traps_t *tr,
                                   qnodes_t *qs) {
  segment_t *s = &seg[segnum];

  const size_t i1 = newnode(qs);
  qnodes_at(qs, i1)->nodetype = T_Y;
  _max(&qnodes_at(qs, i1)->yval, &s->v0, &s->v1); // root
  const size_t root = i1;

  const size_t i2 = newnode(qs);
  qnodes_at(qs, i1)->right = i2;
  qnodes_at(qs, i2)->nodetype = T_SINK;
  qnodes_at(qs, i2)->parent = i1;

  const size_t i3 = newnode(qs);
  qnodes_at(qs, i1)->left = i3;
  qnodes_at(qs, i3)->nodetype = T_Y;
  _min(&qnodes_at(qs, i3)->yval, &s->v0, &s->v1); // root
  qnodes_at(qs, i3)->parent = i1;

  const size_t i4 = newnode(qs);
  qnodes_at(qs, i3)->left = i4;
  qnodes_at(qs, i4)->nodetype = T_SINK;
  qnodes_at(qs, i4)->parent = i3;

  const size_t i5 = newnode(qs);
  qnodes_at(qs, i3)->right = i5;
  qnodes_at(qs, i5)->nodetype = T_X;
  qnodes_at(qs, i5)->segnum = segnum;
  qnodes_at(qs, i5)->parent = i3;

  const size_t i6 = newnode(qs);
  qnodes_at(qs, i5)->left = i6;
  qnodes_at(qs, i6)->nodetype = T_SINK;
  qnodes_at(qs, i6)->parent = i5;

  const size_t i7 = newnode(qs);
  qnodes_at(qs, i5)->right = i7;
  qnodes_at(qs, i7)->nodetype = T_SINK;
  qnodes_at(qs, i7)->parent = i5;

  const size_t t1 = newtrap(tr); // middle left
  const size_t t2 = newtrap(tr); // middle right
  const size_t t3 = newtrap(tr); // bottom-most
  const size_t t4 = newtrap(tr); // topmost

  tr->data[t1].hi = qnodes_get(qs, i1).yval;
  tr->data[t2].hi = qnodes_get(qs, i1).yval;
  tr->data[t4].lo = qnodes_get(qs, i1).yval;
  tr->data[t1].lo = qnodes_get(qs, i3).yval;
  tr->data[t2].lo = qnodes_get(qs, i3).yval;
  tr->data[t3].hi = qnodes_get(qs, i3).yval;
  tr->data[t4].hi.y = (double)(INF);
  tr->data[t4].hi.x = (double)(INF);
  tr->data[t3].lo.y = (double)-1 * (INF);
  tr->data[t3].lo.x = (double)-1 * (INF);
  tr->data[t1].rseg = segnum;
  tr->data[t2].lseg = segnum;
  tr->data[t1].u0 = t4;
  tr->data[t2].u0 = t4;
  tr->data[t1].d0 = t3;
  tr->data[t2].d0 = t3;
  tr->data[t4].d0 = t1;
  tr->data[t3].u0 = t1;
  tr->data[t4].d1 = t2;
  tr->data[t3].u1 = t2;

  tr->data[t1].sink = i6;
  tr->data[t2].sink = i7;
  tr->data[t3].sink = i4;
  tr->data[t4].sink = i2;

  tr->data[t1].state = ST_VALID;
  tr->data[t2].state = ST_VALID;
  tr->data[t3].state = ST_VALID;
  tr->data[t4].state = ST_VALID;

  qnodes_at(qs, i2)->trnum = t4;
  qnodes_at(qs, i4)->trnum = t3;
  qnodes_at(qs, i6)->trnum = t1;
  qnodes_at(qs, i7)->trnum = t2;

  s->is_inserted = true;
  return root;
}

/* Return true if the vertex v is to the left of line segment no.
 * segnum. Takes care of the degenerate cases when both the vertices
 * have the same y--cood, etc.
 */
static bool
is_left_of (int segnum, segment_t* seg, pointf *v)
{
  segment_t *s = &seg[segnum];
  double area;

  if (_greater_than(&s->v1, &s->v0)) /* seg. going upwards */
    {
      if (FP_EQUAL(s->v1.y, v->y))
	{
	  if (v->x < s->v1.x)
	    area = 1.0;
	  else
	    area = -1.0;
	}
      else if (FP_EQUAL(s->v0.y, v->y))
	{
	  if (v->x < s->v0.x)
	    area = 1.0;
	  else
	    area = -1.0;
	}
      else
	area = CROSS(s->v0, s->v1, *v);
    }
  else				/* v0 > v1 */
    {
      if (FP_EQUAL(s->v1.y, v->y))
	{
	  if (v->x < s->v1.x)
	    area = 1.0;
	  else
	    area = -1.0;
	}
      else if (FP_EQUAL(s->v0.y, v->y))
	{
	  if (v->x < s->v0.x)
	    area = 1.0;
	  else
	    area = -1.0;
	}
      else
	area = CROSS(s->v1, s->v0, (*v));
    }

  return area > 0.0;
}

/* Returns true if the corresponding endpoint of the given segment is */
/* already inserted into the segment tree. Use the simple test of */
/* whether the segment which shares this endpoint is already inserted */
static bool inserted (int segnum, segment_t* seg, int whichpt)
{
  if (whichpt == FIRSTPT)
    return seg[seg[segnum].prev].is_inserted;
  else
    return seg[seg[segnum].next].is_inserted;
}

/* This is query routine which determines which trapezoid does the
 * point v lie in. The return value is the trapezoid number.
 */
static size_t locate_endpoint(pointf *v, pointf *vo, size_t r, segment_t *seg,
                           qnodes_t *qs) {
  qnode_t *rptr = qnodes_at(qs, r);

  switch (rptr->nodetype) {
    case T_SINK:
      return rptr->trnum;

    case T_Y:
      if (_greater_than(v, &rptr->yval)) /* above */
	return locate_endpoint(v, vo, rptr->right, seg, qs);
      if (_equal_to(v, &rptr->yval)) { // the point is already inserted
	  if (_greater_than(vo, &rptr->yval)) /* above */
	    return locate_endpoint(v, vo, rptr->right, seg, qs);
	  return locate_endpoint(v, vo, rptr->left, seg, qs); // below
	}
      return locate_endpoint(v, vo, rptr->left, seg, qs); // below

    case T_X:
      if (_equal_to(v, &seg[rptr->segnum].v0) ||
	       _equal_to(v, &seg[rptr->segnum].v1))
	{
	  if (FP_EQUAL(v->y, vo->y)) /* horizontal segment */
	    {
	      if (vo->x < v->x)
		return locate_endpoint(v, vo, rptr->left, seg, qs); /* left */
	      return locate_endpoint(v, vo, rptr->right, seg, qs); // right
	    }

	  if (is_left_of(rptr->segnum, seg, vo))
	    return locate_endpoint(v, vo, rptr->left, seg, qs); /* left */
	  return locate_endpoint(v, vo, rptr->right, seg, qs); // right
	}
      if (is_left_of(rptr->segnum, seg, v))
	return locate_endpoint(v, vo, rptr->left, seg, qs); /* left */
      return locate_endpoint(v, vo, rptr->right, seg, qs); // right

    default:
      break;
    }
    UNREACHABLE();
}

/* Thread in the segment into the existing trapezoidation. The
 * limiting trapezoids are given by tfirst and tlast (which are the
 * trapezoids containing the two endpoints of the segment. Merges all
 * possible trapezoids which flank this segment and have been recently
 * divided because of its insertion
 */
static void merge_trapezoids(int segnum, size_t tfirst, size_t tlast, int side,
                             traps_t *tr, qnodes_t *qs) {
  /* First merge polys on the LHS */
  size_t t = tfirst;
  while (is_valid_trap(t) && _greater_than_equal_to(&tr->data[t].lo, &tr->data[tlast].lo))
    {
      size_t tnext;
      bool cond;
      if (side == S_LEFT)
	cond = (is_valid_trap(tnext = tr->data[t].d0) && tr->data[tnext].rseg == segnum) ||
		(is_valid_trap(tnext = tr->data[t].d1) && tr->data[tnext].rseg == segnum);
      else
	cond = (is_valid_trap(tnext = tr->data[t].d0) && tr->data[tnext].lseg == segnum) ||
		(is_valid_trap(tnext = tr->data[t].d1) && tr->data[tnext].lseg == segnum);

      if (cond)
	{
	  if (tr->data[t].lseg == tr->data[tnext].lseg &&
	      tr->data[t].rseg == tr->data[tnext].rseg) /* good neighbours */
	    {			              /* merge them */
	      /* Use the upper node as the new node i.e. t */

	      const size_t ptnext = qnodes_get(qs, tr->data[tnext].sink).parent;

	      if (qnodes_get(qs, ptnext).left == tr->data[tnext].sink)
		qnodes_at(qs, ptnext)->left = tr->data[t].sink;
	      else
		qnodes_at(qs, ptnext)->right = tr->data[t].sink; // redirect parent


	      /* Change the upper neighbours of the lower trapezoids */

	      if (is_valid_trap(tr->data[t].d0 = tr->data[tnext].d0)) {
		if (tr->data[tr->data[t].d0].u0 == tnext)
		  tr->data[tr->data[t].d0].u0 = t;
		else if (tr->data[tr->data[t].d0].u1 == tnext)
		  tr->data[tr->data[t].d0].u1 = t;
	      }

	      if (is_valid_trap(tr->data[t].d1 = tr->data[tnext].d1)) {
		if (tr->data[tr->data[t].d1].u0 == tnext)
		  tr->data[tr->data[t].d1].u0 = t;
		else if (tr->data[tr->data[t].d1].u1 == tnext)
		  tr->data[tr->data[t].d1].u1 = t;
	      }

	      tr->data[t].lo = tr->data[tnext].lo;
	      tr->data[tnext].state = ST_INVALID; /* invalidate the lower */
				            /* trapezium */
	    }
	  else		    /* not good neighbours */
	    t = tnext;
	}
      else		    /* do not satisfy the outer if */
	t = tnext;

    } /* end-while */

}

static void update_trapezoid(segment_t *s, segment_t *seg, traps_t *tr,
                             size_t t, size_t tn) {
  if (is_valid_trap(tr->data[t].u0) && is_valid_trap(tr->data[t].u1))
  {			/* continuation of a chain from abv. */
    if (is_valid_trap(tr->data[t].usave)) { // three upper neighbours
      if (tr->data[t].uside == S_LEFT)
      {
	tr->data[tn].u0 = tr->data[t].u1;
	tr->data[t].u1 = SIZE_MAX;
	tr->data[tn].u1 = tr->data[t].usave;

	tr->data[tr->data[t].u0].d0 = t;
	tr->data[tr->data[tn].u0].d0 = tn;
	tr->data[tr->data[tn].u1].d0 = tn;
      }
      else		/* intersects in the right */
      {
	tr->data[tn].u1 = SIZE_MAX;
	tr->data[tn].u0 = tr->data[t].u1;
	tr->data[t].u1 = tr->data[t].u0;
	tr->data[t].u0 = tr->data[t].usave;

	tr->data[tr->data[t].u0].d0 = t;
	tr->data[tr->data[t].u1].d0 = t;
	tr->data[tr->data[tn].u0].d0 = tn;
      }

      tr->data[t].usave = 0;
      tr->data[tn].usave = 0;
    }
    else		/* No usave.... simple case */
    {
      tr->data[tn].u0 = tr->data[t].u1;
      tr->data[t].u1 = SIZE_MAX;
      tr->data[tn].u1 = SIZE_MAX;
      tr->data[tr->data[tn].u0].d0 = tn;
    }
  }
  else
  {			/* fresh seg. or upward cusp */
    const size_t tmp_u = tr->data[t].u0;
    size_t td0;
    if (is_valid_trap(td0 = tr->data[tmp_u].d0) && is_valid_trap(tr->data[tmp_u].d1))
    {		/* upward cusp */
      if (tr->data[td0].rseg > 0 && !is_left_of(tr->data[td0].rseg, seg, &s->v1))
      {
	tr->data[t].u0 = SIZE_MAX;
	tr->data[t].u1 = SIZE_MAX;
	tr->data[tn].u1 = SIZE_MAX;
	tr->data[tr->data[tn].u0].d1 = tn;
      }
      else		/* cusp going leftwards */
      {
	tr->data[tn].u0 = SIZE_MAX;
	tr->data[tn].u1 = SIZE_MAX;
	tr->data[t].u1 = SIZE_MAX;
	tr->data[tr->data[t].u0].d0 = t;
      }
    }
    else		/* fresh segment */
    {
      tr->data[tr->data[t].u0].d0 = t;
      tr->data[tr->data[t].u0].d1 = tn;
    }
  }
}

/* Add in the new segment into the trapezoidation and update Q and T
 * structures. First locate the two endpoints of the segment in the
 * Q-structure. Then start from the topmost trapezoid and go down to
 * the  lower trapezoid dividing all the trapezoids in between .
 */
static void add_segment(int segnum, segment_t *seg, traps_t *tr, qnodes_t *qs) {
  segment_t s;
  size_t tfirst, tlast;
  size_t tfirstr = 0, tlastr = 0;
  int tribot = 0;
  bool is_swapped;
  int tmptriseg;

  s = seg[segnum];
  if (_greater_than(&s.v1, &s.v0)) /* Get higher vertex in v0 */
    {
      SWAP(&s.v0, &s.v1);
      SWAP(&s.root0, &s.root1);
      is_swapped = true;
    }
  else is_swapped = false;

  if (!inserted(segnum, seg, is_swapped ? LASTPT : FIRSTPT))
    /* insert v0 in the tree */
    {
      size_t tmp_d;

      const size_t tu = locate_endpoint(&s.v0, &s.v1, s.root0, seg, qs);
      const size_t tl = newtrap(tr); // tl is the new lower trapezoid
      tr->data[tl] = tr->data[tu];
      tr->data[tu].lo = s.v0;
      tr->data[tl].hi = s.v0;
      tr->data[tu].d0 = tl;
      tr->data[tu].d1 = 0;
      tr->data[tl].u0 = tu;
      tr->data[tl].u1 = 0;

      if (is_valid_trap(tmp_d = tr->data[tl].d0) && tr->data[tmp_d].u0 == tu)
	tr->data[tmp_d].u0 = tl;
      if (is_valid_trap(tmp_d = tr->data[tl].d0) && tr->data[tmp_d].u1 == tu)
	tr->data[tmp_d].u1 = tl;

      if (is_valid_trap(tmp_d = tr->data[tl].d1) && tr->data[tmp_d].u0 == tu)
	tr->data[tmp_d].u0 = tl;
      if (is_valid_trap(tmp_d = tr->data[tl].d1) && tr->data[tmp_d].u1 == tu)
	tr->data[tmp_d].u1 = tl;

      /* Now update the query structure and obtain the sinks for the */
      /* two trapezoids */

      const size_t i1 = newnode(qs); // Upper trapezoid sink
      const size_t i2 = newnode(qs); // Lower trapezoid sink
      const size_t sk = tr->data[tu].sink;

      qnodes_at(qs, sk)->nodetype = T_Y;
      qnodes_at(qs, sk)->yval = s.v0;
      qnodes_at(qs, sk)->segnum = segnum; // not really required … maybe later
      qnodes_at(qs, sk)->left = i2;
      qnodes_at(qs, sk)->right = i1;

      qnodes_at(qs, i1)->nodetype = T_SINK;
      qnodes_at(qs, i1)->trnum = tu;
      qnodes_at(qs, i1)->parent = sk;

      qnodes_at(qs, i2)->nodetype = T_SINK;
      qnodes_at(qs, i2)->trnum = tl;
      qnodes_at(qs, i2)->parent = sk;

      tr->data[tu].sink = i1;
      tr->data[tl].sink = i2;
      tfirst = tl;
    }
  else				/* v0 already present */
    {       /* Get the topmost intersecting trapezoid */
      tfirst = locate_endpoint(&s.v0, &s.v1, s.root0, seg, qs);
    }


  if (!inserted(segnum, seg, is_swapped ? FIRSTPT : LASTPT))
    /* insert v1 in the tree */
    {
      size_t tmp_d;

      const size_t tu = locate_endpoint(&s.v1, &s.v0, s.root1, seg, qs);

      const size_t tl = newtrap(tr); // tl is the new lower trapezoid
      tr->data[tl] = tr->data[tu];
      tr->data[tu].lo = s.v1;
      tr->data[tl].hi = s.v1;
      tr->data[tu].d0 = tl;
      tr->data[tu].d1 = 0;
      tr->data[tl].u0 = tu;
      tr->data[tl].u1 = 0;

      if (is_valid_trap(tmp_d = tr->data[tl].d0) && tr->data[tmp_d].u0 == tu)
	tr->data[tmp_d].u0 = tl;
      if (is_valid_trap(tmp_d = tr->data[tl].d0) && tr->data[tmp_d].u1 == tu)
	tr->data[tmp_d].u1 = tl;

      if (is_valid_trap(tmp_d = tr->data[tl].d1) && tr->data[tmp_d].u0 == tu)
	tr->data[tmp_d].u0 = tl;
      if (is_valid_trap(tmp_d = tr->data[tl].d1) && tr->data[tmp_d].u1 == tu)
	tr->data[tmp_d].u1 = tl;

      /* Now update the query structure and obtain the sinks for the */
      /* two trapezoids */

      const size_t i1 = newnode(qs); // Upper trapezoid sink
      const size_t i2 = newnode(qs); // Lower trapezoid sink
      const size_t sk = tr->data[tu].sink;

      qnodes_at(qs, sk)->nodetype = T_Y;
      qnodes_at(qs, sk)->yval = s.v1;
      qnodes_at(qs, sk)->segnum = segnum; // not really required … maybe later
      qnodes_at(qs, sk)->left = i2;
      qnodes_at(qs, sk)->right = i1;

      qnodes_at(qs, i1)->nodetype = T_SINK;
      qnodes_at(qs, i1)->trnum = tu;
      qnodes_at(qs, i1)->parent = sk;

      qnodes_at(qs, i2)->nodetype = T_SINK;
      qnodes_at(qs, i2)->trnum = tl;
      qnodes_at(qs, i2)->parent = sk;

      tr->data[tu].sink = i1;
      tr->data[tl].sink = i2;
      tlast = tu;
    }
  else				/* v1 already present */
    {       /* Get the lowermost intersecting trapezoid */
      tlast = locate_endpoint(&s.v1, &s.v0, s.root1, seg, qs);
      tribot = 1;
    }

  /* Thread the segment into the query tree creating a new X-node */
  /* First, split all the trapezoids which are intersected by s into */
  /* two */

  size_t t = tfirst; // topmost trapezoid

  while (is_valid_trap(t) && _greater_than_equal_to(&tr->data[t].lo, &tr->data[tlast].lo))
				/* traverse from top to bot */
    {
      const size_t sk = tr->data[t].sink;
      const size_t i1 = newnode(qs); // left trapezoid sink
      const size_t i2 = newnode(qs); // right trapezoid sink

      qnodes_at(qs, sk)->nodetype = T_X;
      qnodes_at(qs, sk)->segnum = segnum;
      qnodes_at(qs, sk)->left = i1;
      qnodes_at(qs, sk)->right = i2;

      qnodes_at(qs, i1)->nodetype = T_SINK; // left trapezoid (use existing one)
      qnodes_at(qs, i1)->trnum = t;
      qnodes_at(qs, i1)->parent = sk;

      qnodes_at(qs, i2)->nodetype = T_SINK; // right trapezoid (allocate new)
      const size_t tn = newtrap(tr);
      qnodes_at(qs, i2)->trnum = tn;
      tr->data[tn].state = ST_VALID;
      qnodes_at(qs, i2)->parent = sk;

      if (t == tfirst)
	tfirstr = tn;
      if (_equal_to(&tr->data[t].lo, &tr->data[tlast].lo))
	tlastr = tn;

      tr->data[tn] = tr->data[t];
      tr->data[t].sink = i1;
      tr->data[tn].sink = i2;
      const size_t t_sav = t;
      const size_t tn_sav = tn;

      /* error */

      if (!is_valid_trap(tr->data[t].d0) && !is_valid_trap(tr->data[t].d1)) // case cannot arise
	{
	  fprintf(stderr, "add_segment: error\n");
	  break;
	}

      /* only one trapezoid below. partition t into two and make the */
      /* two resulting trapezoids t and tn as the upper neighbours of */
      /* the sole lower trapezoid */

      else if (is_valid_trap(tr->data[t].d0) && !is_valid_trap(tr->data[t].d1))
	{			/* Only one trapezoid below */
	  update_trapezoid(&s, seg, tr, t, tn);

	  if (FP_EQUAL(tr->data[t].lo.y, tr->data[tlast].lo.y) &&
	      FP_EQUAL(tr->data[t].lo.x, tr->data[tlast].lo.x) && tribot)
	    {		/* bottom forms a triangle */

	      if (is_swapped)
		tmptriseg = seg[segnum].prev;
	      else
		tmptriseg = seg[segnum].next;

	      if (tmptriseg > 0 && is_left_of(tmptriseg, seg, &s.v0))
		{
				/* L-R downward cusp */
		  tr->data[tr->data[t].d0].u0 = t;
		  tr->data[tn].d0 = SIZE_MAX;
		  tr->data[tn].d1 = SIZE_MAX;
		}
	      else
		{
				/* R-L downward cusp */
		  tr->data[tr->data[tn].d0].u1 = tn;
		  tr->data[t].d0 = SIZE_MAX;
		  tr->data[t].d1 = SIZE_MAX;
		}
	    }
	  else
	    {
	      if (is_valid_trap(tr->data[tr->data[t].d0].u0) && is_valid_trap(tr->data[tr->data[t].d0].u1))
		{
		  if (tr->data[tr->data[t].d0].u0 == t) /* passes through LHS */
		    {
		      tr->data[tr->data[t].d0].usave = tr->data[tr->data[t].d0].u1;
		      tr->data[tr->data[t].d0].uside = S_LEFT;
		    }
		  else
		    {
		      tr->data[tr->data[t].d0].usave = tr->data[tr->data[t].d0].u0;
		      tr->data[tr->data[t].d0].uside = S_RIGHT;
		    }
		}
	      tr->data[tr->data[t].d0].u0 = t;
	      tr->data[tr->data[t].d0].u1 = tn;
	    }

	  t = tr->data[t].d0;
	}


      else if (!is_valid_trap(tr->data[t].d0) && is_valid_trap(tr->data[t].d1))
	{			/* Only one trapezoid below */
	  update_trapezoid(&s, seg, tr, t, tn);

	  if (FP_EQUAL(tr->data[t].lo.y, tr->data[tlast].lo.y) &&
	      FP_EQUAL(tr->data[t].lo.x, tr->data[tlast].lo.x) && tribot)
	    {		/* bottom forms a triangle */

	      if (is_swapped)
		tmptriseg = seg[segnum].prev;
	      else
		tmptriseg = seg[segnum].next;

	      if (tmptriseg > 0 && is_left_of(tmptriseg, seg, &s.v0))
		{
		  /* L-R downward cusp */
		  tr->data[tr->data[t].d1].u0 = t;
		  tr->data[tn].d0 = SIZE_MAX;
		  tr->data[tn].d1 = SIZE_MAX;
		}
	      else
		{
		  /* R-L downward cusp */
		  tr->data[tr->data[tn].d1].u1 = tn;
		  tr->data[t].d0 = SIZE_MAX;
		  tr->data[t].d1 = SIZE_MAX;
		}
	    }
	  else
	    {
	      if (is_valid_trap(tr->data[tr->data[t].d1].u0) && is_valid_trap(tr->data[tr->data[t].d1].u1))
		{
		  if (tr->data[tr->data[t].d1].u0 == t) /* passes through LHS */
		    {
		      tr->data[tr->data[t].d1].usave = tr->data[tr->data[t].d1].u1;
		      tr->data[tr->data[t].d1].uside = S_LEFT;
		    }
		  else
		    {
		      tr->data[tr->data[t].d1].usave = tr->data[tr->data[t].d1].u0;
		      tr->data[tr->data[t].d1].uside = S_RIGHT;
		    }
		}
	      tr->data[tr->data[t].d1].u0 = t;
	      tr->data[tr->data[t].d1].u1 = tn;
	    }

	  t = tr->data[t].d1;
	}

      /* two trapezoids below. Find out which one is intersected by */
      /* this segment and proceed down that one */

      else
	{
	  double y0, yt;
	  pointf tmppt;
	  size_t tnext;
	  bool i_d0, i_d1;

	  i_d0 = i_d1 = false;
	  if (FP_EQUAL(tr->data[t].lo.y, s.v0.y))
	    {
	      if (tr->data[t].lo.x > s.v0.x)
		i_d0 = true;
	      else
		i_d1 = true;
	    }
	  else
	    {
	      tmppt.y = y0 = tr->data[t].lo.y;
	      yt = (y0 - s.v0.y)/(s.v1.y - s.v0.y);
	      tmppt.x = s.v0.x + yt * (s.v1.x - s.v0.x);

	      if (_less_than(&tmppt, &tr->data[t].lo))
		i_d0 = true;
	      else
		i_d1 = true;
	    }

	  /* check continuity from the top so that the lower-neighbour */
	  /* values are properly filled for the upper trapezoid */

	  update_trapezoid(&s, seg, tr, t, tn);

	  if (FP_EQUAL(tr->data[t].lo.y, tr->data[tlast].lo.y) &&
	      FP_EQUAL(tr->data[t].lo.x, tr->data[tlast].lo.x) && tribot)
	    {
	      /* this case arises only at the lowest trapezoid.. i.e.
		 tlast, if the lower endpoint of the segment is
		 already inserted in the structure */

	      tr->data[tr->data[t].d0].u0 = t;
	      tr->data[tr->data[t].d0].u1 = SIZE_MAX;
	      tr->data[tr->data[t].d1].u0 = tn;
	      tr->data[tr->data[t].d1].u1 = SIZE_MAX;

	      tr->data[tn].d0 = tr->data[t].d1;
	      tr->data[t].d1 = SIZE_MAX;
	      tr->data[tn].d1 = SIZE_MAX;

	      tnext = tr->data[t].d1;
	    }
	  else if (i_d0)
				/* intersecting d0 */
	    {
	      tr->data[tr->data[t].d0].u0 = t;
	      tr->data[tr->data[t].d0].u1 = tn;
	      tr->data[tr->data[t].d1].u0 = tn;
	      tr->data[tr->data[t].d1].u1 = SIZE_MAX;

	      /* new code to determine the bottom neighbours of the */
	      /* newly partitioned trapezoid */

	      tr->data[t].d1 = SIZE_MAX;

	      tnext = tr->data[t].d0;
	    }
	  else			/* intersecting d1 */
	    {
	      tr->data[tr->data[t].d0].u0 = t;
	      tr->data[tr->data[t].d0].u1 = SIZE_MAX;
	      tr->data[tr->data[t].d1].u0 = t;
	      tr->data[tr->data[t].d1].u1 = tn;

	      /* new code to determine the bottom neighbours of the */
	      /* newly partitioned trapezoid */

	      tr->data[tn].d0 = tr->data[t].d1;
	      tr->data[tn].d1 = SIZE_MAX;

	      tnext = tr->data[t].d1;
	    }

	  t = tnext;
	}

      tr->data[t_sav].rseg = segnum;
      tr->data[tn_sav].lseg = segnum;
    } /* end-while */

  /* Now combine those trapezoids which share common segments. We can */
  /* use the pointers to the parent to connect these together. This */
  /* works only because all these new trapezoids have been formed */
  /* due to splitting by the segment, and hence have only one parent */

  const size_t tfirstl = tfirst;
  const size_t tlastl = tlast;
  merge_trapezoids(segnum, tfirstl, tlastl, S_LEFT, tr, qs);
  merge_trapezoids(segnum, tfirstr, tlastr, S_RIGHT, tr, qs);

  seg[segnum].is_inserted = true;
}

/* Update the roots stored for each of the endpoints of the segment.
 * This is done to speed up the location-query for the endpoint when
 * the segment is inserted into the trapezoidation subsequently
 */
static void
find_new_roots(int segnum, segment_t *seg, traps_t *tr, qnodes_t *qs) {
  segment_t *s = &seg[segnum];

  if (s->is_inserted) return;

  s->root0 = (size_t)locate_endpoint(&s->v0, &s->v1, s->root0, seg, qs);
  s->root0 = tr->data[s->root0].sink;

  s->root1 = (size_t)locate_endpoint(&s->v1, &s->v0, s->root1, seg, qs);
  s->root1 = tr->data[s->root1].sink;
}

/* Get log*n for given n */
static int math_logstar_n(int n)
{
  int i;
  double v;

  for (i = 0, v = (double) n; v >= 1; i++)
      v = log2(v);

  return i - 1;
}

static int math_N(int n, int h)
{
  int i;
  double v;

  for (i = 0, v = (double) n; i < h; i++)
      v = log2(v);

  return (int)ceil(1.0 * n / v);
}

/* Main routine to perform trapezoidation */
traps_t construct_trapezoids(int nseg, segment_t *seg, int *permute) {
    int i;
    int h;
    int segi = 0;

    // We will append later nodes by expanding this on-demand. First node is a
    // sentinel.
    qnodes_t qs = {0};
    qnodes_append(&qs, (qnode_t){0});

    // First trapezoid is reserved as a sentinel. We will append later
    // trapezoids by expanding this on-demand.
    traps_t tr = {.length = 1, .data = gv_calloc(1, sizeof(trap_t))};

  /* Add the first segment and get the query structure and trapezoid */
  /* list initialised */

    const size_t root = init_query_structure(permute[segi++], seg, &tr, &qs);

    for (i = 1; i <= nseg; i++)
	seg[i].root0 = seg[i].root1 = root;

    const int logstar = math_logstar_n(nseg);
    for (h = 1; h <= logstar; h++) {
	for (i = math_N(nseg, h -1) + 1; i <= math_N(nseg, h); i++)
	    add_segment(permute[segi++], seg, &tr, &qs);

      /* Find a new root for each of the segment endpoints */
	for (i = 1; i <= nseg; i++)
	    find_new_roots(i, seg, &tr, &qs);
    }

    for (i = math_N(nseg, logstar) + 1; i <= nseg; i++)
	add_segment(permute[segi++], seg, &tr, &qs);

    qnodes_free(&qs);
    return tr;
}
