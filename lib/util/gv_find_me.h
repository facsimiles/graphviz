/// @file
/// @brief platform abstraction for finding the path to yourself

#pragma once

/// hide the symbols this header declares by default
///
/// The expectation is that, while other libraries may not want to call
/// `gv_find_me`, they may end up linking against it in order to use other
/// libutil functionality. They almost certainly do not want to re-export
/// `gv_find_me`.
///
/// This annotation is only correct while the containing library is built
/// statically. If it were built as a shared library, `gv_find_me` would need to
/// have `default` visibility (and thus be unavoidably re-exported) in order to
/// be callable.
#ifndef UTIL_API
#if !defined(__CYGWIN__) && defined(__GNUC__) && !defined(__MINGW32__)
#define UTIL_API __attribute__((visibility("hidden")))
#else
#define UTIL_API /* nothing */
#endif
#endif

#ifdef __cplusplus
extern "C" {
#endif

/// find an absolute path to the current executable
///
/// The caller is responsible for freeing the returned pointer.
///
/// It is assumed the containing executable is an on-disk file. If it is an
/// in-memory executable with no actual path, results are undefined.
///
/// @return An absolute path to the containing executable on success or `NULL`
///   on failure
UTIL_API char *gv_find_me(void);

#ifdef __cplusplus
}
#endif
