/// @file
/// @brief Base64 encoding
#pragma once

/// hide the symbols this header declares by default
///
/// The expectation is that users of this header (applications, shared
/// libraries, or static libraries) want to call `gv_base64` but not re-export
/// it to their users. This annotation is only correct while the containing
/// library is built statically. If it were built as a shared library,
/// `gv_base64` would need to have `default` visibility (and thus be unavoidably
/// re-exported) in order to be callable.
#ifndef UTIL_API
#if !defined(__CYGWIN__) && defined(__GNUC__) && !defined(__MINGW32__)
#define UTIL_API __attribute__((visibility("hidden")))
#else
#define UTIL_API /* nothing */
#endif
#endif

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/// how many bytes does it take to encode a given source data length?
///
/// @param source_size The number of bytes in the source to encoding
/// @return The number of bytes required in the destination of encoding
UTIL_API size_t gv_base64_size(size_t source_size);

/// Base64 encode some data
///
/// This function does not return on failure, like memory allocation. It calls
/// `exit`. The caller is expected to `free` the returned pointer.
///
/// @param source Pointer to the start of data to encode
/// @param size Number of bytes in the source
/// @return A buffer of the encoded data
UTIL_API char *gv_base64(const unsigned char *source, size_t size);

#ifdef __cplusplus
}
#endif
