#include <common/types.h>
#include <common/utils.h>
#include <ctype.h>
#include <stdbool.h>

// variant of `isalpha` that assumes a C locale
static bool isalpha_no_locale(char c) {
  if (c >= 'a' && c <= 'z')
    return true;
  if (c >= 'A' && c <= 'Z')
    return true;
  return false;
}

/* return true if *s points to &[A-Za-z]+;      (e.g. &Ccedil; )
 *                          or &#[0-9]*;        (e.g. &#38; )
 *                          or &#x[0-9a-fA-F]*; (e.g. &#x6C34; )
 */
static bool xml_isentity(const char *s)
{
    s++;			/* already known to be '&' */
    if (*s == ';') { // '&;' is not a valid entity
	return false;
    }
    if (*s == '#') {
	s++;
	if (*s == 'x' || *s == 'X') {
	    s++;
	    while (isxdigit(*s))
		s++;
	} else {
	    while (isdigit(*s))
		s++;
	}
    } else {
	while (isalpha_no_locale(*s))
	    s++;
    }
    if (*s == ';')
	return true;
    return false;
}

/** XML-escape a character
 *
 * \param previous The source character preceding the current one or '\0' if
 *   there was no prior character.
 * \param current Pointer to the current position in a source string being
 *   escaped.
 * \param flags Options for configuring behavior.
 * \param cb User function for emitting escaped data. This is expected to take a
 *   caller-defined state type as the first parameter and the string to emit as
 *   the second, and then return an opaque value that is passed back to the
 *   caller.
 * \param state Data to pass as the first parameter when calling `cb`.
 * \return The return value of a call to `cb`.
 */
static int xml_core(char previous, const char *current, xml_flags_t flags,
                    int (*cb)(void *state, const char *s), void *state) {

  char c = *current;

  // escape '&' only if not part of a legal entity sequence
  if (c == '&' && (flags.raw || !xml_isentity(current)))
    return cb(state, "&amp;");

  // '<' '>' are safe to substitute even if string is already UTF-8 coded since
  // UTF-8 strings won't contain '<' or '>'
  if (c == '<')
    return cb(state, "&lt;");

  if (c == '>')
    return cb(state, "&gt;");

  // '-' cannot be used in XML comment strings
  if (c == '-' && flags.dash)
    return cb(state, "&#45;");

  if (c == ' ' && previous == ' ' && flags.nbsp)
    // substitute 2nd and subsequent spaces with required_spaces
    return cb(state, "&#160;"); // Inkscape does not recognize &nbsp;

  if (c == '"')
    return cb(state, "&quot;");

  if (c == '\'')
    return cb(state, "&#39;");

  if (c == '\n' && flags.raw)
    return cb(state, "&#10;");

  if (c == '\r' && flags.raw)
    return cb(state, "&#13;");

  // otherwise, output the character as-is
  char buffer[2] = {c, '\0'};
  return cb(state, buffer);
}

int xml_escape(const char *s, xml_flags_t flags,
               int (*cb)(void *state, const char *s), void *state) {
  char previous = '\0';
  int rc = 0;
  while (*s != '\0') {
    rc = xml_core(previous, s, flags, cb, state);
    if (rc < 0)
      return rc;
    previous = *s;
    ++s;
  }
  return rc;
}
