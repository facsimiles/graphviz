/**
 * @file
 * @ingroup cgraph_core
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

%require "3.0"

  /* By default, Bison emits a parser using symbols prefixed with "yy". Graphviz
   * contains multiple Bison-generated parsers, so we alter this prefix to avoid
   * symbol clashes.
   */
%define api.prefix {aag}

  /* Generate a reentrant parser with no global state. */
%define api.pure full
  /* aagparse() gets an argument defined by flex. */
%param { aagscan_t scanner }


%code requires {
#include <agxbuf.h>
#include <cghdr.h>

struct gstack_s;

struct aagextra_s {
	/* Common */
	Agdisc_t *Disc;		/* discipline passed to agread or agconcat */
	void *Ifile;
	/* Parser */
	int SubgraphDepth;
	/* Lexer */
	int line_num; // = 1;
	int html_nest;  /* nesting level for html strings */
	agxbuf InputFile;
	int graphType;
	/* buffer for arbitrary length strings (longer than BUFSIZ) */
	agxbuf Sbuf;
};

}

%code {
	extern void aagerror(aagscan_t, const char*);
	extern int aaglex(AAGSTYPE *, aagscan_t);
}

%{

#include <stdbool.h>
#include <stdio.h>
#include <cghdr.h>
#include <cgraph/agxbuf.h>
#include <stddef.h>
#include <util/alloc.h>
#include <util/streq.h>
#include <util/unreachable.h>

static const char Key[] = "key";

typedef union s {					/* possible items in generic list */
		Agnode_t		*n;
		Agraph_t		*subg;
		Agedge_t		*e;
		Agsym_t			*asym;	/* bound attribute */
		char			*name;	/* unbound attribute */
		struct item_s	*list;	/* list-of-lists (for edgestmt) */
} val_t;

typedef struct item_s {		/* generic list */
	int				tag;	/* T_node, T_subgraph, T_edge, T_attr */
	val_t			u;		/* primary element */
	char			*str;	/* secondary value - port or attr value */
	struct item_s	*next;
} item;

typedef struct list_s {		/* maintain head and tail ptrs for fast append */
	item			*first;
	item			*last;
} list_t;

typedef struct gstack_s {
	Agraph_t *g;
	Agraph_t *subg;
	list_t	nodelist,edgelist,attrlist;
	struct gstack_s *down;
} gstack_t;

/* functions */
static void appendnode(aagscan_t scanner, char *name, char *port, char *sport);
static void attrstmt(aagscan_t scanner, int tkind, char *macroname);
static void startgraph(aagscan_t scanner, char *name, bool directed, bool strict);
static void getedgeitems(aagscan_t scanner);
static void newedge(aagscan_t scanner, Agnode_t *t, char *tport, Agnode_t *h, char *hport, char *key);
static void edgerhs(aagscan_t scanner, Agnode_t *n, char *tport, item *hlist, char *key);
static void appendattr(aagscan_t scanner, char *name, char *value);
static void bindattrs(int kind);
static void applyattrs(void *obj);
static void endgraph(aagscan_t scanner);
static void endnode(aagscan_t scanner);
static void endedge(aagscan_t scanner);
static void freestack(void);
static char* concat(char*, char*);
static char* concatPort(char*, char*);

static void opensubg(aagscan_t scanner, char *name);
static void closesubg(aagscan_t scanner);

/* global */
static Agraph_t *G;				/* top level graph */
static gstack_t *S;

%}

%union	{
			int				i;
			char			*str;
			struct Agnode_s	*n;
}

%token <i> T_graph T_node T_edge T_digraph T_subgraph T_strict T_edgeop
	/* T_list, T_attr are internal tags, not really tokens */
%token T_list T_attr
%token <str> T_atom T_qatom

%type <i>  optstrict graphtype rcompound attrtype
%type <str> optsubghdr optgraphname optmacroname atom qatom


%%

graph		:  hdr body {freestack(); endgraph(scanner);}
			|  error	{if (G) {freestack(); endgraph(scanner); agclose(G); G = Ag_G_global = NULL;}}
			|  /* empty */
			;

body		: '{' optstmtlist '}' ;

hdr			:	optstrict graphtype optgraphname {startgraph(scanner,$3,$2 != 0,$1 != 0);}
			;

optgraphname:	atom {$$=$1;} | /* empty */ {$$=0;} ;

optstrict	:  T_strict  {$$=1;} |  /* empty */ {$$=0;} ;

graphtype	:	T_graph {$$ = 0;} |	T_digraph {$$ = 1;} ;

optstmtlist	:	stmtlist |	/* empty */ ;

stmtlist 	:	stmtlist stmt |	stmt ;

optsemi		: ';' | ;

stmt		:  attrstmt  optsemi
			|  compound	 optsemi
			;

compound 	:	simple rcompound optattr
					{if ($2) endedge(scanner); else endnode(scanner);}
			;

simple		:	nodelist | subgraph ;

rcompound	:	T_edgeop {getedgeitems(scanner);} simple {getedgeitems(scanner);} rcompound {$$ = 1;}
			|	/* empty */ {$$ = 0;}
			;


nodelist	: node | nodelist ',' node ;

node		: atom {appendnode(scanner,$1,NULL,NULL);}
            | atom ':' atom {appendnode(scanner,$1,$3,NULL);}
            | atom ':' atom ':' atom {appendnode(scanner,$1,$3,$5);}
            ;

attrstmt	:  attrtype optmacroname attrlist {attrstmt(scanner,$1,$2);}
			|  graphattrdefs {attrstmt(scanner,T_graph,NULL);}
			;

attrtype :	T_graph {$$ = T_graph;}
			| T_node {$$ = T_node;}
			| T_edge {$$ = T_edge;}
			;

optmacroname : atom '=' {$$ = $1;}
			| /* empty */ {$$ = NULL; }
			;

optattr		:	attrlist |  /* empty */ ;

attrlist	: optattr '[' optattrdefs ']' ;

optattrdefs	: optattrdefs attrdefs 
			| /* empty */ ;

attrdefs	:  attrassignment optseparator
			;

attrassignment	:  atom '=' atom {appendattr(scanner,$1,$3);}
			;

graphattrdefs : attrassignment
			;

subgraph	:  optsubghdr {opensubg(scanner,$1);} body {closesubg(scanner);}
			;

optsubghdr	: T_subgraph atom {$$=$2;}
			| T_subgraph  {$$=NULL;}
			| /* empty */ {$$=NULL;}
			;

optseparator :  ';' | ',' | /*empty*/ ;

atom	:  T_atom {$$ = $1;}
			|  qatom {$$ = $1;}
			;

qatom	:  T_qatom {$$ = $1;}
			|  qatom '+' T_qatom {$$ = concat($1,$3);}
			;
%%

static item *newitem(int tag, void *p0, char *p1)
{
	item	*rv = agalloc(G,sizeof(item));
	rv->tag = tag; rv->u.name = (char*)p0; rv->str = p1;
	return rv;
}

static item *cons_node(Agnode_t *n, char *port)
	{ return newitem(T_node,n,port); }

static item *cons_attr(char *name, char *value)
	{ return newitem(T_atom,name,value); }

static item *cons_list(item *list)
	{ return newitem(T_list,list,NULL); }

static item *cons_subg(Agraph_t *subg)
	{ return newitem(T_subgraph,subg,NULL); }

static gstack_t *push(gstack_t *s, Agraph_t *subg) {
	gstack_t *rv;
	rv = agalloc(G,sizeof(gstack_t));
	rv->down = s;
	rv->g = subg;
	return rv;
}

static gstack_t *pop(gstack_t *s)
{
	gstack_t *rv;
	rv = s->down;
	agfree(G,s);
	return rv;
}

static void delete_items(item *ilist)
{
	item	*p,*pn;

	for (p = ilist; p; p = pn) {
		pn = p->next;
		if (p->tag == T_list) delete_items(p->u.list);
		if (p->tag == T_atom) agstrfree(G,p->str);
		agfree(G,p);
	}
}

static void deletelist(list_t *list)
{
	delete_items(list->first);
	list->first = list->last = NULL;
}

static void listapp(list_t *list, item *v)
{
	if (list->last) list->last->next = v;
	list->last = v;
	if (list->first == NULL) list->first = v;
}


/* attrs */
static void appendattr(aagscan_t scanner, char *name, char *value)
{
	(void)scanner;
	item		*v;

	assert(value != NULL);
	v = cons_attr(name,value);
	listapp(&(S->attrlist),v);
}

static void bindattrs(int kind)
{
	item		*aptr;
	char		*name;

	for (aptr = S->attrlist.first; aptr; aptr = aptr->next) {
		assert(aptr->tag == T_atom);	/* signifies unbound attr */
		name = aptr->u.name;
		if (kind == AGEDGE && streq(name,Key)) continue;
		if ((aptr->u.asym = agattr(S->g,kind,name,NULL)) == NULL)
			aptr->u.asym = agattr(S->g,kind,name,"");
		aptr->tag = T_attr;				/* signifies bound attr */
		agstrfree(G,name);
	}
}

/* attach node/edge specific attributes */
static void applyattrs(void *obj)
{
	item		*aptr;

	for (aptr = S->attrlist.first; aptr; aptr = aptr->next) {
		if (aptr->tag == T_attr) {
			if (aptr->u.asym) {
				agxset(obj,aptr->u.asym,aptr->str);
			}
		}
		else {
			assert(AGTYPE(obj) == AGINEDGE || AGTYPE(obj) == AGOUTEDGE);
			assert(aptr->tag == T_atom);
			assert(streq(aptr->u.name,Key));
		}
	}
}

static void nomacros(void)
{
  agwarningf("attribute macros not implemented");
}

/* attrstmt:
 * First argument is always attrtype, so switch covers all cases.
 * This function is used to handle default attribute value assignment.
 */
static void attrstmt(aagscan_t scanner, int tkind, char *macroname)
{
	(void)scanner;
	item			*aptr;
	int				kind = 0;
	Agsym_t*  sym;

		/* creating a macro def */
	if (macroname) nomacros();
		/* invoking a macro def */
	for (aptr = S->attrlist.first; aptr; aptr = aptr->next)
		if (aptr->str == NULL) nomacros();

	switch(tkind) {
		case T_graph: kind = AGRAPH; break;
		case T_node: kind = AGNODE; break;
		case T_edge: kind = AGEDGE; break;
		default: UNREACHABLE();
	}
	bindattrs(kind);	/* set up defaults for new attributes */
	for (aptr = S->attrlist.first; aptr; aptr = aptr->next) {
		/* If the tag is still T_atom, aptr->u.asym has not been set */
		if (aptr->tag == T_atom) continue;
		if (!(aptr->u.asym->fixed) || (S->g != G))
			sym = agattr(S->g,kind,aptr->u.asym->name,aptr->str);
		else
			sym = aptr->u.asym;
		if (S->g == G)
			sym->print = true;
	}
	deletelist(&(S->attrlist));
}

/* nodes */

static void appendnode(aagscan_t scanner, char *name, char *port, char *sport)
{
	(void)scanner;
	item		*elt;

	if (sport) {
		port = concatPort (port, sport);
	}
	elt = cons_node(agnode(S->g, name, 1), port);
	listapp(&(S->nodelist),elt);
	agstrfree(G,name);
}

/* apply current optional attrs to nodelist and clean up lists */
/* what's bad is that this could also be endsubg.  also, you can't
clean up S->subg in closesubg() because S->subg might be needed
to construct edges.  these are the sort of notes you write to yourself
in the future. */
static void endnode(aagscan_t scanner)
{
	(void)scanner;
	item	*ptr;

	bindattrs(AGNODE);
	for (ptr = S->nodelist.first; ptr; ptr = ptr->next)
		applyattrs(ptr->u.n);
	deletelist(&(S->nodelist));
	deletelist(&(S->attrlist));
	deletelist(&(S->edgelist));
	S->subg = 0;  /* notice a pattern here? :-( */
}

/* edges - store up node/subg lists until optional edge key can be seen */

static void getedgeitems(aagscan_t scanner)
{
	(void)scanner;
	item	*v = 0;

	if (S->nodelist.first) {
		v = cons_list(S->nodelist.first);
		S->nodelist.first = S->nodelist.last = NULL;
	}
	else {if (S->subg) v = cons_subg(S->subg); S->subg = 0;}
	/* else nil append */
	if (v) listapp(&(S->edgelist),v);
}

static void endedge(aagscan_t scanner)
{
	char			*key;
	item			*aptr,*tptr,*p;

	Agnode_t		*t;
	Agraph_t		*subg;

	bindattrs(AGEDGE);

	/* look for "key" pseudo-attribute */
	key = NULL;
	for (aptr = S->attrlist.first; aptr; aptr = aptr->next) {
		if ((aptr->tag == T_atom) && streq(aptr->u.name,Key))
			key = aptr->str;
	}

	/* can make edges with node lists or subgraphs */
	for (p = S->edgelist.first; p->next; p = p->next) {
		if (p->tag == T_subgraph) {
			subg = p->u.subg;
			for (t = agfstnode(subg); t; t = agnxtnode(subg,t))
				edgerhs(scanner,agsubnode(S->g, t, 0), NULL, p->next, key);
		}
		else {
			for (tptr = p->u.list; tptr; tptr = tptr->next)
				edgerhs(scanner,tptr->u.n,tptr->str,p->next,key);
		}
	}
	deletelist(&(S->nodelist));
	deletelist(&(S->edgelist));
	deletelist(&(S->attrlist));
	S->subg = 0;
}

/* concat:
 */
static char*
concat (char* s1, char* s2)
{
  char*  s;
  char   buf[BUFSIZ];
  char*  sym;
  size_t len = strlen(s1) + strlen(s2) + 1;

  if (len <= BUFSIZ) sym = buf;
  else sym = gv_alloc(len);
  strcpy(sym,s1);
  strcat(sym,s2);
  s = agstrdup (G,sym);
  agstrfree (G,s1);
  agstrfree (G,s2);
  if (sym != buf) free (sym);
  return s;
}

static char*
concatPort (char* s1, char* s2)
{
  agxbuf buf = {0};

  agxbprint(&buf, "%s:%s", s1, s2);
  char *s = agstrdup(G, agxbuse(&buf));
  agstrfree (G,s1);
  agstrfree (G,s2);
  agxbfree(&buf);
  return s;
}


static void edgerhs(aagscan_t scanner, Agnode_t *tail, char *tport, item *hlist, char *key)
{
	Agnode_t		*head;
	Agraph_t		*subg;
	item			*hptr;

	if (hlist->tag == T_subgraph) {
		subg = hlist->u.subg;
		for (head = agfstnode(subg); head; head = agnxtnode(subg,head))
			newedge(scanner, tail, tport, agsubnode(S->g, head, 0), NULL, key);
	}
	else {
		for (hptr = hlist->u.list; hptr; hptr = hptr->next)
			newedge(scanner, tail, tport, agsubnode(S->g, hptr->u.n, 0), hptr->str, key);
	}
}

static void mkport(aagscan_t scanner, Agedge_t *e, char *name, char *val)
{
	(void)scanner;
	Agsym_t *attr;
	if (val) {
		if ((attr = agattr(S->g,AGEDGE,name,NULL)) == NULL)
			attr = agattr(S->g,AGEDGE,name,"");
		agxset(e,attr,val);
	}
}

static void newedge(aagscan_t scanner, Agnode_t *t, char *tport, Agnode_t *h, char *hport, char *key)
{
	Agedge_t 	*e;

	e = agedge(S->g, t, h, key, 1);
	if (e) {		/* can fail if graph is strict and t==h */
		char    *tp = tport;
		char    *hp = hport;
		if ((agtail(e) != aghead(e)) && (aghead(e) == t)) {
			/* could happen with an undirected edge */
			char    *temp;
			temp = tp; tp = hp; hp = temp;
		}
		mkport(scanner, e,TAILPORT_ID,tp);
		mkport(scanner, e,HEADPORT_ID,hp);
		applyattrs(e);
	}
}

/* graphs and subgraphs */


static void startgraph(aagscan_t scanner, char *name, bool directed, bool strict)
{
	aagextra_t *ctx = aagget_extra(scanner);
	if (G == NULL) {
		ctx->SubgraphDepth = 0;
		Agdesc_t req = {.directed = directed, .strict = strict, .maingraph = true};
		Ag_G_global = G = agopen(name,req,ctx->Disc);
	}
	else {
		Ag_G_global = G;
	}
	S = push(S,G);
	agstrfree(NULL,name);
}

static void endgraph(aagscan_t scanner)
{
	aglexeof(scanner);
	aginternalmapclearlocalnames(G);
}

static void opensubg(aagscan_t scanner, char *name)
{
  aagextra_t *ctx = aagget_extra(scanner);

  if (++ctx->SubgraphDepth >= YYMAXDEPTH/2) {
    agerrorf("subgraphs nested more than %d deep", YYMAXDEPTH);
  }
	S = push(S, agsubg(S->g, name, 1));
	agstrfree(G,name);
}

static void closesubg(aagscan_t scanner)
{
	aagextra_t *ctx = aagget_extra(scanner);
	Agraph_t *subg = S->g;
	--ctx->SubgraphDepth;
	S = pop(S);
	S->subg = subg;
	assert(subg);
}

static void freestack(void)
{
	while (S) {
		deletelist(&(S->nodelist));
		deletelist(&(S->attrlist));
		deletelist(&(S->edgelist));
		S = pop(S);
	}
}
static const char *InputFile;

  /* (Re)set file:
   */
void agsetfile(const char* f) { InputFile = f; }

Agraph_t *agconcat(Agraph_t *g, void *chan, Agdisc_t *disc)
{
	aagscan_t scanner = NULL;
	aagextra_t extra = {
		.Disc = disc ? disc : &AgDefaultDisc,
		.Ifile = chan,
		.line_num = 1,
	};
	if (InputFile)
		agxbput(&extra.InputFile, InputFile);
	if (aaglex_init_extra(&extra, &scanner)) {
		return NULL;
	}
	aagset_in(chan, scanner);
	G = g;
	Ag_G_global = NULL;
	aagparse(scanner);
	if (Ag_G_global == NULL) aglexbad(scanner);
	aaglex_destroy(scanner);
	agxbfree(&extra.InputFile);
	agxbfree(&extra.Sbuf);
	return Ag_G_global;
}

Agraph_t *agread(void *fp, Agdisc_t *disc) {return agconcat(NULL,fp,disc); }

