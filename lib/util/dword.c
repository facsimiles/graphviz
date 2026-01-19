#ifndef NO_CONFIG // defined by test_dword.c
#include "config.h"
#endif

#include <assert.h>
#include <stdatomic.h>
#include <stdbool.h>
#include <string.h>
#include <util/dword.h>

#ifdef _MSC_VER
#include <intrin.h>
#endif

#ifndef FORCE_CAS
/// Callers using `dword_t` values are typically implementing lock-free
/// algorithms. …
#if defined(__GNUC__) && !defined(__clang__) // GCC
#if defined(__aarch64__) || defined(__i386__) || defined(__x86_64__)
#define FORCE_CAS 1
#endif
#endif
#endif
#ifndef FORCE_CAS
#define FORCE_CAS 0
#endif

dword_t gv_dword_new(void) {
  dword_t d;
  memset(&d, 0, sizeof(d));
  return d;
}

dword_t gv_dword_atomic_load(atomic_dword_t *src) {
  assert(src != NULL);
#if FORCE_CAS
  return __sync_val_compare_and_swap(src, 1, 1);
#elif defined(_M_ARM64)
  {
    dword_t result = gv_dword_new();
    (void)_InterlockedCompareExchange128_acq(src, 0, 0, &result);
    return result;
  }
#elif defined(_WIN64)
  {
    dword_t result = gv_dword_new();
    (void)_InterlockedCompareExchange128(src, 0, 0, &result);
    return result;
  }
#elif defined(_M_ARM)
  return _InterlockedCompareExchange64_acq(src, 0, 0);
#elif defined(_WIN32)
  return _InterlockedCompareExchange64(src, 0, 0);
#else
  return atomic_load_explicit(src, memory_order_acquire);
#endif
}

void gv_dword_atomic_store(atomic_dword_t *dst, dword_t src) {
  assert(dst != NULL);
#if FORCE_CAS
  for (dword_t old = gv_dword_new();;) {
    const dword_t expected = old;
    old = __sync_val_compare_and_swap(dst, expected, src);
    if (old == expected) {
      break;
    }
  }
#elif defined(_WIN64)
  for (dword_t expected = gv_dword_new();;) {
    if (_InterlockedCompareExchange128(dst, src.word[1], src.word[0],
                                       &expected)) {
      break;
    }
  }
#elif defined(_M_ARM)
  (void)_InterlockedExchange64_rel(dst, src);
#elif defined(_WIN32)
  for (dword_t old = gv_dword_new();;) {
    const dword_t expected = old;
    old = _InterlockedCompareExchange64(dst, src, expected);
    if (old == expected) {
      break;
    }
  }
#else
  atomic_store_explicit(dst, src, memory_order_release);
#endif
}

bool gv_dword_atomic_cas(atomic_dword_t *dst, dword_t *expected,
                         dword_t desired) {
  assert(dst != NULL);
  assert(expected != NULL);
#if FORCE_CAS
  {
    const dword_t expectation = *expected;
    *expected = __sync_val_compare_and_swap(dst, expectation, desired);
    return *expected == expectation;
  }
#elif defined(_WIN64)
  return _InterlockedCompareExchange128(dst, desired.word[1], desired.word[0],
                                        expected);
#elif defined(_WIN32)
  {
    const dword_t expectation = *expected;
    *expected = _InterlockedCompareExchange64(dst, desired, expectation);
    return *expected == expectation;
  }
#else
  return atomic_compare_exchange_strong_explicit(
      dst, expected, desired, memory_order_acq_rel, memory_order_acquire);
#endif
}

dword_t gv_dword_atomic_xchg(atomic_dword_t *dst, dword_t src) {
  assert(dst != NULL);
#ifdef _M_ARM
  return _InterlockedExchange64(dst, src);
#elif FORCE_CAS || defined(_WIN32)
  {
    dword_t old = gv_dword_new();
    while (!gv_dword_atomic_cas(dst, &old, src)) {
    }
    return old;
  }
#else
  return atomic_exchange_explicit(dst, src, memory_order_acq_rel);
#endif
}
