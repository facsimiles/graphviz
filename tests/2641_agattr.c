/// @file
/// @brief Accompanying test code for test_2641[agattr]

#include <assert.h>
#include <graphviz/cgraph.h>
#include <stddef.h>

int main(void) {

  // create a new, empty graph
  Agraph_t *const g = agopen("foo", Agdirected, NULL);
  assert(g != NULL);

  // 1. a regular text attribute should be created as regular text
  {
    Agsym_t *const attr = agattr_text(g, AGRAPH, "bar", "bar value");
    assert(attr != NULL);
    const char *const value = agxget(g, attr);
    assert(value != NULL);
    assert(!aghtmlstr(value) &&
           "regular text attribute was created as HTML-like");
  }

  // 2. an HTML-like text attribute should be created as HTML-like
  {
    Agsym_t *const attr = agattr_html(g, AGRAPH, "baz", "baz value");
    assert(attr != NULL);
    const char *const value = agxget(g, attr);
    assert(value != NULL);
    assert(aghtmlstr(value) &&
           "HTML-like attribute was created as regular text");
  }

  // 3. creating an attribute through the ambiguous `agattr` API should default
  // to regular text
  {
    Agsym_t *const attr = agattr(g, AGRAPH, "qux", "qux value");
    assert(attr != NULL);
    const char *const value = agxget(g, attr);
    assert(value != NULL);
    assert(!aghtmlstr(value) && "agattr created a new attribute as HTML-like");
  }

  // 4. creating an attribute through `agattr` using something we `agstrdup`ed
  // should create a regular text attribute
  {
    char *const quux = agstrdup(g, "quux value");
    Agsym_t *const attr = agattr(g, AGRAPH, "quux", quux);
    assert(attr != NULL);
    const char *const value = agxget(g, attr);
    assert(value != NULL);
    assert(!aghtmlstr(value) && "agattr did not detect agstrdup-ed value");
  }

  // 5. creating an attribute through `agattr` using something we
  // `agstrdup_html`ed should create an HTML-like attribute
  {
    char *const corge = agstrdup_html(g, "corge value");
    Agsym_t *const attr = agattr(g, AGRAPH, "corge", corge);
    assert(attr != NULL);
    const char *const value = agxget(g, attr);
    assert(value != NULL);
    assert(aghtmlstr(value) && "agattr did not detect agstrdup_html-ed value");
  }

  // 6. creating an attribute through `agattr` using something we `agstrdup`ed
  // should create a regular text attribute, even when it aliases a previously
  // created regular text value
  {
    char *const bar = agstrdup(g, "bar value");
    Agsym_t *const attr = agattr(g, AGRAPH, "grault", bar);
    assert(attr != NULL);
    const char *const value = agxget(g, attr);
    assert(value != NULL);
    assert(!aghtmlstr(value) && "agattr did not detect agstrdup-ed value");
  }

  // 7. creating an attribute through `agattr` using something we `agstrdup`ed
  // should create a regular text attribute, even when it aliases a previously
  // created HTML-like value
  {
    char *const baz = agstrdup(g, "baz value");
    Agsym_t *const attr = agattr(g, AGRAPH, "garply", baz);
    assert(attr != NULL);
    const char *const value = agxget(g, attr);
    assert(value != NULL);
    assert(!aghtmlstr(value) && "agattr did not detect agstrdup-ed value");
  }

  // 8. creating an attribute through `agattr` using something we
  // `agstrdup_html`ed should create an HTML-like attribute, even when it
  // aliases a previously created regular text value
  {
    char *const bar = agstrdup_html(g, "bar value");
    Agsym_t *const attr = agattr(g, AGRAPH, "waldo", bar);
    assert(attr != NULL);
    const char *const value = agxget(g, attr);
    assert(value != NULL);
    assert(aghtmlstr(value) && "agattr did not detect agstrdup_html-ed value");
  }

  // 9. creating an attribute through `agattr` using something we
  // `agstrdup_html`ed should create an HTML-like attribute, even when it
  // aliases a previously created HTML-like value
  {
    char *const baz = agstrdup_html(g, "baz value");
    Agsym_t *const attr = agattr(g, AGRAPH, "fred", baz);
    assert(attr != NULL);
    const char *const value = agxget(g, attr);
    assert(value != NULL);
    assert(aghtmlstr(value) && "agattr did not detect agstrdup_html-ed value");
  }

  (void)agclose(g);

  return 0;
}
