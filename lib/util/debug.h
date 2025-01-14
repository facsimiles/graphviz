/// @file
/// @brief helpers for verbose/debug printing

#pragma once

#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <util/lockfile.h>

/// print an informational message
///
/// This assumes the `Verbose` global is in scope.
#define GV_INFO(...)                                                           \
  do {                                                                         \
    if (Verbose) {                                                             \
      const char *const name_ = strrchr(__FILE__, '/') == NULL                 \
                                    ? __FILE__                                 \
                                    : strrchr(__FILE__, '/') + 1;              \
      lockfile(stderr);                                                        \
      const time_t now_ = time(NULL);                                          \
      const struct tm *const now_tm_ = localtime(&now_);                       \
      fprintf(stderr, "[Graphviz] %s:%d: %04d-%02d-%02d %02d:%02d: ", name_,   \
              __LINE__, now_tm_->tm_year + 1900, now_tm_->tm_mon + 1,          \
              now_tm_->tm_mday, now_tm_->tm_hour, now_tm_->tm_sec);            \
      fprintf(stderr, __VA_ARGS__);                                            \
      fprintf(stderr, "\n");                                                   \
      unlockfile(stderr);                                                      \
    }                                                                          \
  } while (0)
