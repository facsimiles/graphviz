/// @file
/// @brief Accompanying test code for test_2641[agsafeset]

#include <assert.h>
#include <graphviz/cgraph.h>
#include <stddef.h>

int main(void) {

  // create a new, empty graph
  Agraph_t *const g = agopen("foo", Agdirected, NULL);
  assert(g != NULL);

  // 1. a regular text attribute should be created as regular text
  {
    const int rc = agsafeset_text(g, "bar", "bar value", "bar default");
    assert(rc == 0);
    const char *const value = agget(g, "bar");
    assert(value != NULL);
    assert(!aghtmlstr(value) &&
           "regular text attribute was created as HTML-like");
  }

  // 2. an HTML-like text attribute should be created as HTML-like
  {
    const int rc = agsafeset_html(g, "baz", "baz value", "baz default");
    assert(rc == 0);
    const char *const value = agget(g, "baz");
    assert(value != NULL);
    assert(aghtmlstr(value) &&
           "HTML-like attribute was created as regular text");
  }

  // 3. creating an attribute through the ambiguous `agsafeset` API should
  // default to regular text
  {
    const int rc = agsafeset(g, "qux", "qux value", "qux default");
    assert(rc == 0);
    const char *const value = agget(g, "qux");
    assert(value != NULL);
    assert(!aghtmlstr(value) &&
           "agsafeset created a new attribute as HTML-like");
  }

  // 4. creating an attribute through `agsafeset` using something we
  // `agstrdup`ed should create a regular text attribute
  {
    const char *const quux = agstrdup(g, "quux value");
    assert(quux != NULL);
    const int rc = agsafeset(g, "quux", quux, "quux default");
    assert(rc == 0);
    const char *const value = agget(g, "quux");
    assert(value != NULL);
    assert(!aghtmlstr(value) && "agsafeset did not detect agstrdup-ed value");
  }

  // 5. creating an attribute through `agsafeset` using something we
  // `agstrdup_html`ed should create an HTML-like attribute
  {
    const char *const corge = agstrdup_html(g, "corge value");
    assert(corge != NULL);
    const int rc = agsafeset(g, "corge", corge, "corge default");
    assert(rc == 0);
    const char *const value = agget(g, "corge");
    assert(value != NULL);
    assert(aghtmlstr(value) &&
           "agsafeset did not detect agstrdup_html-ed value");
  }

  // 6. creating an attribute through `agsafeset` using something we
  // `agstrdup`ed should create a regular text attribute, even when it aliases
  // a previously created regular text value
  {
    const char *const bar = agstrdup(g, "bar value");
    assert(bar != NULL);
    const int rc = agsafeset(g, "bar", bar, "bar default");
    assert(rc == 0);
    const char *const value = agget(g, "bar");
    assert(value != NULL);
    assert(!aghtmlstr(value) && "agsafeset did not detect agstrdup-ed value");
  }

  // 7. creating an attribute through `agsafeset` using something we
  // `agstrdup`ed should create a regular text attribute, even when it aliases a
  // previously created HTML-like value
  {
    const char *const baz = agstrdup(g, "baz value");
    assert(baz != NULL);
    const int rc = agsafeset(g, "baz", baz, "baz default");
    assert(rc == 0);
    const char *const value = agget(g, "baz");
    assert(value != NULL);
    assert(!aghtmlstr(value) && "agsafeset did not detect agstrdup-ed value");
  }

  // 8. creating an attribute through `agsafeset` using something we
  // `agstrdup_html`ed should create an HTML-like attribute, even when it
  // aliases a previously created regular text value
  {
    const char *const bar = agstrdup_html(g, "bar value");
    assert(bar != NULL);
    const int rc = agsafeset(g, "bar", bar, "bar default");
    assert(rc == 0);
    const char *const value = agget(g, "bar");
    assert(value != NULL);
    assert(aghtmlstr(value) &&
           "agsafeset did not detect agstrdup_html-ed value");
  }

  // 9. creating an attribute through `agsafeset` using something we
  // `agstrdup_html`ed should create an HTML-like attribute, even when it
  // aliases a previously created HTML-like value
  {
    const char *const baz = agstrdup_html(g, "baz value");
    assert(baz != NULL);
    const int rc = agsafeset(g, "baz", baz, "baz default");
    assert(rc == 0);
    const char *const value = agget(g, "baz");
    assert(value != NULL);
    assert(aghtmlstr(value) &&
           "agsafeset did not detect agstrdup_html-ed value");
  }

  (void)agclose(g);

  return 0;
}
