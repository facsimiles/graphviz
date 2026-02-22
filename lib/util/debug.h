/// @file
/// @brief helpers for verbose/debug printing
#ifndef _UTIL_DEBUG_H
#define _UTIL_DEBUG_H

#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <util/lockfile.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifdef GVDLL
#ifdef UTIL_EXPORTS
#define GLOBALS_API __declspec(dllexport)
#else
#define GLOBALS_API __declspec(dllimport)
#endif
#else               // !GVDLL
#define GLOBALS_API /* nothing */
#endif

#ifndef EXTERN
#define EXTERN extern
#endif

// Helper function to handle formatting and locking of writes to stderr.
// This prevents code bloat and fixes double-evaluation issues.
GLOBALS_API EXTERN void gv_log_impl(const char *file, int line, const char *fmt,
                                    ...);

// Forward declare global (so this header works without externs elsewhere)
// TODO(#2558): May be better if we can avoid using globals here.
// We can change these to better mechanism when available.

// If non-zero, log `GV_INFO()` messages, and if > 1, log `GV_DEBUG()` messages.
GLOBALS_API EXTERN unsigned char Verbose;

/// print an informational message
///
/// Logs when `Verbose` is non-zero.
/// These are typically shown with command-line flag "-v" or "-v1".
#define GV_INFO(...)                                                           \
  do {                                                                         \
    if (Verbose)                                                               \
      gv_log_impl(__FILE__, __LINE__, __VA_ARGS__);                            \
  } while (0)

/// print a debug message
///
/// This is intended for messages for Graphgiz developers, while `GV_INFO`
/// is intended for messages for Graphvi users.  `GV_DEBUG` is enabled if
/// Verbose >= 2, achieved with a command-line flag like "-v2" or higher.
#define GV_DEBUG(...)                                                          \
  do {                                                                         \
    if (Verbose > 1)                                                           \
      gv_log_impl(__FILE__, __LINE__, __VA_ARGS__);                            \
  } while (0)

#ifdef __cplusplus
}
#endif

#endif // _UTIL_DEBUG_H
