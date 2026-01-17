#pragma once

#include <util/api.h>
#include <stdbool.h>
#include <stdatomic.h>
#include <stdint.h>

#if defined(__SIZEOF_POINTER__) && __SIZEOF_POINTER__ == 4 // e.g GCC on x86
typedef uint64_t dword_t;
#elif defined(__SIZEOF_POINTER__) && __SIZEOF_POINTER__ == 8 // e.g. GCC on x86-64
typedef unsigned __int128 dword_t;
#elif defined(_WIN64) // MSVC on 64-bit ARM, x64, ARM64EC
typedef struct { uint64_t lo, hi; } dword_t;
#elif defined(_WIN32) // MSVC on 32-bit ARM, x86
typedef uint64_t dword_t;
#else
#error "unknown platform pointer size"
#endif

UTIL_API dword_t dword_new(void);

UTIL_API dword_t dword_atomic_load(_Atomic dword_t *src);

UTIL_API void dword_atomic_store(_Atomic dword_t *dst, dword_t src);

UTIL_API bool dword_atomic_cas(_Atomic dword_t *dst, dword_t *expected, dword_t desired);
