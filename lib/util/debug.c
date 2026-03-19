/// @file
/// @brief helpers for verbose/debug printing

#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <util/debug.h>
#include <util/lockfile.h>

unsigned char Verbose;

// Helper function to handle formatting and locking.
// This prevents code bloat and fixes double-evaluation issues.
void gv_log_impl(const char *file, int line, const char *fmt, ...) {
  // 1. Get filename part
  const char *short_name = strrchr(file, '/');
  short_name = short_name ? short_name + 1 : file;

  // 2. Thread-safe Output
  lockfile(stderr);
  const time_t now_ = time(NULL);
  const struct tm *const now_tm_ = localtime(&now_);

  // Print prefix
  fprintf(stderr, "[Graphviz] %s:%d: %04d-%02d-%02d %02d:%02d:%02d ",
          short_name, line, now_tm_->tm_year + 1900, now_tm_->tm_mon + 1,
          now_tm_->tm_mday, now_tm_->tm_hour, now_tm_->tm_min, now_tm_->tm_sec);

  // Print user message
  va_list args;
  va_start(args, fmt);
  vfprintf(stderr, fmt, args);
  va_end(args);

  // Print newline
  fputc('\n', stderr);

  unlockfile(stderr);
}
