/*************************************************************************
 * Copyright (c) 2011 AT&T Intellectual Property 
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * https://www.eclipse.org/legal/epl-v10.html
 *
 * Contributors: Details at https://graphviz.org
 *************************************************************************/

%require "3.0"

  /* By default, Bison emits a parser using symbols prefixed with "yy". Graphviz
   * contains multiple Bison-generated parsers, so we alter this prefix to avoid
   * symbol clashes.
   */
%define api.prefix {gml}

%{
#include <stdlib.h>
#include <string.h>
#include <arith.h>
#include <gml2gv.h>
#include <assert.h>
#include <util/agxbuf.h>
#include <util/alloc.h>
#include <util/exit.h>
#include <util/list.h>

static gmlgraph* G;
static gmlnode* N;
static gmledge* E;

static attrs_t *L;

static void free_attrs(attrs_t *a) {
  LIST_FREE(a);
  free(a);
}

static LIST(attrs_t *) liststk = {.dtor = free_attrs};

static char *sortToStr(unsigned short sort);

static void free_node(gmlnode *p) {
    if (!p) return;
    LIST_FREE(&p->attrlist);
    free(p->id);
    free (p);
}

static void free_edge(gmledge *p) {
    if (!p) return;
    LIST_FREE(&p->attrlist);
    free(p->target);
    free(p->source);
    free (p);
}

static void free_graph(gmlgraph *p) {
    if (!p) return;
    LIST_FREE(&p->nodelist);
    LIST_FREE(&p->edgelist);
    LIST_FREE(&p->attrlist);
    LIST_FREE(&p->graphlist);
    free (p);
}

static void
cleanup (void)
{
    LIST_CLEAR(&liststk);
    if (L) {
	free_attrs(L);
	L = NULL;
    }
    if (N) {
	free_node(N);
	N = NULL;
    }
    if (E) {
	free_edge(E);
	E = NULL;
    }
    if (G) {
	free_graph(G);
	G = NULL;
    }
}

static void free_attr(gmlattr *p);

static void
pushAlist (void)
{
    attrs_t *const lp = gv_alloc(sizeof(attrs_t));
    lp->dtor = free_attr;

    if (L) {
	LIST_PUSH_BACK(&liststk, L);
    }
    L = lp;
}

static attrs_t *popAlist(void) {
    attrs_t *lp = L;

    if (!LIST_IS_EMPTY(&liststk))
	L = LIST_POP_BACK(&liststk);
    else
	L = NULL;

    return lp;
}

static void
popG (void)
{
    G = G->parent;
}

static void
pushG (void)
{
    gmlgraph* g = gv_alloc(sizeof(gmlgraph));

    g->attrlist.dtor = free_attr;
    g->nodelist.dtor = free_node;
    g->edgelist.dtor = free_edge;
    g->graphlist.dtor = free_graph;
    g->parent = G;
    g->directed = -1;

    if (G)
	LIST_APPEND(&G->graphlist, g);

    G = g;
}

static gmlnode*
mkNode (void)
{
    gmlnode *const n = gv_alloc(sizeof(gmlnode));
    n->attrlist.dtor = free_attr;
    return n;
}

static gmledge*
mkEdge (void)
{
    gmledge* ep = gv_alloc(sizeof(gmledge));
    ep->attrlist.dtor = free_attr;
    return ep;
}

static gmlattr *mkAttr(char* name, unsigned short sort, unsigned short kind,
                       char* str,  attrs_t* list) {
    gmlattr* gp = gv_alloc(sizeof(gmlattr));

    assert (name || sort);
    if (!name)
	name = gv_strdup (sortToStr (sort));
    gp->sort = sort;
    gp->kind = kind;
    gp->name = name;
    if (str)
	gp->u.value = str;
    else {
	if (list != NULL && LIST_IS_EMPTY(list)) {
	    free_attrs(list);
	    list = 0;
	}
	gp->u.lp = list;
    }
    return gp;
}

static int
setDir (char* d)
{
    gmlgraph* g;
    int dir = atoi (d);

    free (d);
    if (dir < 0) dir = -1;
    else if (dir > 0) dir = 1;
    else dir = 0;
    G->directed = dir;

    if (dir >= 0) {
	for (g = G->parent; g; g = g->parent) {
	    if (g->directed < 0)
		g->directed = dir;
	    else if (g->directed != dir)
		return 1;
        }
    }

    return 0;
}

%}
%union  {
    int i;
    char *str;
    gmlnode* np;
    gmledge* ep;
    gmlattr* ap;
    attrs_t *list;
}

%token GRAPH NODE EDGE DIRECTED SOURCE TARGET
%token XVAL YVAL WVAL HVAL LABEL GRAPHICS LABELGRAPHICS TYPE FILL
%token OUTLINE OUTLINESTYLE OUTLINEWIDTH WIDTH
%token STYLE LINE POINT
%token TEXT FONTSIZE FONTNAME COLOR
%token <str> INTEGER REAL STRING ID NAME
%token <list> GML_LIST

%type <np> node
%type <ep> edge
%type <list> attrlist
%type <ap> alistitem

%%
graph : optalist hdr body  {gmllexeof(); if (G->parent) popG(); }
      | error { cleanup(); YYABORT; }
      | 
      ;

hdr   : GRAPH { pushG(); }
      ;

body  : '[' optglist ']'
      ;

optglist : glist
         |  /* empty */ 
         ;

glist  : glist glistitem
       | glistitem
       ;

glistitem : node { LIST_APPEND(&G->nodelist, $1); }
          | edge { LIST_APPEND(&G->edgelist, $1); }
          | hdr body 
          | DIRECTED INTEGER { 
		if (setDir($2)) { 
		    yyerror("mixed directed and undirected graphs"); 
		    cleanup ();
		    YYABORT;
		}
	  }
	  | ID INTEGER { LIST_APPEND(&G->attrlist, mkAttr(gv_strdup("id"), 0, INTEGER, $2, 0)); }
          | alistitem { LIST_APPEND(&G->attrlist, $1); }
          ;

node :  NODE { N = mkNode(); } '[' nlist ']' { $$ = N; N = NULL; }
     ;

nlist : nlist nlistitem
      | nlistitem
      ;

nlistitem : ID INTEGER { N->id = $2; }
          | alistitem { LIST_APPEND(&N->attrlist, $1); }
          ;

edge :  EDGE { E = mkEdge(); } '[' elist ']' { $$ = E; E = NULL; }
     ;

elist : elist elistitem
      | elistitem
      ;

elistitem : SOURCE INTEGER { E->source = $2; }
          | TARGET INTEGER { E->target = $2; }
	  | ID INTEGER { LIST_APPEND(&E->attrlist, mkAttr(gv_strdup("id"), 0, INTEGER, $2, 0)); }
          | alistitem { LIST_APPEND(&E->attrlist, $1); }
          ;

attrlist  : '[' {pushAlist(); } optalist ']' { $$ = popAlist(); }
          ;

optalist  : alist
          | /* empty */ 
          ;

alist  : alist alistitem  { LIST_APPEND(L, $2); }
       | alistitem  { LIST_APPEND(L, $1); }
       ;

alistitem : NAME INTEGER { $$ = mkAttr ($1, 0, INTEGER, $2, 0); }
          | NAME REAL    { $$ = mkAttr ($1, 0, REAL, $2, 0); }
          | NAME STRING  { $$ = mkAttr ($1, 0, STRING, $2, 0); }
          | NAME attrlist  { $$ = mkAttr ($1, 0, GML_LIST, 0, $2); }
          | XVAL REAL  { $$ = mkAttr (0, XVAL, REAL, $2, 0); }
          | XVAL INTEGER  { $$ = mkAttr (0, XVAL, REAL, $2, 0); }
          | YVAL REAL  { $$ = mkAttr (0, YVAL, REAL, $2, 0); }
          | WVAL REAL  { $$ = mkAttr (0, WVAL, REAL, $2, 0); }
          | HVAL REAL  { $$ = mkAttr (0, HVAL, REAL, $2, 0); }
          | LABEL STRING  { $$ = mkAttr (0, LABEL, STRING, $2, 0); }
          | GRAPHICS attrlist  { $$ = mkAttr (0, GRAPHICS, GML_LIST, 0, $2); }
          | LABELGRAPHICS attrlist  { $$ = mkAttr (0, LABELGRAPHICS, GML_LIST, 0, $2); }
          | TYPE STRING  { $$ = mkAttr (0, TYPE, STRING, $2, 0); }
          | FILL STRING  { $$ = mkAttr (0, FILL, STRING, $2, 0); }
          | OUTLINE STRING  { $$ = mkAttr (0, OUTLINE, STRING, $2, 0); }
          | OUTLINESTYLE STRING  { $$ = mkAttr (0, OUTLINESTYLE, STRING, $2, 0); }
          | OUTLINEWIDTH INTEGER  { $$ = mkAttr (0, OUTLINEWIDTH, INTEGER, $2, 0); }
          | WIDTH REAL  { $$ = mkAttr (0, WIDTH, REAL, $2, 0); }
          | WIDTH INTEGER  { $$ = mkAttr (0, WIDTH, INTEGER, $2, 0); }
          | STYLE STRING  { $$ = mkAttr (0, STYLE, STRING, $2, 0); }
          | STYLE attrlist  { $$ = mkAttr (0, STYLE, GML_LIST, 0, $2); }
          | LINE attrlist  { $$ = mkAttr (0, LINE, GML_LIST, 0, $2); }
          | POINT attrlist  { $$ = mkAttr (0, POINT, GML_LIST, 0, $2); }
          | TEXT STRING  { $$ = mkAttr (0, TEXT, STRING, $2, 0); }
          | FONTNAME STRING  { $$ = mkAttr (0, FONTNAME, STRING, $2, 0); }
          | FONTSIZE INTEGER  { $$ = mkAttr (0, FONTNAME, INTEGER, $2, 0); }
          | COLOR STRING  { $$ = mkAttr (0, COLOR, STRING, $2, 0); }
          ;

%%

static void free_attr(gmlattr *p) {
    if (!p) return;
    if (p->kind == GML_LIST && p->u.lp)
	free_attrs(p->u.lp);
    else
	free (p->u.value);
    free (p->name);
    free (p);
}

static void deparseList(attrs_t *alist, agxbuf *xb);

static void
deparseAttr (gmlattr* ap, agxbuf* xb)
{
    if (ap->kind == GML_LIST) {
	agxbprint (xb, "%s ", ap->name);
	deparseList (ap->u.lp, xb);
    }
    else if (ap->kind == STRING) {
	agxbprint (xb, "%s \"%s\"", ap->name, ap->u.value);
    }
    else {
	agxbprint (xb, "%s %s", ap->name, ap->u.value);
    }
}

static void deparseList(attrs_t *alist, agxbuf *xb) {
    agxbput (xb, "[ "); 
    for (size_t i = 0; alist != NULL && i < LIST_SIZE(alist); ++i) {
	gmlattr *const ap = LIST_GET(alist, i);
	deparseAttr (ap, xb);
	agxbputc (xb, ' ');
    }
    agxbput (xb, "]"); 
  
}

static void
unknown (Agobj_t* obj, gmlattr* ap, agxbuf* xb)
{
    char* str;

    if (ap->kind == GML_LIST) {
	deparseList (ap->u.lp, xb);
	str = agxbuse (xb);
    }
    else
	str = ap->u.value;

    agsafeset (obj, ap->name, str, "");
}

static void addNodeLabelGraphics(Agnode_t *np, attrs_t *alist, agxbuf *unk) {
    int cnt = 0;

    if (!alist)
	return;

    for (size_t i = 0; i < LIST_SIZE(alist); ++i) {
	gmlattr *const ap = LIST_GET(alist, i);
	if (ap->sort == TEXT) {
	    agsafeset (np, "label", ap->u.value, "");
	}
	else if (ap->sort == COLOR) {
	    agsafeset (np, "fontcolor", ap->u.value, "");
	}
	else if (ap->sort == FONTSIZE) {
	    agsafeset (np, "fontsize", ap->u.value, "");
	}
	else if (ap->sort == FONTNAME) {
	    agsafeset (np, "fontname", ap->u.value, "");
	}
	else {
	    if (cnt)
		agxbputc (unk, ' '); 
	    else {
		agxbput (unk, "[ "); 
	    }
	    deparseAttr (ap, unk);
	    cnt++;
	}
    }

    if (cnt) {
	agxbput (unk, " ]"); 
	agsafeset (np, "LabelGraphics", agxbuse (unk), "");
    }
    else
	agxbclear (unk); 
}

static void addEdgeLabelGraphics(Agedge_t *ep, attrs_t *alist, agxbuf *xb,
                                 agxbuf *unk) {
    char* x = "0";
    char* y = "0";
    int cnt = 0;

    if (!alist)
	return;

    for (size_t i = 0; i < LIST_SIZE(alist); ++i) {
	gmlattr *const ap = LIST_GET(alist, i);
	if (ap->sort == TEXT) {
	    agsafeset (ep, "label", ap->u.value, "");
	}
	else if (ap->sort == COLOR) {
	    agsafeset (ep, "fontcolor", ap->u.value, "");
	}
	else if (ap->sort == FONTSIZE) {
	    agsafeset (ep, "fontsize", ap->u.value, "");
	}
	else if (ap->sort == FONTNAME) {
	    agsafeset (ep, "fontname", ap->u.value, "");
	}
	else if (ap->sort == XVAL) {
	    x = ap->u.value;
	}
	else if (ap->sort == YVAL) {
	    y = ap->u.value;
	}
	else {
	    if (cnt)
		agxbputc (unk, ' '); 
	    else {
		agxbput (unk, "[ "); 
	    }
	    deparseAttr (ap, unk);
	    cnt++;
	}
    }

    agxbprint (xb, "%s,%s", x, y);
    agsafeset (ep, "lp", agxbuse (xb), "");

    if (cnt) {
	agxbput (unk, " ]"); 
	agsafeset (ep, "LabelGraphics", agxbuse (unk), "");
    }
    else
	agxbclear (unk); 
}

static void addNodeGraphics(Agnode_t *np, attrs_t *alist, agxbuf *xb,
                            agxbuf *unk) {
    char* x = "0";
    char* y = "0";
    char buf[BUFSIZ];
    double d;
    int cnt = 0;

    for (size_t i = 0; alist != NULL && i < LIST_SIZE(alist); ++i) {
	gmlattr *const ap = LIST_GET(alist, i);
	if (ap->sort == XVAL) {
	    x = ap->u.value;
	}
	else if (ap->sort == YVAL) {
	    y = ap->u.value;
	}
	else if (ap->sort == WVAL) {
	    d = atof (ap->u.value);
	    snprintf(buf, sizeof(buf), "%.04f", d/72.0);
	    agsafeset (np, "width", buf, "");
	}
	else if (ap->sort == HVAL) {
	    d = atof (ap->u.value);
	    snprintf(buf, sizeof(buf), "%.04f", d/72.0);
	    agsafeset (np, "height", buf, "");
	}
	else if (ap->sort == TYPE) {
	    agsafeset (np, "shape", ap->u.value, "");
	}
	else if (ap->sort == FILL) {
	    agsafeset (np, "color", ap->u.value, "");
	}
	else if (ap->sort == OUTLINE) {
	    agsafeset (np, "pencolor", ap->u.value, "");
	}
	else if (ap->sort == WIDTH || ap->sort == OUTLINEWIDTH) {
	    agsafeset (np, "penwidth", ap->u.value, "");
	}
	else if (ap->sort == STYLE || ap->sort == OUTLINESTYLE) {
	    agsafeset (np, "style", ap->u.value, "");
	}
	else {
	    if (cnt)
		agxbputc (unk, ' '); 
	    else {
		agxbput (unk, "[ "); 
	    }
	    deparseAttr (ap, unk);
	    cnt++;
	}
    }

    agxbprint (xb, "%s,%s", x, y);
    agsafeset (np, "pos", agxbuse (xb), "");

    if (cnt) {
	agxbput (unk, " ]"); 
	agsafeset (np, "graphics", agxbuse (unk), "");
    }
    else
	agxbclear (unk); 
}

static void addEdgePoint(Agedge_t *ep, attrs_t *alist, agxbuf *xb) {
    char* x = "0";
    char* y = "0";

    for (size_t i = 0; alist != NULL && i < LIST_SIZE(alist); ++i) {
        gmlattr *const ap = LIST_GET(alist, i);
        if (ap->sort == XVAL) {
	    x = ap->u.value;
	}
	else if (ap->sort == YVAL) {
	    y = ap->u.value;
	}
	else {
	    fprintf (stderr, "non-X/Y field in point attribute");
	    unknown ((Agobj_t*)ep, ap, xb);
	}
    }

    if (agxblen(xb)) agxbputc (xb, ' ');
    agxbprint (xb, "%s,%s", x, y);
}

static void addEdgePos(Agedge_t *ep, attrs_t *alist, agxbuf *xb) {
    if (!alist) return;
    for (size_t i = 0; i < LIST_SIZE(alist); ++i) {
	gmlattr *const ap = LIST_GET(alist, i);
	if (ap->sort == POINT) {
	    addEdgePoint (ep, ap->u.lp, xb);
	}
	else {
	    fprintf (stderr, "non-point field in line attribute");
	    unknown(&ep->base, ap, xb);
	}
    }
    agsafeset (ep, "pos", agxbuse (xb), "");
}

static void addEdgeGraphics(Agedge_t *ep, attrs_t *alist, agxbuf *xb,
                            agxbuf *unk) {
    int cnt = 0;

    for (size_t i = 0; alist != NULL && i < LIST_SIZE(alist); ++i) {
	gmlattr *const ap = LIST_GET(alist, i);
	if (ap->sort == WIDTH) {
	    agsafeset (ep, "penwidth", ap->u.value, "");
	}
	else if (ap->sort == STYLE) {
	    agsafeset (ep, "style", ap->u.value, "");
	}
	else if (ap->sort == FILL) {
	    agsafeset (ep, "color", ap->u.value, "");
	}
	else if (ap->sort == LINE) {
	    addEdgePos (ep, ap->u.lp, xb);
	}
	else {
	    if (cnt)
		agxbputc (unk, ' '); 
	    else {
		agxbput (unk, "[ "); 
	    }
	    deparseAttr (ap, unk);
	    cnt++;
	}
    }

    if (cnt) {
	agxbput (unk, " ]"); 
	agsafeset (ep, "graphics", agxbuse (unk), "");
    }
    else
	agxbclear(unk);
}

static void addAttrs(Agobj_t *obj, attrs_t *alist, agxbuf *xb, agxbuf *unk) {
    for (size_t i = 0; i < LIST_SIZE(alist); ++i) {
	gmlattr *const ap = LIST_GET(alist, i);
	if (ap->sort == GRAPHICS) {
	    if (AGTYPE(obj) == AGNODE)
		addNodeGraphics ((Agnode_t*)obj, ap->u.lp, xb, unk);
	    else if (AGTYPE(obj) == AGEDGE)
		addEdgeGraphics ((Agedge_t*)obj, ap->u.lp, xb, unk);
	    else
		unknown (obj, ap, xb);
	}
	else if (ap->sort == LABELGRAPHICS) {
	    if (AGTYPE(obj) == AGNODE)
		addNodeLabelGraphics ((Agnode_t*)obj, ap->u.lp, unk);
	    else if (AGTYPE(obj) == AGEDGE)
		addEdgeLabelGraphics ((Agedge_t*)obj, ap->u.lp, xb, unk);
	    else
		unknown (obj, ap, xb);
	}
	else
	    unknown (obj, ap, xb);
    }
}

static Agraph_t *mkGraph(gmlgraph *graph, Agraph_t *parent, char *name,
                         agxbuf *xb, agxbuf *unk) {
    Agraph_t* g;
    Agnode_t* n;
    Agnode_t* h;
    Agedge_t* e;

    if (parent) {
	g = agsubg (parent, NULL, 1);
    }
    else if (graph->directed >= 1)
	g = agopen (name, Agdirected, 0);
    else
	g = agopen (name, Agundirected, 0);

    if (!parent && L) {
	addAttrs(&g->base, L, xb, unk);
    } 
    for (size_t i = 0; i < LIST_SIZE(&graph->nodelist); ++i) {
	gmlnode *const np = LIST_GET(&graph->nodelist, i);
	if (!np->id) {
	   fprintf (stderr, "node without an id attribute"); 
	   graphviz_exit (1);
        }
	n = agnode (g, np->id, 1);
	addAttrs(&n->base, &np->attrlist, xb, unk);
    }

    for (size_t i = 0; i < LIST_SIZE(&graph->edgelist); ++i) {
	gmledge *ep = LIST_GET(&graph->edgelist, i);
	if (!ep->source) {
	   fprintf (stderr, "edge without an source attribute"); 
	   graphviz_exit (1);
        }
	if (!ep->target) {
	   fprintf (stderr, "node without an target attribute"); 
	   graphviz_exit (1);
        }
	n = agnode (g, ep->source, 1);
	h = agnode (g, ep->target, 1);
	e = agedge (g, n, h, NULL, 1);
	addAttrs(&e->base, &ep->attrlist, xb, unk);
    }
    for (size_t i = 0; i < LIST_SIZE(&graph->graphlist); ++i) {
	gmlgraph *const gp = LIST_GET(&graph->graphlist, i);
	mkGraph (gp, g, NULL, xb, unk);
    }

    addAttrs(&g->base, &graph->attrlist, xb, unk);

    return g;
}

Agraph_t*
gml_to_gv (char* name, FILE* fp, int cnt, int* errors)
{
    Agraph_t* g;
    int error;

    if (cnt == 0)
	initgmlscan(fp);
    else
	initgmlscan(0);
		
    L = NULL;
    pushAlist ();
    gmlparse ();

    error = gmlerrors();
    *errors |= error;
    if (!G || error)
	g = NULL;
    else {
	agxbuf xb = {0};
	agxbuf unk = {0};
	g = mkGraph (G, NULL, name, &xb, &unk);
	agxbfree (&xb);
	agxbfree(&unk);
    }

    cleanup ();

    return g;
}

static char *sortToStr(unsigned short sort) {
    char* s;

    switch (sort) {
    case GRAPH : 
	s = "graph"; break;
    case NODE : 
	s = "node"; break;
    case EDGE : 
	s = "edge"; break;
    case DIRECTED : 
	s = "directed"; break;
    case ID : 
	s = "id"; break;
    case SOURCE : 
	s = "source"; break;
    case TARGET : 
	s = "target"; break;
    case XVAL : 
	s = "xval"; break;
    case YVAL : 
	s = "yval"; break;
    case WVAL : 
	s = "wval"; break;
    case HVAL : 
	s = "hval"; break;
    case LABEL : 
	s = "label"; break;
    case GRAPHICS : 
	s = "graphics"; break;
    case LABELGRAPHICS : 
	s = "labelGraphics"; break;
    case TYPE : 
	s = "type"; break;
    case FILL : 
	s = "fill"; break;
    case OUTLINE : 
	s = "outline"; break;
    case OUTLINESTYLE : 
	s = "outlineStyle"; break;
    case OUTLINEWIDTH : 
	s = "outlineWidth"; break;
    case WIDTH : 
	s = "width"; break;
    case STYLE : 
	s = "style"; break;
    case LINE : 
	s = "line"; break;
    case POINT : 
	s = "point"; break;
    case TEXT : 
	s = "text"; break;
    case FONTSIZE : 
	s = "fontSize"; break;
    case FONTNAME : 
	s = "fontName"; break;
    case COLOR : 
	s = "color"; break;
    case INTEGER : 
	s = "integer"; break;
    case REAL : 
	s = "real"; break;
    case STRING : 
	s = "string"; break;
    case NAME : 
	s = "name"; break;
    case GML_LIST : 
	s = "list"; break;
    case '[' : 
	s = "["; break;
    case ']' : 
	s = "]"; break;
    default : 
	s = NULL;break;
    }

    return s;
}
