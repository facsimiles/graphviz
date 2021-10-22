/*************************************************************************
 * Copyright (c) 2011 AT&T Intellectual Property 
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *
 * Contributors: Details at https://graphviz.org
 *************************************************************************/
#ifdef _WIN32
#include <windows.h>
#include <io.h>
#else
#include <unistd.h>
#endif
#include "viewport.h"
#include "draw.h"
#include <common/color.h>
#include <glade/glade.h>
#include "gui.h"
#include "menucallbacks.h"
#include <stddef.h>
#include <string.h>
#include "glcompui.h"
#include "gltemplate.h"
#include <common/colorprocs.h>
#include <common/memory.h>
#include "topviewsettings.h"
#include "arcball.h"
#include "hotkeymap.h"
#include "topviewfuncs.h"
#include <cgraph/strcasecmp.h>


static colorschemaset *create_color_theme(int themeid);

ViewInfo *view;
/* these two global variables should be wrapped in something else */
GtkMessageDialog *Dlg;

static void clear_viewport(ViewInfo * view)
{
    /*free topview if there is one */
    if (view->activeGraph >= 0)
    {
	freeSmGraph(view->g[view->activeGraph],view->Topview);
    }
    if (view->graphCount)
	agclose(view->g[view->activeGraph]);
}
static void *get_glut_font(int ind)
{

    switch (ind) {
    case 0:
	return GLUT_BITMAP_9_BY_15;
	break;
    case 1:
	return GLUT_BITMAP_8_BY_13;
	break;
    case 2:
	return GLUT_BITMAP_TIMES_ROMAN_10;
	break;
    case 3:
	return GLUT_BITMAP_HELVETICA_10;
	break;
    case 4:
	return GLUT_BITMAP_HELVETICA_12;
	break;
    case 5:
	return GLUT_BITMAP_HELVETICA_18;
	break;
    default:
	return GLUT_BITMAP_TIMES_ROMAN_10;
    }

}

int close_graph(ViewInfo * view, int graphid)
{
    if (view->activeGraph < 0)
	return 1;
    clear_viewport(view);
    return 1;

}

char *get_attribute_value(char *attr, ViewInfo * view, Agraph_t * g)
{
    char *buf;
    buf = agget(g, attr);
    if ((!buf) || (*buf == '\0'))
	buf = agget(view->systemGraphs.def_attrs, attr);
    return buf;
}

void set_viewport_settings_from_template(ViewInfo * view, Agraph_t * g)
{
    gvcolor_t cl;
    char *buf;
    colorxlate(get_attribute_value("bordercolor", view, g), &cl,
	       RGBA_DOUBLE);
    view->borderColor.R = (float) cl.u.RGBA[0];
    view->borderColor.G = (float) cl.u.RGBA[1];
    view->borderColor.B = (float) cl.u.RGBA[2];

    view->borderColor.A =
	(float) atof(get_attribute_value("bordercoloralpha", view, g));

    view->bdVisible = atoi(get_attribute_value("bordervisible", view, g));

    buf = get_attribute_value("gridcolor", view, g);
    colorxlate(buf, &cl, RGBA_DOUBLE);
    view->gridColor.R = (float) cl.u.RGBA[0];
    view->gridColor.G = (float) cl.u.RGBA[1];
    view->gridColor.B = (float) cl.u.RGBA[2];
    view->gridColor.A =
	(float) atof(get_attribute_value("gridcoloralpha", view, g));

    view->gridSize = (float) atof(buf =
				  get_attribute_value("gridsize", view,
						      g));

    view->defaultnodeshape = atoi(buf =
				  get_attribute_value("defaultnodeshape",
						      view, g));

    view->gridVisible = atoi(get_attribute_value("gridvisible", view, g));

    //background color , default white
    colorxlate(get_attribute_value("bgcolor", view, g), &cl, RGBA_DOUBLE);

    view->bgColor.R = (float) cl.u.RGBA[0];
    view->bgColor.G = (float) cl.u.RGBA[1];
    view->bgColor.B = (float) cl.u.RGBA[2];
    view->bgColor.A = (float) 1;

    //selected nodes are drawn with this color
    colorxlate(get_attribute_value("selectednodecolor", view, g), &cl,
	       RGBA_DOUBLE);
    view->selectedNodeColor.R = (float) cl.u.RGBA[0];
    view->selectedNodeColor.G = (float) cl.u.RGBA[1];
    view->selectedNodeColor.B = (float) cl.u.RGBA[2];
    view->selectedNodeColor.A = (float)
	atof(get_attribute_value("selectednodecoloralpha", view, g));
    //selected edge are drawn with this color
    colorxlate(get_attribute_value("selectededgecolor", view, g), &cl,
	       RGBA_DOUBLE);
    view->selectedEdgeColor.R = (float) cl.u.RGBA[0];
    view->selectedEdgeColor.G = (float) cl.u.RGBA[1];
    view->selectedEdgeColor.B = (float) cl.u.RGBA[2];
    view->selectedEdgeColor.A = (float)
	atof(get_attribute_value("selectededgecoloralpha", view, g));

    colorxlate(get_attribute_value("highlightednodecolor", view, g), &cl,
	       RGBA_DOUBLE);
    view->highlightedNodeColor.R = (float) cl.u.RGBA[0];
    view->highlightedNodeColor.G = (float) cl.u.RGBA[1];
    view->highlightedNodeColor.B = (float) cl.u.RGBA[2];
    view->highlightedNodeColor.A = (float)
	atof(get_attribute_value("highlightednodecoloralpha", view, g));

    buf = agget(g, "highlightededgecolor");
    colorxlate(get_attribute_value("highlightededgecolor", view, g), &cl,
	       RGBA_DOUBLE);
    view->highlightedEdgeColor.R = (float) cl.u.RGBA[0];
    view->highlightedEdgeColor.G = (float) cl.u.RGBA[1];
    view->highlightedEdgeColor.B = (float) cl.u.RGBA[2];
    view->highlightedEdgeColor.A = (float)
	atof(get_attribute_value("highlightededgecoloralpha", view, g));
    view->defaultnodealpha = (float)
	atof(get_attribute_value("defaultnodealpha", view, g));

    view->defaultedgealpha = (float)
	atof(get_attribute_value("defaultedgealpha", view, g));



    /*default line width */
    view->LineWidth =
	(float) atof(get_attribute_value("defaultlinewidth", view, g));
    view->FontSize =
	(float) atof(get_attribute_value("defaultfontsize", view, g));

    view->topviewusermode = atoi(get_attribute_value("usermode", view, g));
    get_attribute_value("defaultmagnifierwidth", view, g);
    view->mg.width =
	atoi(get_attribute_value("defaultmagnifierwidth", view, g));
    view->mg.height =
	atoi(get_attribute_value("defaultmagnifierheight", view, g));

    view->mg.kts =
	(float) atof(get_attribute_value("defaultmagnifierkts", view, g));

    view->fmg.constantR =
	atoi(get_attribute_value
	     ("defaultfisheyemagnifierradius", view, g));

    view->fmg.fisheye_distortion_fac =
	atoi(get_attribute_value
	     ("defaultfisheyemagnifierdistort", view, g));
    view->drawnodes = atoi(get_attribute_value("drawnodes", view, g));
    view->drawedges = atoi(get_attribute_value("drawedges", view, g));
    view->drawnodelabels=atoi(get_attribute_value("labelshownodes", view, g));
    view->drawedgelabels=atoi(get_attribute_value("labelshowedges", view, g));
    view->nodeScale=atof(get_attribute_value("nodesize", view, g))*.30;

    view->glutfont =
	get_glut_font(atoi(get_attribute_value("labelglutfont", view, g)));
    colorxlate(get_attribute_value("nodelabelcolor", view, g), &cl,
	       RGBA_DOUBLE);
    view->nodelabelcolor.R = (float) cl.u.RGBA[0];
    view->nodelabelcolor.G = (float) cl.u.RGBA[1];
    view->nodelabelcolor.B = (float) cl.u.RGBA[2];
    view->nodelabelcolor.A =
	(float) atof(get_attribute_value("defaultnodealpha", view, g));
    colorxlate(get_attribute_value("edgelabelcolor", view, g), &cl,
	       RGBA_DOUBLE);
    view->edgelabelcolor.R = (float) cl.u.RGBA[0];
    view->edgelabelcolor.G = (float) cl.u.RGBA[1];
    view->edgelabelcolor.B = (float) cl.u.RGBA[2];
    view->edgelabelcolor.A =
	(float) atof(get_attribute_value("defaultedgealpha", view, g));
    view->labelwithdegree =
	atoi(get_attribute_value("labelwithdegree", view, g));
    view->labelnumberofnodes =
	atof(get_attribute_value("labelnumberofnodes", view, g));
    view->labelshownodes =
	atoi(get_attribute_value("labelshownodes", view, g));
    view->labelshowedges =
	atoi(get_attribute_value("labelshowedges", view, g));
    view->colschms =
	create_color_theme(atoi
			   (get_attribute_value("colortheme", view, g)));
    view->edgerendertype=atoi(get_attribute_value("edgerender", view, g));


    if (view->graphCount > 0)
	glClearColor(view->bgColor.R, view->bgColor.G, view->bgColor.B, view->bgColor.A);	//background color
}

static gboolean gl_main_expose(gpointer data)
{
    if (view->activeGraph >= 0) {
	if (view->Topview->fisheyeParams.animate == 1)
	    expose_event(view->drawing_area, NULL, NULL);
	return 1;
    }
    return 1;
}

static void get_data_dir(void)
{
    if (view->template_file) {
	free(view->template_file);
    }

    view->template_file = strdup(smyrnaPath("template.dot"));
}

void init_viewport(ViewInfo * view)
{
    FILE *input_file = NULL;
    FILE *input_file2 = NULL;
    static char* path;
    get_data_dir();
    input_file = fopen(view->template_file, "rb");
    if (!input_file) {
	fprintf(stderr,
		"default attributes template graph file \"%s\" not found\n",
		view->template_file);
	exit(-1);
    } 
    view->systemGraphs.def_attrs = agread(input_file, NULL);
    fclose (input_file);

    if (!view->systemGraphs.def_attrs) {
	fprintf(stderr,
		"could not load default attributes template graph file \"%s\"\n",
		view->template_file);
	exit(-1);
    }
    if (!path)
	path = smyrnaPath("attr_widgets.dot");
    input_file2 = fopen(path, "rb");
    if (!input_file2) {
	fprintf(stderr,	"default attributes template graph file \"%s\" not found\n",smyrnaPath("attr_widgets.dot"));
	exit(-1);

    }
    view->systemGraphs.attrs_widgets = agread(input_file2, NULL);
    fclose (input_file2);
    if (!(view->systemGraphs.attrs_widgets )) {
	fprintf(stderr,"could not load default attribute widgets graph file \"%s\"\n",smyrnaPath("attr_widgets.dot"));
	exit(-1);
    }
    //init graphs
    view->g = NULL;		//no graph, gl screen should check it
    view->graphCount = 0;	//and disable interactivity if count is zero

    view->bdxLeft = 0;
    view->bdxRight = 500;
    view->bdyBottom = 0;
    view->bdyTop = 500;

    view->borderColor.R = 1;
    view->borderColor.G = 0;
    view->borderColor.B = 0;
    view->borderColor.A = 1;

    view->bdVisible = 1;	//show borders red

    view->gridSize = 10;
    view->gridColor.R = 0.5;
    view->gridColor.G = 0.5;
    view->gridColor.B = 0.5;
    view->gridColor.A = 1;
    view->gridVisible = 0;	//show grids in light gray

    //mouse mode=pan
    //pen color
    view->penColor.R = 0;
    view->penColor.G = 0;
    view->penColor.B = 0;
    view->penColor.A = 1;

    view->fillColor.R = 1;
    view->fillColor.G = 0;
    view->fillColor.B = 0;
    view->fillColor.A = 1;
    //background color , default white
    view->bgColor.R = 1;
    view->bgColor.G = 1;
    view->bgColor.B = 1;
    view->bgColor.A = 1;

    //selected objets are drawn with this color
    view->selectedNodeColor.R = 1;
    view->selectedNodeColor.G = 0;
    view->selectedNodeColor.B = 0;
    view->selectedNodeColor.A = 1;

    //default line width;
    view->LineWidth = 1;

    //default view settings , camera is not active
    view->panx = 0;
    view->pany = 0;
    view->panz = 0;


    view->zoom = -20;
    view->FontSize = 52;

    view->topviewusermode = TOP_VIEW_USER_NOVICE_MODE;	//for demo
    view->mg.active = 0;
    view->mg.x = 0;
    view->mg.y = 0;
    view->mg.width = DEFAULT_MAGNIFIER_WIDTH;
    view->mg.height = DEFAULT_MAGNIFIER_HEIGHT;
    view->mg.kts = DEFAULT_MAGNIFIER_KTS;
    view->fmg.constantR = DEFAULT_FISHEYE_MAGNIFIER_RADIUS;
    view->fmg.active = 0;
    view->mouse.down = 0;
    view->activeGraph = -1;
    view->Topview = GNEW(topview);
    view->Topview->fisheyeParams.fs = 0;
    view->Topview->xDot=NULL;

    /* init topfish parameters */
    view->Topview->fisheyeParams.level.num_fine_nodes = 10;
    view->Topview->fisheyeParams.level.coarsening_rate = 2.5;
    view->Topview->fisheyeParams.hier.dist2_limit = 1;
    view->Topview->fisheyeParams.hier.min_nvtxs = 20;
    view->Topview->fisheyeParams.repos.rescale = Polar;
    view->Topview->fisheyeParams.repos.width =
	(int) (view->bdxRight - view->bdxLeft);
    view->Topview->fisheyeParams.repos.height =
	(int) (view->bdyTop - view->bdyBottom);
    view->Topview->fisheyeParams.repos.margin = 0;
    view->Topview->fisheyeParams.repos.graphSize = 100;
    view->Topview->fisheyeParams.repos.distortion = 1.0;
    /*create timer */
    view->timer = g_timer_new();
    view->timer2 = g_timer_new();
    view->timer3 = g_timer_new();

    g_timer_stop(view->timer);
    view->active_frame = 0;
    view->total_frames = 1500;
    /*add a call back to the main() */
    g_timeout_add_full((gint) G_PRIORITY_DEFAULT, (guint) 100,
		       gl_main_expose, NULL, NULL);
    view->cameras = '\0';;
    view->camera_count = 0;
    view->active_camera = -1;
    set_viewport_settings_from_template(view, view->systemGraphs.def_attrs);
    view->Topview->Graphdata.GraphFileName = (char *) 0;
    view->Topview->Graphdata.Modified = 0;
    view->colschms = NULL;
    view->arcball = NEW(ArcBall_t);
    view->keymap.down=0;
    load_mouse_actions (view);
    view->refresh.color=1;
    view->refresh.pos=1;
    view->refresh.selection=1;
    view->refresh.visibility=1;
    view->refresh.nodesize=1;
    view->edgerendertype=0;
    if(view->guiMode!=GUI_FULLSCREEN)
	view->guiMode=GUI_WINDOWED;

    /*create glcomp menu system */
    view->widgets = glcreate_gl_topview_menu();

}


/* update_graph_params:
 * adds gledit params
 * assumes custom_graph_data has been attached to the graph.
 */
static void update_graph_params(Agraph_t * graph)
{
    agattr(graph, AGRAPH, "GraphFileName",
	   view->Topview->Graphdata.GraphFileName);
}

static Agraph_t *loadGraph(char *filename)
{
    Agraph_t *g;
    FILE *input_file;
    if (!(input_file = fopen(filename, "r"))) {
	g_print("Cannot open %s\n", filename);
	return 0;
    }
    g = agread(input_file, NULL);
    fclose (input_file);
    if (!g) {
	g_print("Cannot read graph in  %s\n", filename);
	return 0;
    }

    /* If no position info, run layout with -Txdot
     */
    if (!agattr(g, AGNODE, "pos", NULL)) {
	g_print("There is no position info in graph %s in %s\n", agnameof(g), filename);
	agclose (g);
	return 0;
    }
    view->Topview->Graphdata.GraphFileName = strdup(filename);
    return g;
}

/* add_graph_to_viewport_from_file:
 * returns 1 if successful else 0
 */
int add_graph_to_viewport_from_file(char *fileName)
{
    Agraph_t *graph = loadGraph(fileName);

    return add_graph_to_viewport(graph, fileName);
}

/* updateRecord:
 * Update fields which may be added dynamically.
 */
void updateRecord (Agraph_t* g)
{
    GN_size(g) = agattr (g, AGNODE, "size", 0);
    GN_visible(g) = agattr (g, AGNODE, "visible", 0);
    GN_selected(g) = agattr (g, AGNODE, "selected", 0);
    GN_labelattribute(g) = agattr (g, AGNODE, "nodelabelattribute", 0);

    GE_pos(g)=agattr(g,AGEDGE,"pos",0);
    GE_visible(g) = agattr (g, AGEDGE, "visible", 0);
    GE_selected(g) = agattr (g, AGEDGE, "selected", 0);
    GE_labelattribute(g) = agattr (g, AGEDGE, "edgelabelattribute", 0);
}

/* graphRecord:
 * add graphRec to graph if necessary.
 * update fields of graphRec.
 * We assume the graph has attributes nodelabelattribute, edgelabelattribute,
 * nodelabelcolor and edgelabelcolor from template.dot.
 * We assume nodes have pos attributes. 
 * Only size, visible, selected and edge pos may or may not be defined.
 */
static void
graphRecord (Agraph_t* g)
{
    agbindrec(g, "graphRec", sizeof(graphRec), 1);

    GG_nodelabelcolor(g) = agattr (g, AGRAPH, "nodelabelcolor", 0);
    GG_edgelabelcolor(g) = agattr (g, AGRAPH, "edgelabelcolor", 0);
    GG_labelattribute(g) = agattr (g, AGRAPH, "nodelabelattribute", 0);
    GG_elabelattribute(g) = agattr (g, AGRAPH, "edgelabelattribute", 0);

    GN_pos(g) = agattr (g, AGNODE, "pos", 0);


    updateRecord (g);
}

void refreshViewport(int doClear)
{
    Agraph_t *graph = view->g[view->activeGraph];
    view->refresh.color=1;
    view->refresh.nodesize=1;
    view->refresh.pos=1;
    view->refresh.selection=1;
    view->refresh.visibility=1;
    load_settings_from_graph(graph);

    if(view->guiMode!=GUI_FULLSCREEN)
	update_graph_from_settings(graph);
    set_viewport_settings_from_template(view, graph);
    graphRecord(graph);
    initSmGraph(graph,view->Topview);

    expose_event(view->drawing_area, NULL, NULL);
}

static void activate(int id, int doClear)
{
    view->activeGraph = id;
    refreshViewport(doClear);
}

int add_graph_to_viewport(Agraph_t * graph, char *id)
{
    if (graph) {
	view->graphCount = view->graphCount + 1;
	view->g = realloc(view->g,
				  sizeof(Agraph_t *) * view->graphCount);
	view->g[view->graphCount - 1] = graph;

	gtk_combo_box_append_text(view->graphComboBox, id);
	gtk_combo_box_set_active(view->graphComboBox,view->graphCount-1);
	activate(view->graphCount - 1, 0);
	return 1;
    } else {
	return 0;
    }

}
void switch_graph(int graphId)
{
    if ((graphId >= view->graphCount) || (graphId < 0))
	return;			/*wrong entry */
    else
	activate(graphId, 0);
}

/* save_graph_with_file_name:
 * saves graph with file name; if file name is NULL save as is
 */
int save_graph_with_file_name(Agraph_t * graph, char *fileName)
{
    int ret;
    FILE *output_file;
    update_graph_params(graph);
    if (fileName)
	output_file = fopen(fileName, "w");
    else if (view->Topview->Graphdata.GraphFileName)
	output_file = fopen(view->Topview->Graphdata.GraphFileName, "w");
    else {
	g_print("there is no file name to save! Programmer error\n");
	return 0;
    }
    if (output_file == NULL) {
	g_print("Cannot create file \n");
	return 0;
    }

    ret = agwrite(graph, output_file);
    fclose (output_file);
    if (ret) {
	g_print("%s successfully saved \n", fileName);
	return 1;
    }
    return 0;
}

/* save_graph:
 * save without prompt
 */
int save_graph(void)
{
    //check if there is an active graph
    if (view->activeGraph > -1) 
    {
	//check if active graph has a file name
	if (view->Topview->Graphdata.GraphFileName) {
	    return save_graph_with_file_name(view->g[view->activeGraph],
					     view->Topview->Graphdata.
					     GraphFileName);
	} else
	    return save_as_graph();
    }
    return 1;

}

/* save_as_graph:
 * save with prompt
 */
int save_as_graph(void)
{
    //check if there is an active graph
    if (view->activeGraph > -1) {
	GtkWidget *dialog;
	dialog = gtk_file_chooser_dialog_new("Save File",
					     NULL,
					     GTK_FILE_CHOOSER_ACTION_SAVE,
					     GTK_STOCK_CANCEL,
					     GTK_RESPONSE_CANCEL,
					     GTK_STOCK_SAVE,
					     GTK_RESPONSE_ACCEPT, NULL);
	gtk_file_chooser_set_do_overwrite_confirmation(GTK_FILE_CHOOSER
						       (dialog), TRUE);
	if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
	    char *filename;
	    filename =
		gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
	    save_graph_with_file_name(view->g[view->activeGraph],
				      filename);
	    g_free(filename);
	    gtk_widget_destroy(dialog);

	    return 1;
	} else {
	    gtk_widget_destroy(dialog);
	    return 0;
	}
    }
    return 0;
}

void glexpose(void)
{
    expose_event(view->drawing_area, NULL, NULL);
}

static float interpol(float minv, float maxv, float minc, float maxc, float x)
{
    return ((x - minv) * (maxc - minc) / (maxv - minv) + minc);
}

void getcolorfromschema(colorschemaset * sc, float l, float maxl,
			glCompColor * c)
{
    int ind;
    float percl = l / maxl;

    if (sc->smooth) {
	/* For smooth schemas, s[0].perc = 0, so we start with ind=1 */
	for (ind = 1; ind < sc->schemacount-1; ind++) {
	    if (percl < sc->s[ind].perc)
		break;
	}
	c->R =
	    interpol(sc->s[ind - 1].perc, sc->s[ind].perc,
		     sc->s[ind - 1].c.R, sc->s[ind].c.R, percl);
	c->G =
	    interpol(sc->s[ind - 1].perc, sc->s[ind].perc,
		     sc->s[ind - 1].c.G, sc->s[ind].c.G, percl);
	c->B =
	    interpol(sc->s[ind - 1].perc, sc->s[ind].perc,
		     sc->s[ind - 1].c.B, sc->s[ind].c.B, percl);
    }
    else {
	for (ind = 0; ind < sc->schemacount-1; ind++) {
	    if (percl < sc->s[ind].perc)
		break;
	}
	c->R = sc->s[ind].c.R;
	c->G = sc->s[ind].c.G;
	c->B = sc->s[ind].c.B;
    }

    c->A = 1;
}

/* set_color_theme_color:
 * Convert colors as strings to RGB
 */
static void set_color_theme_color(colorschemaset * sc, char **colorstr)
{
    int ind;
    int colorcnt = sc->schemacount;
    gvcolor_t cl;
    float av_perc;

    sc->smooth = 1;
    av_perc = 1.0 / (float) (colorcnt-1);
    for (ind = 0; ind < colorcnt; ind++) {
        colorxlate(colorstr[ind], &cl, RGBA_DOUBLE);
        sc->s[ind].c.R = cl.u.RGBA[0];
        sc->s[ind].c.G = cl.u.RGBA[1];
        sc->s[ind].c.B = cl.u.RGBA[2];
        sc->s[ind].c.A = cl.u.RGBA[3];
        sc->s[ind].perc = ind * av_perc;
    }
}

static void clear_color_theme(colorschemaset * cs)
{
    free(cs->s);
    free(cs);
}

static char *deep_blue[] = {
    "#C8CBED", "#9297D3", "#0000FF", "#2C2E41"
};
static char *pastel[] = {
    "#EBBE29", "#D58C4A", "#74AE09", "#893C49"
};
static char *magma[] = {
    "#E0061E", "#F0F143", "#95192B", "#EB712F"
};
static char *rain_forest[] = {
    "#1E6A10", "#2ABE0E", "#AEDD39", "#5EE88B"
};
#define CSZ(x) (sizeof(x)/sizeof(char*))
typedef struct {
    int cnt;
    char **colors;
} colordata;
static colordata palette[] = {
    {CSZ(deep_blue), deep_blue},
    {CSZ(pastel), pastel},
    {CSZ(magma), magma},
    {CSZ(rain_forest), rain_forest},
};
#define NUM_SCHEMES (sizeof(palette)/sizeof(colordata))

static colorschemaset *create_color_theme(int themeid)
{
    colorschemaset *s;

    if ((themeid < 0) || (NUM_SCHEMES <= themeid)) {
	fprintf (stderr, "colorschemaset: illegal themeid %d\n", themeid);
	return view->colschms;
    }

    s = NEW(colorschemaset);
    if (view->colschms)
	clear_color_theme(view->colschms);

    s->schemacount = palette[themeid].cnt;
    s->s = N_NEW(s->schemacount,colorschema);
    set_color_theme_color(s, palette[themeid].colors);

    return s;
}

