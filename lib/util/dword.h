#pragma once

#include <stdatomic.h>
#include <stdbool.h>
#include <stdint.h>
#include <util/api.h>

#if defined(__SIZEOF_POINTER__) && __SIZEOF_POINTER__ == 4 // e.g GCC on x86
typedef uint64_t dword_t;
#elif defined(__SIZEOF_POINTER__) &&                                           \
    __SIZEOF_POINTER__ == 8 // e.g. GCC on x86-64
typedef unsigned __int128 dword_t;
#elif defined(_WIN64)       // MSVC on 64-bit ARM, x64, ARM64EC
typedef struct __declspec(align(16)) {
  int64_t word[2];
} dword_t;
#elif defined(_WIN32)       // MSVC on 32-bit ARM, x86
typedef __declspec(align(8)) int64_t dword_t;
#else
#error "unknown platform pointer size"
#endif

#ifdef _MSC_VER
#ifdef _ISO_VOLATILE
#error "dword atomics require /volatile:ms"
#endif
typedef volatile dword_t atomic_dword_t;
#else
typedef _Atomic dword_t atomic_dword_t;
#endif

UTIL_API dword_t gv_dword_new(void);

UTIL_API dword_t gv_dword_atomic_load(atomic_dword_t *src);

UTIL_API void gv_dword_atomic_store(atomic_dword_t *dst, dword_t src);

UTIL_API bool gv_dword_atomic_cas(atomic_dword_t *dst, dword_t *expected,
                                  dword_t desired);

UTIL_API dword_t gv_dword_atomic_xchg(atomic_dword_t *dst, dword_t src);
