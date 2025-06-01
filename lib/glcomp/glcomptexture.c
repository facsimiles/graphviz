/*************************************************************************
 * Copyright (c) 2011 AT&T Intellectual Property 
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * https://www.eclipse.org/legal/epl-v10.html
 *
 * Contributors: Details at https://graphviz.org
 *************************************************************************/

#include <assert.h>
#include <glcomp/glcomptexture.h>
#include <glcomp/glpangofont.h>
#include <stddef.h>
#include <stdbool.h>
#include <util/alloc.h>
#include <util/streq.h>

static glCompTex *glCompSetAddNewTexture(glCompSet *s, int width, int height,
                                         const unsigned char *data, bool is2D) {
    int offset, ind;
    unsigned char *tarData;

    if (!data)
	return NULL;

    glCompTex *t = gv_alloc(sizeof(glCompTex));
    if (!is2D) {		/*use opengl texture */
	glEnable(GL_TEXTURE_2D);
	glShadeModel(GL_FLAT);
	glEnable(GL_DEPTH_TEST);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glGenTextures(1, &t->id);	//get next id
	if (glGetError() != GL_NO_ERROR) {		/*for some opengl based error , texture couldnt be created */
	    /* drain the OpenGL error queue */
	    while (glGetError() != GL_NO_ERROR);
	    free(t);
	    return NULL;
	} else {
	    glBindTexture(GL_TEXTURE_2D, t->id);
	    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,
			    GL_NEAREST);
	    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
			    GL_NEAREST);
	    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0,
			 GL_RGBA, GL_UNSIGNED_BYTE, data);
	    glDisable(GL_TEXTURE_2D);
	}
    }
    if (is2D) {
	assert(width >= 0);
	assert(height >= 0);
	t->data = gv_calloc(4 * (unsigned)width * (unsigned)height,
	                    sizeof(unsigned char));
	offset = 4;		//RGBA  mod,TO DO implement other modes 
	/*data upside down because of pango gl coord system */
	for (ind = 0; ind < height; ind++) {
	    const unsigned char *srcData = data + (height - 1 - ind) * offset * width;
	    tarData = t->data + ind * offset * width;
	    memcpy(tarData, srcData, 4 * (unsigned)width);
	}
    }

    t->userCount = 1;
    t->width = width;
    t->height = height;
    if(s)
    {
	s->textures =
	gv_recalloc(s->textures, s->textureCount, s->textureCount + 1, sizeof(glCompTex *));
	s->textureCount++;
	s->textures[s->textureCount - 1] = t;
    }
    return t;


}

glCompTex *glCompSetAddNewTexImage(glCompSet *s, int width, int height,
                                   const unsigned char *data, bool is2D) {

    glCompTex *t;
    if (!data)
	return NULL;
    t = glCompSetAddNewTexture(s, width, height, data, is2D);
    if (!t)
	return NULL;
    t->type = glTexImage;
    return t;

}

glCompTex *glCompSetAddNewTexLabel(glCompSet *s, char *def, int fs,
                                   const char *text, bool is2D) {
    int width, height;
    glCompTex *t;
    cairo_surface_t *surface = NULL;
    unsigned char *data = NULL;
    if (!def)
	return NULL;
    /*first check if the same label with same font def created before
       if it was , return its id
     */
    for (size_t ind = 0; ind < s->textureCount; ind++) {
	if (s->textures[ind]->type == glTexLabel) {
	    if (streq(def, s->textures[ind]->def)
		&& s->textures[ind]->type == glTexLabel
		&& streq(text, s->textures[ind]->text)
		&& s->textures[ind]->fontSize==fs) {
		s->textures[ind]->userCount++;
		return s->textures[ind];
	    }
	}
    }


    data = glCompCreatePangoTexture(def, fs, text, &surface, &width, &height);
    if (!data) // pango error
	return NULL;
    t = glCompSetAddNewTexture(s, width, height, data, is2D);
    cairo_surface_destroy(surface);
    if (!t) {
	free(data);
	return NULL;
    }

    t->def = gv_strdup(def);
    t->text = gv_strdup(text);
    t->type = glTexLabel;
    return t;
}

void glCompDeleteTexture(glCompTex * t)
{
    if (!t)
	return;
    t->userCount--;
    if (!t->userCount) {
	free(t->data);
	free(t->def);
	free(t->text);
	free(t);
    }
}
