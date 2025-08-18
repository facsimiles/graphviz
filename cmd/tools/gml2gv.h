/**
 * @file
 * @brief <a href=https://en.wikipedia.org/wiki/Graph_Modelling_Language>GML</a>-DOT converter
 */

#include <stdio.h>
#include <cgraph/cgraph.h>
#include <util/list.h>

typedef struct {
    unsigned short kind;
    unsigned short sort;
    char* name;
    union {
	char* value;
	void *lp; ///< actually an `attrs_t *`
    }u;
} gmlattr;

typedef LIST(gmlattr *) attrs_t;

typedef struct {
    char* id;
    attrs_t attrlist;  
} gmlnode;

typedef struct {
    char* source;
    char* target;
    attrs_t attrlist;  
} gmledge;

typedef struct gmlgraph {
    struct gmlgraph* parent;
    int directed;
    attrs_t attrlist;  
    LIST(gmlnode *) nodelist;
    LIST(gmledge *) edgelist;
    LIST(struct gmlgraph *) graphlist;
} gmlgraph;

extern int gmllex(void);
extern void gmllexeof(void);
extern void gmlerror(const char *);
extern int gmlerrors(void);
extern void initgmlscan (FILE*);
extern Agraph_t* gml_to_gv (char*, FILE*, int, int*);
