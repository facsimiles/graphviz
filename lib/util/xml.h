/// @file
/// @brief XML escaping functionality

#pragma once

/// hide the symbols this header declares by default
///
/// The expectation is that, while other libraries may not want to call
/// `gv_xml_escape`, they may end up linking against it in order to use other
/// libutil functionality. They almost certainly do not want to re-export
/// `gv_xml_escape`.
///
/// This annotation is only correct while the containing library is built
/// statically. If it were built as a shared library, `gv_xml_escape` would need
/// to have `default` visibility (and thus be unavoidably re-exported) in order
/// to be callable.
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

/// options to tweak the behavior of XML escaping
typedef struct {
  /// assume no embedded escapes, and escape "\n" and "\r"
  unsigned raw : 1;
  /// escape '-'
  unsigned dash : 1;
  /// escape consecutive ' '
  unsigned nbsp : 1;
  /// anticipate non-ASCII characters that need to be encoded
  unsigned utf8 : 1;
} xml_flags_t;

/// XML-escape a string
///
/// @param s Source string to process.
/// @param flags Options of how to configure escaping.
/// @param cb An `fputs` analogue for emitting escaped output.
/// @param state Caller-defined data to pass to `cb`.
/// @return The first negative value `cb` returns or the last return value of
///   `cb`.
UTIL_API int gv_xml_escape(const char *s, xml_flags_t flags,
                           int (*cb)(void *state, const char *s), void *state);

#ifdef __cplusplus
}
#endif
