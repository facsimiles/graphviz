/// \file
/// \brief Device that renders using ANSI terminal colors

#include <assert.h>
#include <gvc/gvplugin.h>
#include <gvc/gvplugin_device.h>
#include <limits.h>
#include <stddef.h>
#include <util/gv_math.h>

#include <gvc/gvio.h>

/// an ANSI color
typedef struct {
  unsigned value;
  unsigned red;
  unsigned green;
  unsigned blue;

} color_t;

/// ANSI 3-bit colors
static const color_t COLORS[] = {
    {0, 0x00, 0x00, 0x00}, ///< black
    {1, 0xff, 0x00, 0x00}, ///< red
    {2, 0x00, 0xff, 0x00}, ///< green
    {3, 0xff, 0xff, 0x00}, ///< yellow
    {4, 0x00, 0x00, 0xff}, ///< blue
    {5, 0xff, 0x00, 0xff}, ///< magenta
    {6, 0x00, 0xff, 0xff}, ///< cyan
    {7, 0xff, 0xff, 0xff}, ///< white
};

/// a metric of “closeness” to a given color
static unsigned distance(const color_t base, unsigned red, unsigned green,
                         unsigned blue) {
  unsigned diff = 0;
  diff += red > base.red ? red - base.red : base.red - red;
  diff += green > base.green ? green - base.green : base.green - green;
  diff += blue > base.blue ? blue - base.blue : base.blue - blue;
  return diff;
}

/// find closest ANSI color
static unsigned get_color(unsigned red, unsigned green, unsigned blue) {
  unsigned winner = 0;
  unsigned diff = UINT_MAX;
  for (size_t i = 0; i < sizeof(COLORS) / sizeof(COLORS[0]); ++i) {
    unsigned d = distance(COLORS[i], red, green, blue);
    if (d < diff) {
      diff = d;
      winner = COLORS[i].value;
    }
  }
  return winner;
}

static void process(GVJ_t *job, int color_depth) {

  unsigned char *data = job->imagedata;

  assert(color_depth == 3 || color_depth == 24);

  for (unsigned y = 0; y < job->height; y += 2) {
    for (unsigned x = 0; x < job->width; ++x) {

      {
        // extract the upper pixel
        unsigned offset =
            y * job->width * BYTES_PER_PIXEL + x * BYTES_PER_PIXEL;
        unsigned red = data[offset + 2];
        unsigned green = data[offset + 1];
        unsigned blue = data[offset];

        // use this to select a foreground color
        if (color_depth == 3) {
          unsigned fg = get_color(red, green, blue);
          gvprintf(job, "\033[3%um", fg);
        } else {
          assert(color_depth == 24);
          gvprintf(job, "\033[38;2;%u;%u;%um", red, green, blue);
        }
      }

      {
        // extract the lower pixel
        unsigned red = 0;
        unsigned green = 0;
        unsigned blue = 0;
        if (y + 1 < job->height) {
          unsigned offset =
              (y + 1) * job->width * BYTES_PER_PIXEL + x * BYTES_PER_PIXEL;
          red = data[offset + 2];
          green = data[offset + 1];
          blue = data[offset];
        }

        // use this to select a background color
        if (color_depth == 3) {
          unsigned bg = get_color(red, green, blue);
          gvprintf(job, "\033[4%um", bg);
        } else {
          assert(color_depth == 24);
          gvprintf(job, "\033[48;2;%u;%u;%um", red, green, blue);
        }
      }

      // print unicode “upper half block” to effectively do two rows of
      // pixels per one terminal row
      gvprintf(job, "▀\033[0m");
    }
    gvprintf(job, "\n");
  }
}

static void process3(GVJ_t *job) { process(job, 3); }

static void process24(GVJ_t *job) { process(job, 24); }

/// convert an RGB color to grayscale
static unsigned rgb_to_grayscale(unsigned red, unsigned green, unsigned blue) {

  /// use “perceptual” scaling,
  /// https://en.wikipedia.org/wiki/Grayscale#Colorimetric_(perceptual_luminance-preserving)_conversion_to_grayscale

  const double r_linear = red / 255.0;
  const double g_linear = green / 255.0;
  const double b_linear = blue / 255.0;

  const double y_linear =
      0.2126 * r_linear + 0.7152 * g_linear + 0.0722 * b_linear;
  return (unsigned)(y_linear * 255.999);
}

/// draw a y_stride×x_stride-pixels-per-character monochrome image
///
/// @param job GVC job to operate on
/// @param y_stride How many Y pixels fit in a character
/// @param x_stride How many X pixels fit in a character
/// @param tiles In-order list of characters for each representation
static void processNup(GVJ_t *job, unsigned y_stride, unsigned x_stride,
                       const char **tiles) {
  assert(y_stride > 0);
  assert(x_stride > 0);
  assert(tiles != NULL);
  for (unsigned i = 0; i < y_stride; ++i) {
    for (unsigned j = 0; j < x_stride; ++j) {
      assert(tiles[i * x_stride + j] != NULL && "missing or not enough tiles");
    }
  }

  unsigned char *data = job->imagedata;

  for (unsigned y = 0; y < job->height; y += y_stride) {
    for (unsigned x = 0; x < job->width; x += x_stride) {

      unsigned index = 0;

      for (unsigned y_offset = 0;
           y + y_offset < job->height && y_offset < y_stride; ++y_offset) {
        for (unsigned x_offset = 0;
             x + x_offset < job->width && x_offset < x_stride; ++x_offset) {

          const unsigned offset =
              (y + y_offset) * job->width * BYTES_PER_PIXEL +
              (x + x_offset) * BYTES_PER_PIXEL;
          const unsigned red = data[offset + 2];
          const unsigned green = data[offset + 1];
          const unsigned blue = data[offset];

          const unsigned gray = rgb_to_grayscale(red, green, blue);
          // The [0, 256) grayscale measurement can be quantized into 16
          // 16-stride buckets. I.e. [0, 16) as bucket 1, [16, 32) as bucket 2,
          // … Drawing a threshold at 240, and considering only the last bucket
          // to be white when converting to monochrome empirically seems to
          // generate reasonable results.
          const unsigned pixel = gray >= 240;

          index |= pixel << (y_offset * x_stride + x_offset);
        }
      }

      gvputs(job, tiles[index]);
    }
    gvputc(job, '\n');
  }
}

/// draw a 4-pixels-per-character monochrome image
static void process4up(GVJ_t *job) {
  // block characters from the “Amstrad CPC character set”
  const char *tiles[] = {" ", "▘", "▝", "▀", "▖", "▍", "▞", "▛",
                         "▗", "▚", "▐", "▜", "▃", "▙", "▟", "█"};
  const unsigned y_stride = 2;
  const unsigned x_stride = 2;
  assert(sizeof(tiles) / sizeof(tiles[0]) == 1 << (y_stride * x_stride));
  processNup(job, y_stride, x_stride, tiles);
}

/// draw a 6-pixels-per-character monochrome image
static void process6up(GVJ_t *job) {
  // the “Teletext G1 Block Mosaics Set”
  const char *tiles[] = {" ", "🬀", "🬁", "🬂", "🬃", "🬄", "🬅", "🬆", "🬇", "🬈", "🬉",
                         "🬊", "🬋", "🬌", "🬍", "🬎", "🬏", "🬐", "🬑", "🬒", "🬓", "▌",
                         "🬔", "🬕", "🬖", "🬗", "🬘", "🬙", "🬚", "🬛", "🬜", "🬝", "🬞",
                         "🬟", "🬠", "🬡", "🬢", "🬣", "🬤", "🬥", "🬦", "🬧", "▐", "🬨",
                         "🬩", "🬪", "🬫", "🬬", "🬭", "🬮", "🬯", "🬰", "🬱", "🬲", "🬳",
                         "🬴", "🬵", "🬶", "🬷", "🬸", "🬹", "🬺", "🬻", "█"};
  const unsigned y_stride = 3;
  const unsigned x_stride = 2;
  assert(sizeof(tiles) / sizeof(tiles[0]) == 1 << (y_stride * x_stride));
  processNup(job, y_stride, x_stride, tiles);
}

/// draw a 8-pixels-per-character monochrome image
static void process8up(GVJ_t *job) {
  // the Unicode “Braille Patterns” block
  const char *tiles[] = {
      " ", "⠁", "⠈", "⠉", "⠂", "⠃", "⠊", "⠋", "⠐", "⠑", "⠘", "⠙", "⠒", "⠓", "⠚",
      "⠛", "⠄", "⠅", "⠌", "⠍", "⠆", "⠇", "⠎", "⠏", "⠔", "⠕", "⠜", "⠝", "⠖", "⠗",
      "⠞", "⠟", "⠠", "⠡", "⠨", "⠩", "⠢", "⠣", "⠪", "⠫", "⠰", "⠱", "⠸", "⠹", "⠲",
      "⠳", "⠺", "⠻", "⠤", "⠥", "⠬", "⠭", "⠦", "⠧", "⠮", "⠯", "⠴", "⠵", "⠼", "⠽",
      "⠶", "⠷", "⠾", "⠿", "⡀", "⡁", "⡈", "⡉", "⡂", "⡃", "⡊", "⡋", "⡐", "⡑", "⡘",
      "⡙", "⡒", "⡓", "⡚", "⡛", "⡄", "⡅", "⡌", "⡍", "⡆", "⡇", "⡎", "⡏", "⡔", "⡕",
      "⡜", "⡝", "⡖", "⡗", "⡞", "⡟", "⡠", "⡡", "⡨", "⡩", "⡢", "⡣", "⡪", "⡫", "⡰",
      "⡱", "⡸", "⡹", "⡲", "⡳", "⡺", "⡻", "⡤", "⡥", "⡬", "⡭", "⡦", "⡧", "⡮", "⡯",
      "⡴", "⡵", "⡼", "⡽", "⡶", "⡷", "⡾", "⡿", "⢀", "⢁", "⢈", "⢉", "⢂", "⢃", "⢊",
      "⢋", "⢐", "⢑", "⢘", "⢙", "⢒", "⢓", "⢚", "⢛", "⢄", "⢅", "⢌", "⢍", "⢆", "⢇",
      "⢎", "⢏", "⢔", "⢕", "⢜", "⢝", "⢖", "⢗", "⢞", "⢟", "⢠", "⢡", "⢨", "⢩", "⢢",
      "⢣", "⢪", "⢫", "⢰", "⢱", "⢸", "⢹", "⢲", "⢳", "⢺", "⢻", "⢤", "⢥", "⢬", "⢭",
      "⢦", "⢧", "⢮", "⢯", "⢴", "⢵", "⢼", "⢽", "⢶", "⢷", "⢾", "⢿", "⣀", "⣁", "⣈",
      "⣉", "⣂", "⣃", "⣊", "⣋", "⣐", "⣑", "⣘", "⣙", "⣒", "⣓", "⣚", "⣛", "⣄", "⣅",
      "⣌", "⣍", "⣆", "⣇", "⣎", "⣏", "⣔", "⣕", "⣜", "⣝", "⣖", "⣗", "⣞", "⣟", "⣠",
      "⣡", "⣨", "⣩", "⣢", "⣣", "⣪", "⣫", "⣰", "⣱", "⣸", "⣹", "⣲", "⣳", "⣺", "⣻",
      "⣤", "⣥", "⣬", "⣭", "⣦", "⣧", "⣮", "⣯", "⣴", "⣵", "⣼", "⣽", "⣶", "⣷", "⣾",
      "⣿"};
  const unsigned y_stride = 4;
  const unsigned x_stride = 2;
  assert(sizeof(tiles) / sizeof(tiles[0]) == 1 << (y_stride * x_stride));
  processNup(job, y_stride, x_stride, tiles);
}

static gvdevice_engine_t engine3 = {
    .format = process3,
};

static gvdevice_engine_t engine24 = {
    .format = process24,
};

static gvdevice_engine_t engine4up = {
    .format = process4up,
};

static gvdevice_engine_t engine6up = {
    .format = process6up,
};

static gvdevice_engine_t engine8up = {
    .format = process8up,
};

static gvdevice_features_t device_features = {
    .default_dpi = {96, 96},
};

static gvplugin_installed_t device_types[] = {
    {8, "vt:cairo", 0, &engine3, &device_features},
    {1 << 24, "vt-24bit:cairo", 0, &engine24, &device_features},
    {4, "vt-4up:cairo", 0, &engine4up, &device_features},
    {6, "vt-6up:cairo", 0, &engine6up, &device_features},
    {7, "vt-8up:cairo", 0, &engine8up, &device_features},
    {0},
};

static gvplugin_api_t apis[] = {
    {API_device, device_types},
    {(api_t)0, 0},
};

#ifdef GVDLL
#define GVPLUGIN_VT_API __declspec(dllexport)
#else
#define GVPLUGIN_VT_API
#endif

GVPLUGIN_VT_API gvplugin_library_t gvplugin_vt_LTX_library = {"vt", apis};
