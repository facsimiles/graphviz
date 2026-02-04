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

#include <gvc/gvplugin_device.h>
#include <IL/il.h>
#include <IL/ilu.h>
#include <stdint.h>
#include <string.h>
#include <util/gv_math.h>

/// flip the row ordering of image data
///
/// @param width Width of the image in pixels
/// @param height Height of the image in pixels
/// @param data [in,out] Image content in 4-byte-per-pixel, row-major format
static void Y_inv(unsigned width, unsigned height, unsigned char *data) {
        unsigned int x, y;

        unsigned char *a = data;
        unsigned char *b = a + (height - 1) * width * sizeof(uint32_t);
        for (y = 0; y < height/2; y++) {
                for (x = 0; x < width; x++) {
			uint32_t t;
			memcpy(&t, a, sizeof(t));
			memcpy(a, b, sizeof(t));
			a += sizeof(uint32_t);
			memcpy(b, &t, sizeof(t));
			b += sizeof(uint32_t);
                }
                b -= 2 * width * sizeof(uint32_t);
        }
}

static void devil_format(GVJ_t * job)
{
    ILuint	ImgId;

    // Check if the shared lib's version matches the executable's version.
    if (ilGetInteger(IL_VERSION_NUM) < IL_VERSION ||
    	iluGetInteger(ILU_VERSION_NUM) < ILU_VERSION) {
    	fprintf(stderr, "DevIL version is different...exiting!\n");
    }

    // Initialize DevIL.
    ilInit();

    // Generate the main image name to use.
    ilGenImages(1, &ImgId);

    // Bind this image name.
    ilBindImage(ImgId);

    // cairo's in-memory image format needs inverting for DevIL
    Y_inv ( job->width, job->height, job->imagedata );
    
    // let the DevIL do its thing
    (void)ilTexImage(job->width, job->height,
    		1,		// Depth
    		BYTES_PER_PIXEL,
    		IL_BGRA,	// Format
    		IL_UNSIGNED_BYTE,// Type
    		job->imagedata);

    // check the last step succeeded
    ILenum Error = ilGetError();
    if (Error == 0) {
    	// output to the provided open file handle
    	ilSaveF((ILenum)job->device.id, job->output_file);
    } else {
    	fprintf(stderr, "Error: %s\n", iluErrorString(Error));
    }
    
    // We're done with the image, so delete it.
    ilDeleteImages(1, &ImgId);
    
    // Simple Error detection loop that displays the Error to the user in a human-readable form.
    while ((Error = ilGetError())) {
    	fprintf(stderr, "Error: %s\n", iluErrorString(Error));
    }
}

static gvdevice_engine_t devil_engine = {
    NULL,		/* devil_initialize */
    devil_format,
    NULL,		/* devil_finalize */
};

static gvdevice_features_t device_features_devil = {
	GVDEVICE_BINARY_FORMAT        
          | GVDEVICE_NO_WRITER
          | GVDEVICE_DOES_TRUECOLOR,/* flags */
	{0.,0.},                    /* default margin - points */
	{0.,0.},                    /* default page width, height - points */
	{96.,96.},                  /* svg 72 dpi */
};

gvplugin_installed_t gvdevice_devil_types[] = {
    {IL_BMP, "bmp:cairo", -1, &devil_engine, &device_features_devil},
    {IL_JPG, "jpg:cairo", -1, &devil_engine, &device_features_devil},
    {IL_JPG, "jpe:cairo", -1, &devil_engine, &device_features_devil},
    {IL_JPG, "jpeg:cairo", -1, &devil_engine, &device_features_devil},
    {IL_PNG, "png:cairo", -1, &devil_engine, &device_features_devil},
    {IL_TIF, "tif:cairo", -1, &devil_engine, &device_features_devil},
    {IL_TIF, "tiff:cairo", -1, &devil_engine, &device_features_devil},
    {IL_TGA, "tga:cairo", -1, &devil_engine, &device_features_devil},
    {0, NULL, 0, NULL, NULL}
};
