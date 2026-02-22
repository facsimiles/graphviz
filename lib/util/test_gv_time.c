/// @file
/// @brief unit test for gv_time.h

#undef NDEBUG
#ifdef NDEBUG
#error this is not intended to be compiled with assertions off
#endif

#include <assert.h>
#include <locale.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <util/gv_time.h>

static struct timespec delta_timespec(struct timespec ts1,
                                      struct timespec ts2) {
  long raw_delta_ns = ts2.tv_nsec - ts1.tv_nsec;
  bool need_borrow = raw_delta_ns < 0;
  int borrow = need_borrow ? 1 : 0;
  long delta_ns = raw_delta_ns + (need_borrow ? 1000000000L : 0L);
  time_t delta_sec = ts2.tv_sec - ts1.tv_sec - borrow;
  return (struct timespec){.tv_sec = delta_sec, .tv_nsec = delta_ns};
}

// Main purpose here is to validate that things run at all.
// We can't really validate nanoseconds without more control
// over than scheduling than we expect.
static void test_get_nanoseconds(void) {
  struct timespec ts1, ts2, ts3;

  // Sleep for 1.5 seconds to make sure there answer is definitive.
  struct timespec sleep_time =
      (struct timespec){.tv_sec = 1, .tv_nsec = 500000000L};

  int res = get_nanoseconds(&ts1);
  assert(res != 0);

  struct timespec rem_time;
  if (nanosleep(&sleep_time, &rem_time) == -1) {
    struct timespec rem_time2;
    while (nanosleep(&rem_time, &rem_time2) == -1) {
      rem_time = rem_time2;
    }
  };

  // START DELTA2
  res = get_nanoseconds(&ts2);
  assert(res != 0);

  // delta sec should be >= sleep_time.
  struct timespec delta1 = delta_timespec(ts1, ts2);
  assert(delta1.tv_sec >= sleep_time.tv_sec);
  assert(delta1.tv_nsec >= sleep_time.tv_sec);
  assert(0 <= ts1.tv_nsec && ts1.tv_nsec < 1000000000L);
  assert(0 <= ts2.tv_nsec && ts2.tv_nsec < 1000000000L);

  // FINISH DELTA2
  res = get_nanoseconds(&ts3);
  assert(res != 0);

  // Unless asserts fail there is very little happening
  // between between START DELTA2 and FINISH of DELTA2
  // (see comments above), so expect time < 1sec.
  struct timespec delta2 = delta_timespec(ts2, ts3);
  assert(0 <= ts3.tv_nsec && ts3.tv_nsec < 1000000000L);
  assert(delta2.tv_sec == 0);
  assert(delta2.tv_nsec > 0);
}

static void expect_equal_tms(struct tm *tm1, struct tm *tm2) {
  assert(tm1->tm_sec == tm2->tm_sec);
  assert(tm1->tm_min == tm2->tm_min);
  assert(tm1->tm_hour == tm2->tm_hour);
  assert(tm1->tm_mday == tm2->tm_mday);
  assert(tm1->tm_mon == tm2->tm_mon);
  assert(tm1->tm_year == tm2->tm_year);
  assert(tm1->tm_wday == tm2->tm_wday);
  assert(tm1->tm_yday == tm2->tm_yday);
  assert(tm1->tm_isdst == tm2->tm_isdst);
  assert(tm1->tm_gmtoff == tm2->tm_gmtoff);
  assert(tm1->tm_zone == tm2->tm_zone ||
         (0 == strncmp(tm1->tm_zone, tm2->tm_zone, 15)));
}

static void expect_offset_tms(struct tm *gtm, struct tm *ltm, long offset) {
  long deltasec = ltm->tm_sec - gtm->tm_sec;
  deltasec += (ltm->tm_min - gtm->tm_min) * 60L;
  deltasec += (ltm->tm_hour - gtm->tm_hour) * (60L * 60);
  deltasec += (ltm->tm_yday - gtm->tm_yday) * (60L * 60 * 24);
  if (ltm->tm_year == gtm->tm_year) {
    assert(deltasec == offset);
  } else {
    // It's too close to NYE, skip it for now.
  }
}

#ifdef DEBUG_TEST
static void print_tm(struct tm *tmptr) {
  fprintf(stderr,
          "tm_sec = %d, tm_min = %d, tm_hour = %d, tm_mday = %d, "
          "tm_mon = %d, tm_year = %d, tm_wday = %d, tm_yday = %d, "
          "tm_isdst = %d, tm_gmtoff = %ld, tm_zone = \"%s\"\n",
          tmptr->tm_sec, tmptr->tm_min, tmptr->tm_hour, tmptr->tm_mday,
          tmptr->tm_mon, tmptr->tm_year, tmptr->tm_wday, tmptr->tm_yday,
          tmptr->tm_isdst, tmptr->tm_gmtoff, tmptr->tm_zone);
}
#endif // DEBUG_TEST

static void test_portable_gm_and_localtime_r(void) {
  const time_t tsec1 = 12345678910L;
  const time_t tsec2 = 987654321098L;

  char *locale = setlocale(LC_TIME, "POSIX");
  assert(locale != NULL);

  struct tm ltm1, ltm2, gtm1, gtm2;
  struct tm *res;

  res = portable_localtime_r(&tsec1, &ltm1);
  assert(res == &ltm1);
  res = portable_localtime_r(&tsec2, &ltm2);
  assert(res == &ltm2);

  res = portable_gmtime_r(&tsec1, &gtm1);
  assert(res == &gtm1);
  res = portable_gmtime_r(&tsec2, &gtm2);
  assert(res == &gtm2);

  long tzoffset = ltm1.tm_gmtoff;
  assert(ltm1.tm_gmtoff == ltm2.tm_gmtoff);

#ifdef DEBUG_TEST
  fprintf(stderr, "ltm1:\n");
  print_tm(&ltm1);

  fprintf(stderr, "ltm2:\n");
  print_tm(&ltm2);

  fprintf(stderr, "gtm1:\n");
  print_tm(&gtm1);

  fprintf(stderr, "gtm2:\n");
  print_tm(&gtm2);
#endif // DEBUG_TEST

  // GTM1 and GTM2 are easy to check.
  struct tm exp_gtm1 = (struct tm){.tm_sec = 10,
                                   .tm_min = 15,
                                   .tm_hour = 19,
                                   .tm_mday = 21,
                                   .tm_mon = 2,
                                   .tm_year = 461,
                                   .tm_wday = 2,
                                   .tm_yday = 79,
                                   .tm_isdst = 0,
                                   .tm_gmtoff = 0,
                                   .tm_zone = "GMT"};
  struct tm exp_gtm2 = (struct tm){.tm_sec = 38,
                                   .tm_min = 31,
                                   .tm_hour = 6,
                                   .tm_mday = 9,
                                   .tm_mon = 6,
                                   .tm_year = 31367,
                                   .tm_wday = 6,
                                   .tm_yday = 189,
                                   .tm_isdst = 0,
                                   .tm_gmtoff = 0,
                                   .tm_zone = "GMT"};
  expect_equal_tms(&gtm1, &exp_gtm1);
  expect_equal_tms(&gtm2, &exp_gtm2);

  // LTM is harder, since we don't know what it is and
  // I can't find a portable way to set locale.  Instead,
  // just see if the specified offset matches the field offset.
  expect_offset_tms(&exp_gtm1, &ltm1, ltm1.tm_gmtoff);
  expect_offset_tms(&exp_gtm2, &ltm1, ltm2.tm_gmtoff);

  // Double-check local time offset by adding the offset to the input and then
  // converting using GMT.
  const time_t ltsec1 = tsec1 + tzoffset;
  const time_t ltsec2 = tsec2 + tzoffset;
  struct tm lgtm1, lgtm2;

  res = portable_gmtime_r(&ltsec1, &lgtm1);
  assert(res == &lgtm1);
  res = portable_gmtime_r(&ltsec2, &lgtm2);
  assert(res == &lgtm2);

#ifdef DEBUG_TEST
  fprintf(stderr, "lgtm1:\n");
  print_tm(&lgtm1);

  fprintf(stderr, "lgtm2:\n");
  print_tm(&lgtm2);
#endif // DEBUG_TEST

  // They should be the same, except that timezone and gmtoff is changed.
  // Just copy over those fields so they match.
  ltm1.tm_gmtoff = 0;
  ltm1.tm_zone = lgtm1.tm_zone;
  ltm1.tm_isdst = lgtm1.tm_isdst;
  expect_equal_tms(&ltm1, &lgtm1);
  ltm2.tm_gmtoff = 0;
  ltm2.tm_zone = lgtm2.tm_zone;
  ltm2.tm_isdst = lgtm2.tm_isdst;
  expect_equal_tms(&ltm2, &lgtm2);
}

int main(void) {
#define RUN(t)                                                                 \
  do {                                                                         \
    fprintf(stderr, "running test_%s... ", #t);                                \
    fflush(stdout);                                                            \
    test_##t();                                                                \
    fprintf(stderr, "OK\n");                                                   \
  } while (0)

  RUN(get_nanoseconds);
  RUN(portable_gm_and_localtime_r);
}
