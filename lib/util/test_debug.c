#include <assert.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

// Include dependencies of the file under test first, so they will not be
// included again when we include the file under test and we can safely
// mock the functionality we use.

#include <time.h>
#include <util/lockfile.h>

// Define the mock replacements BEFORE including the header under test
#define lockfile lockfile_mock
#define unlockfile unlockfile_mock
#define fprintf fprintf_mock
#define vfprintf vfprintf_mock
#define time time_mock
#define localtime localtime_mock

// * Mocks

// Global buffer to capture mock output
static char mock_buffer[2048];
static int mock_calls = 0;

// Mock function signature must match fprintf
int fprintf_mock(FILE *stream, const char *restrict format, ...);
int vfprintf_mock(FILE *stream, const char *restrict format, va_list args);

// * Dependency mocking
// We mock these to ensure the test is self-contained and deterministic.

// Add lock/unlock to the log to make it easy to check
static void lockfile(FILE *fd) {
  (void)fd;
  fprintf_mock(NULL, "DOLOCK");
}
static void unlockfile(FILE *fd) {
  (void)fd;
  fprintf_mock(NULL, "UNLOCK");
}

static time_t counter = 0;
static time_t time_mock_result;

static time_t time_mock(time_t *timer) {
  if (timer) {
    time_mock_result = *timer;
  }
  return time_mock_result + ++counter;
}

// Mock gmtime helper
static const struct tm *localtime_mock(const time_t *now) {
  // Make sure we got the value returned by time_mock.
  assert(*now == time_mock_result + counter);
  // return an arbitrary value
  static struct tm result;
  memset(&result, 0, sizeof(result));
  result.tm_year = 123; // 2023
  result.tm_mon = 0;    // Jan
  result.tm_mday = 1;
  result.tm_hour = 12;
  result.tm_min = 0;
  result.tm_sec = 0;
  return &result;
}

// Now include the header and source file under test
#include <util/debug.c>
#include <util/debug.h>

// Undefine overloads so we can print for debugging/errors below
#undef fprintf
#undef vfprintf

// Globals required by util/debug.h (normally defined in the library)

int vfprintf_mock(FILE *stream, const char *restrict format, va_list args) {
  (void)stream; // We ignore the stream; we always write to buffer
  mock_calls++;

  // Append to buffer so we capture the full multi-call message
  size_t current_len = strlen(mock_buffer);
  size_t remaining = sizeof(mock_buffer) - current_len;

  if (remaining <= 1)
    return 0; // Buffer full

  int written = vsnprintf(mock_buffer + current_len, remaining, format, args);
  return written;
}

// Mock implementation of fprintf
int fprintf_mock(FILE *stream, const char *restrict format, ...) {
  va_list args;
  va_start(args, format);
  int written = vfprintf_mock(stream, format, args);
  va_end(args);
  return written;
}

static void reset_mock(void) {
  // zero out buffer to make sure anything written
  // is zero-terminated eventually.
  memset(mock_buffer, 0, sizeof(mock_buffer));
  mock_calls = 0;
  Verbose = 0;
}

// Convenient check that shows mock output if results vary.  `strnstr` isn't
// completely portable, so we have to use `strstr` here.  But as long as we've
// called `reset_mock` and use only `vfprintf_mock` to modify it, `mock_buffer`
// will definitely be nul-terminated.
static int output_contains(const char *needle) {
  if (!strstr(mock_buffer, needle)) {
    fprintf(stderr, "String \"%s\", not found in buffer \"%s\"\n", needle,
            mock_buffer);
    return 0;
  } else {
    return 1;
  }
}

// Test cases

static void test_gv_info_disabled(void) {
  reset_mock();
  Verbose = 0;

  GV_INFO("This should not appear");

  assert(mock_buffer[0] == '\0');
  assert(mock_calls == 0);
}

static void test_gv_info_enabled(void) {
  reset_mock();
  Verbose = 1;

  GV_INFO("Hello %s", "World");

  // Expect 3 calls: prefix/timestamp, message, newline
  assert(mock_calls >= 3);

  assert(output_contains("[Graphviz]"));
  assert(output_contains("test_debug.c"));
  assert(output_contains("Hello World"));
  // DOLOCK is at the start
  if (strstr(mock_buffer, "DOLOCK") != mock_buffer) {
    fprintf(stderr, "Expected initial call to `lockfile`");
    assert(false && "Expected initial call to `lockfile`");
  }
  // UNLOCK Is at the end
  if (strstr(mock_buffer, "UNLOCK") + strlen("UNLOCK") - strlen(mock_buffer) !=
      mock_buffer) {
    fprintf(stderr, "Expected final call to `unlockfile`");
    assert(false && "Expected final call to `unlockfile`");
  }
}

static void test_gv_debug_levels(void) {
  // Case A: Verbose = 1. GV_DEBUG should be silent (requires > 1)
  reset_mock();
  Verbose = 1;
  GV_DEBUG("Hidden Debug Message");
  assert(mock_buffer[0] == '\0');

  // Case B: Verbose = 2. GV_DEBUG should print.
  reset_mock();
  Verbose = 2;
  GV_DEBUG("Visible Debug Message");
  assert(output_contains("Visible Debug Message"));
}

static void test_timestamp_formatting(void) {
  reset_mock();
  Verbose = 1;
  GV_INFO("Checking timestamp");

  // Based on our mock time values
  assert(output_contains("2023-01-01 12:00:00 "));
}

int main(void) {
#define RUN(t)                                                                 \
  do {                                                                         \
    fprintf(stderr, "running test_%s... ", #t);                                \
    fflush(stdout);                                                            \
    test_##t();                                                                \
    fprintf(stderr, "OK\n");                                                   \
  } while (0)

  RUN(gv_info_disabled);
  RUN(gv_info_enabled);
  RUN(gv_debug_levels);
  RUN(timestamp_formatting);
}
