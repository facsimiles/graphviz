#include <util/dword.h>
#include <string.h>
#include <stdatomic.h>

/// Callers using `dword_t` values are typically implementing lock-free
/// algorithms. …
#if defined(__GNUC__) && (defined(__x86_64__) || defined(__i386__) || (defined(__aarch64__) && !defined(__clang__)))
#define FORCE_CAS 1
#else
#define FORCE_CAS 0
#endif

dword_t dword_new(void) {
  dword_t d;
  memset(&d, 0, sizeof(d));
  return d;
}

dword_t dword_atomic_load(_Atomic dword_t *src) {
#if FORCE_CAS
  return __sync_val_compare_and_swap(src, 1, 1);
#else
  return atomic_load_explicit(src, memory_order_acquire);
#endif
}

void dword_atomic_store(_Atomic dword_t *dst, dword_t src) {
#if FORCE_CAS
  {
    dword_t expected;
    dword_t old = dword_new();
    do {
      expected = old;
      old = __sync_val_compare_and_swap(dst, expected, src);
    } while (expected != old);
  }
#else
  atomic_store_explicit(dst, src, memory_order_release);
#endif
}

bool dword_atomic_cas(_Atomic dword_t *dst, dword_t *expected, dword_t desired) {
#if FORCE_CAS
  return __sync_bool_compare_and_swap(dst, expected, desired);
#else
  return atomic_compare_exchange_strong_explicit(dst, expected, desired, memory_order_acq_rel, memory_order_acquire);
#endif
}
