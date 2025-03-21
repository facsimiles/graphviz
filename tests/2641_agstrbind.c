/// @file
/// @brief Accompanying test code for test_2641[agstrbind]

#include <assert.h>
#include <graphviz/cgraph.h>
#include <stddef.h>

int main(void) {

  // create a new, empty graph
  Agraph_t *const g = agopen("foo", Agdirected, NULL);
  assert(g != NULL);

  // 1. regular text should be findable as regular text
  {
    const char *const bar = agstrdup_text(g, "bar");
    assert(bar != NULL);
    const char *const value = agstrbind_text(g, "bar");
    assert(value != NULL);
    assert(!aghtmlstr(value) && "regular text was created as HTML-like text");
  }

  // 2. HTML-like text should be findable as HTML-like
  {
    const char *const baz = agstrdup_html(g, "baz");
    assert(baz != NULL);
    const char *const value = agstrbind_html(g, "baz");
    assert(value != NULL);
    assert(aghtmlstr(value) && "HTML-like text was created as regular text");
  }

  // 3. creating text through the ambiguous `agstrdup` API should default to
  // regular text
  {
    const char *const qux = agstrdup(g, "qux");
    assert(qux != NULL);
    const char *const value = agstrbind(g, "qux");
    assert(value != NULL);
    assert(!aghtmlstr(value) && "agstrdup created new text as HTML-like");
  }

  // 4. regular text should not be findable as HTML-like text
  {
    const char *const quux = agstrdup_text(g, "quux");
    assert(quux != NULL);
    const char *const value = agstrbind_html(g, "quux");
    assert(value == NULL);
  }

  // 5. HTML-like text should not be findable as regular text
  {
    const char *const corge = agstrdup_html(g, "corge");
    assert(corge != NULL);
    const char *const value = agstrbind_text(g, "corge");
    assert(value == NULL);
  }

  // 6. when regular text and HTML-like text alias one another, the ambiguous
  // `agstrbind` API should find the regular text version
  {
    const char *const grault_text = agstrdup_text(g, "grault");
    assert(grault_text != NULL);

    const char *const grault_html = agstrdup_html(g, "grault");
    assert(grault_html != NULL);
    assert(grault_text != grault_html);

    const char *const value_text = agstrbind(g, "grault");
    assert(value_text != NULL &&
           "regular text could not be found when it aliases HTML-like text");
    assert(!aghtmlstr(value_text) &&
           "regular text was not found as regular text");
    assert(value_text == grault_text &&
           "regular text lookup matched an unexpected dictionary entry");

    const char *const value_html = agstrbind_html(g, "grault");
    assert(value_html != NULL &&
           "HTML-like text could not be found when it aliases regular text");
    assert(aghtmlstr(value_html) &&
           "HTML-like text was not found as HTML-like text");
    assert(value_html == grault_html &&
           "HTML-like text lookup matched an unexpected dictionary entry");

    const char *const value_ambiguous = agstrbind(g, "grault");
    assert(value_ambiguous != NULL &&
           "regular text could not be found when it aliases HTML-like text");
    assert(!aghtmlstr(value_ambiguous) &&
           "regular text was not found as regular text");
    assert(value_ambiguous == grault_text &&
           "regular text lookup matched an unexpected dictionary entry");
  }

  (void)agclose(g);

  return 0;
}
