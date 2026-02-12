/// @file
/// @brief helpers for verbose/debug printing

#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <util/debug.h>
#include <util/gv_math.h>
#include <util/gv_time.h>
#include <util/lockfile.h>

unsigned char Verbose;
unsigned char Timing;

// Helper function to handle formatting and locking.
// This prevents code bloat and fixes double-evaluation issues.
void gv_log_impl(int precision, const char *file, int line, const char *fmt,
                 ...) {
  // 1. Get filename part
  const char *short_name = strrchr(file, '/');
  short_name = short_name ? short_name + 1 : file;

  // 2. Prepare Timestamp
  struct timespec ts;
  struct tm tm_info;
  char time_buf[64];

  // Default fallback
  strcpy(time_buf, "0000-00-00 00:00:00");

  if (get_nanoseconds(&ts)) {
    if (precision > 0) {
      // High-precision timing mode
      double msec = ts.tv_nsec / 1.0e9;
      if (precision > 9)
        precision = 9;

      // Format time + fractional seconds directly
      if (portable_gmtime_r(&ts.tv_sec, &tm_info)) {
        size_t len = strftime(time_buf, 32, "%Y-%m-%d %H:%M:%S", &tm_info);
        // Compute fractional part safely
        char subsec_buf[16];
        snprintf(subsec_buf, sizeof(subsec_buf), "%.*f", precision, msec);
        size_t len2 = strlen(subsec_buf);
        // Skip the leading "0" and copy the rest (".xxx")
        strncpy(time_buf + len, subsec_buf + 1,
                zmax(len2 - 1, sizeof(time_buf) - len));
      }
    } else {
      // Standard rounding
      if (ts.tv_nsec >= 500000000)
        ts.tv_sec++;
      if (portable_gmtime_r(&ts.tv_sec, &tm_info)) {
        strftime(time_buf, sizeof(time_buf), "%Y-%m-%d %H:%M:%S", &tm_info);
      }
    }
  }

  // 3. Thread-safe Output
  lockfile(stderr);

  // Print prefix
  fprintf(stderr, "[Graphviz] %s:%d: %s ", short_name, line, time_buf);

  // Print user message
  va_list args;
  va_start(args, fmt);
  vfprintf(stderr, fmt, args);
  va_end(args);

  // Print newline
  fputc('\n', stderr);

  unlockfile(stderr);
}
