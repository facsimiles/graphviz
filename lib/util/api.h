/// @file
/// @brief macro for API hiding/exposing

#pragma once

/// use this macro to hide symbols by default
///
/// The expectation is that users of this library (applications, shared
/// libraries, or static libraries) want to call some of the exposed functions
/// but not re-export them to their users. This annotation is only correct while
/// the containing library is built statically. If it were built as a shared
/// library, API symbols would need to have `default` visibility (and thus be
/// unavoidably re-exported) in order to be callable.
#ifndef UTIL_API
#if !defined(__CYGWIN__) && defined(__GNUC__) && !defined(__MINGW32__)
#define UTIL_API __attribute__((visibility("hidden")))
#else
#define UTIL_API /* nothing */
#endif
#endif
