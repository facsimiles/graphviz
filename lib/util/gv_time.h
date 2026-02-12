/// @file
/// @brief platform abstraction to get precise times

#pragma once

#include <stdint.h>
#include <time.h>

// Hopefully portable way to get precise timing.
// returns non-0 if successful.
static inline int get_nanoseconds(struct timespec *ts) {
#if defined(__APPLE__) && defined(__MACH__)
#if __MAC_OS_X_VERSION_MIN_REQUIRED >= 101500
  return timespec_get(ts, TIME_UTC);
#else
  return clock_gettime(CLOCK_REALTIME, ts) == 0 ? TIME_UTC : 0;
#endif
#elif defined(_MSC_VER) ||                                                     \
    defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
  return timespec_get(ts, TIME_UTC);
#else
  return clock_gettime(CLOCK_REALTIME, ts) == 0 ? 1 : 0;
#endif
}

static inline struct tm *portable_localtime_r(const time_t *timer,
                                              struct tm *result) {
#if defined(_WIN32) || defined(_WIN64)
  // Windows: localtime_s returns 0 on success, parameters are (result, timer)
  if (localtime_s(result, timer) == 0)
    return result;
  return NULL;
#else
  // POSIX (Linux/macOS): returns pointer to result on success, parameters are
  // (timer, result)
  return localtime_r(timer, result);
#endif
}

static inline struct tm *portable_gmtime_r(const time_t *timer,
                                           struct tm *result) {
#if defined(_WIN32) || defined(_WIN64)
  // Windows: localtime_s returns 0 on success, parameters are (result, timer)
  if (gmtime_s(result, timer) == 0)
    return result;
  return NULL;
#else
  // POSIX (Linux/macOS): returns pointer to result on success, parameters are
  // (timer, result)
  return gmtime_r(timer, result);
#endif
}
