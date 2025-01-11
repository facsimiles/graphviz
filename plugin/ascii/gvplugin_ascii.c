/// @file
/// @brief Device that renders to ASCII art
///
/// This device does not attempt to examine your terminal dimensions. It simply
/// assumes you want output of the dimensions of the renderer graph.

#include <aalib.h>
#include <assert.h>
#include <gvc/gvplugin.h>
#include <gvc/gvplugin_device.h>
#include <limits.h>
#include <stdbool.h>
#include <stddef.h>
#include <util/gv_math.h>

/// convert an RGB color to grayscale
static int rgb_to_grayscale(unsigned red, unsigned green, unsigned blue) {

  /// use “perceptual” scaling,
  /// https://en.wikipedia.org/wiki/Grayscale#Colorimetric_(perceptual_luminance-preserving)_conversion_to_grayscale

  const double r_linear = red / 255.0;
  const double g_linear = green / 255.0;
  const double b_linear = blue / 255.0;

  const double y_linear =
      0.2126 * r_linear + 0.7152 * g_linear + 0.0722 * b_linear;
  return (int)(y_linear * 255.999);
}

/// does the given range only contain space characters?
static bool is_space(const unsigned char *base, size_t size) {
  assert(base != NULL || size == 0);
  // essentially the inverse of memchr, `memcchr(base, ' ', size)`
  for (size_t i = 0; i < size; ++i) {
    if (base[i] != ' ') {
      return false;
    }
  }
  return true;
}

static void process(GVJ_t *job) {
  assert(job != NULL);

  assert(job->width <= INT_MAX);
  const int width = (int)job->width;
  assert(job->height <= INT_MAX);
  const int height = (int)job->height;

  // initialize an in-memory device of the dimensions of our image
  // XXX: Reading the AA-lib docs, one might be led to believe we could simply
  // call `aa_autoinit` and render to the current terminal. However if you
  // attempt this AA-lib, despite seeming to read and understand the terminal
  // dimensions, renders something of the wrong dimensions. To work around this,
  // we render to an in-memory buffer and then strip the surrounding excess
  // space when doing our own rendering.
  aa_hardwareparams params = aa_defparams;
  params.width = width;
  params.height = height;
  aa_context *ctx = aa_init(&mem_d, &params, NULL);
  if (ctx == NULL) {
    agerrorf("failed to initialized AA-lib\n");
    return;
  }

  // draw the image
  const unsigned char *const data = job->imagedata;
  for (unsigned y = 0; y < job->height; ++y) {
    for (unsigned x = 0; x < job->width; ++x) {

      // extract the pixel data
      const unsigned offset =
          y * job->width * BYTES_PER_PIXEL + x * BYTES_PER_PIXEL;
      const unsigned red = data[offset + 2];
      const unsigned green = data[offset + 1];
      const unsigned blue = data[offset];

      const int gray = rgb_to_grayscale(red, green, blue);

      assert(gray >= 0 && gray < 256);
      aa_putpixel(ctx, (int)x, (int)y, gray);
    }
  }

  // render the image in memory
  aa_fastrender(ctx, 0, 0, width, height);
  aa_flush(ctx);

  // now render it from there to stdout, accounting for the quirk of the wrong
  // dimensions as discussed above
  const unsigned char *const text = aa_text(ctx);
  const size_t size = job->height * job->width;
  for (size_t y = 0; y < job->height; ++y) {
    // stop if we are into the lower excess white space
    if (is_space(&text[y * job->width], size - y * job->width)) {
      break;
    }
    for (size_t x = 0; x < job->width; ++x) {
      // stop if we are into the right excess white space
      if (is_space(&text[y * job->width + x], job->width - x)) {
        break;
      }
      printf("%c", (char)text[y * job->width + x]);
    }
    printf("\n");
  }

  aa_close(ctx);
}

static gvdevice_engine_t engine = {
    .format = process,
};

static gvdevice_features_t device_features = {
    .default_dpi = {96, 96},
};

static gvplugin_installed_t device_types[] = {
    {1, "ascii:cairo", 0, &engine, &device_features},
    {0},
};

static gvplugin_api_t apis[] = {{API_device, device_types}, {0}};

#ifdef GVDLL
#define GVPLUGIN_ASCII_API __declspec(dllexport)
#else
#define GVPLUGIN_ASCII_API
#endif

GVPLUGIN_ASCII_API gvplugin_library_t gvplugin_ascii_LTX_library = {"ascii",
                                                                    apis};
