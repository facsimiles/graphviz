/*************************************************************************
 * Copyright (c) 2011 AT&T Intellectual Property 
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * https://www.eclipse.org/legal/epl-v10.html
 *
 * Contributors: Details at https://graphviz.org
 *************************************************************************/
#include "smyrna_utils.h"
#include <assert.h>
#include <common/types.h>
#include <common/utils.h>
#include <limits.h>
#include <stddef.h>
#include <stdio.h>

int l_int(void *obj, Agsym_t * attr, int def)
{
    return late_int(obj, attr, def, INT_MIN);
}

float l_float(void *obj, Agsym_t * attr, float def)
{
    char *p;
    if (!attr || !obj)
	return def;
    p = agxget(obj, attr);
    if (!p || p[0] == '\0')
	return def;
    return atof(p);
}
int getAttrBool(Agraph_t* g,void* obj,char* attr_name,int def)
{
    Agsym_t *const attr = agattr_text(g, AGTYPE(obj), attr_name, 0);
    return late_bool(obj, attr,def);
}
int getAttrInt(Agraph_t* g,void* obj,char* attr_name,int def)
{
    Agsym_t *const attr = agattr_text(g, AGTYPE(obj), attr_name, 0);
    return l_int(obj,attr,def);
}
float getAttrFloat(Agraph_t* g,void* obj,char* attr_name,float def)
{
    Agsym_t *const attr = agattr_text(g, AGTYPE(obj), attr_name, 0);
    return l_float(obj,attr,def);
}
char* getAttrStr(Agraph_t* g,void* obj,char* attr_name,char* def)
{
    Agsym_t *const attr = agattr_text(g, AGTYPE(obj), attr_name, 0);
    return late_string(obj, attr,def);
}

glCompPoint getPointFromStr(const char *str) {
    glCompPoint p = {0};
    (void)sscanf(str, "%f,%f,%f", &p.x, &p.y, &p.z);
    return p;
}

int point_in_polygon(glCompPoly_t *selPoly, glCompPoint p) {
    const size_t npol = glCompPoly_size(selPoly);
    assert(npol > 0);

    int c = 0;
      for (size_t i = 0, j = npol - 1; i < npol; j = i++) {
        const glCompPoint pt_i = glCompPoly_get(selPoly, i);
        const glCompPoint pt_j = glCompPoly_get(selPoly, j);
        if (((pt_i.y <= p.y && p.y < pt_j.y) ||
             (pt_j.y <= p.y && p.y < pt_i.y)) &&
            p.x < (pt_j.x - pt_i.x) * (p.y - pt_i.y) / (pt_j.y - pt_i.y) + pt_i.x)
          c = !c;
      }
      return c;
    }
