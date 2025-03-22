/// @file
/// @brief Accompanying test code for test_2641[agset]

#include <assert.h>
#include <graphviz/cgraph.h>
#include <stddef.h>

int main(void) {

  // create a new, empty graph
  Agraph_t *const g = agopen("foo", Agdirected, NULL);
  assert(g != NULL);

  // 1. a regular text attribute should be created as regular text
  {
    // create the attribute initially as text
    Agsym_t *const attr = agattr_text(g, AGRAPH, "bar", "bar placeholder");
    assert(attr != NULL);
    // assign it a value, explicitly as regular text
    const int rc = agset_text(g, "bar", "bar value");
    assert(rc == 0);
    const char *const value = agget(g, "bar");
    assert(value != NULL);
    assert(!aghtmlstr(value) &&
           "regular text attribute was created as HTML-like");
  }

  // 2. an HTML-like text attribute should be created as HTML-like
  {
    // create the attribute initially as text
    Agsym_t *const attr = agattr_text(g, AGRAPH, "baz", "baz placeholder");
    assert(attr != NULL);
    // assign it a value, explicitly as HTML-like
    char *const baz = "baz";
    const int rc = agset_html(g, baz, "baz value");
    assert(rc == 0);
    const char *const value = agget(g, "baz");
    assert(value != NULL);
    assert(aghtmlstr(value) &&
           "HTML-like attribute was created as regular text");
  }

  // 3. creating an attribute through the ambiguous `agset` API should default
  // to regular text
  { // create an attribute initially as text
    Agsym_t *const attr = agattr_text(g, AGRAPH, "qux", "qux placeholder");
    assert(attr != NULL);
    // assign it a value, ambiguously
    const int rc = agset(g, "qux", "qux value");
    assert(rc == 0);
    const char *const value = agget(g, "qux");
    assert(value != NULL);
    assert(!aghtmlstr(value) && "agset created a new attribute as HTML-like");
  }
  {
    // create an attribute initially as HTML-like
    Agsym_t *const attr = agattr_html(g, AGRAPH, "quux", "quux placeholder");
    assert(attr != NULL);
    // assign it a value, ambiguously
    const int rc = agset(g, "quux", "quux value");
    assert(rc == 0);
    const char *const value = agget(g, "quux");
    assert(value != NULL);
    assert(!aghtmlstr(value) && "agset created a new attribute as HTML-like");
  }

  // 4. creating an attribute through `agset` using something we `agstrdup`ed
  // should create a regular text attribute
  { // create an attribute initially as text
    Agsym_t *const attr = agattr_text(g, AGRAPH, "corge", "corge placeholder");
    assert(attr != NULL);
    // assign it a value that was `agstrdup`ed
    const char *const corge = agstrdup(g, "corge value");
    const int rc = agset(g, "corge", corge);
    assert(rc == 0);
    const char *const value = agget(g, "corge");
    assert(value != NULL);
    assert(!aghtmlstr(value) && "agset did not detect agstrdup-ed value");
  }
  {
    // create an attribute initially as HTML-like
    Agsym_t *const attr =
        agattr_html(g, AGRAPH, "grault", "grault placeholder");
    assert(attr != NULL);
    // assign it a value that was `agstrdup`ed
    const char *const grault = agstrdup(g, "grault value");
    const int rc = agset(g, "grault", grault);
    assert(rc == 0);
    const char *const value = agget(g, "grault");
    assert(value != NULL);
    assert(!aghtmlstr(value) && "agset did not detect agstrdup-ed value");
  }

  // 5. creating an attribute through `agset` using something we
  // `agstrdup_html`ed should create an HTML-like attribute
  { // create an attribute initially as text
    Agsym_t *const attr =
        agattr_text(g, AGRAPH, "garply", "garply placeholder");
    assert(attr != NULL);
    // assign it a value that was `agstrdup_html`ed
    const char *const garply = agstrdup_html(g, "garply value");
    const int rc = agset(g, "garply", garply);
    assert(rc == 0);
    const char *const value = agget(g, "garply");
    assert(value != NULL);
    assert(aghtmlstr(value) && "agset did not detect agstrdup_html-ed value");
  }
  {
    // create an attribute initially as HTML-like
    Agsym_t *const attr = agattr_html(g, AGRAPH, "waldo", "waldo placeholder");
    assert(attr != NULL);
    // assign it a value that was `agstrdup_html`ed
    const char *const waldo = agstrdup_html(g, "waldo value");
    const int rc = agset(g, "waldo", waldo);
    assert(rc == 0);
    const char *const value = agget(g, "waldo");
    assert(value != NULL);
    assert(aghtmlstr(value) && "agset did not detect agstrdup_html-ed value");
  }

  // 6. creating an attribute through `agset` using something we `agstrdup`ed
  // should create a regular text attribute, even when it aliases a previously
  // created regular text value
  { // create an attribute initially as text
    Agsym_t *const attr = agattr_text(g, AGRAPH, "fred", "fred placeholder");
    assert(attr != NULL);
    // assign it a value that was `agstrdup`ed
    const char *const corge = agstrdup(g, "corge value");
    const int rc = agset(g, "fred", corge);
    assert(rc == 0);
    const char *const value = agget(g, "fred");
    assert(value != NULL);
    assert(!aghtmlstr(value) && "agset did not detect agstrdup-ed value");
  }
  {
    // create an attribute initially as HTML-like
    Agsym_t *const attr = agattr_html(g, AGRAPH, "plugh", "plugh placeholder");
    assert(attr != NULL);
    // assign it a value that was `agstrdup`ed
    const char *const grault = agstrdup(g, "grault value");
    const int rc = agset(g, "plugh", grault);
    assert(rc == 0);
    const char *const value = agget(g, "plugh");
    assert(value != NULL);
    assert(!aghtmlstr(value) && "agset did not detect agstrdup-ed value");
  }

  // 7. creating an attribute through `agset` using something we `agstrdup`ed
  // should create a regular text attribute, even when it aliases a previously
  // created HTML-like value
  { // create an attribute initially as text
    Agsym_t *const attr = agattr_text(g, AGRAPH, "xyzzy", "xyzzy placeholder");
    assert(attr != NULL);
    // assign it a value that was `agstrdup`ed
    const char *const garply = agstrdup_text(g, "garply value");
    const int rc = agset(g, "xyzzy", garply);
    assert(rc == 0);
    const char *const value = agget(g, "xyzzy");
    assert(value != NULL);
    assert(!aghtmlstr(value) && "agset did not detect agstrdup-ed value");
  }
  {
    // create an attribute initially as HTML-like
    Agsym_t *const attr = agattr_html(g, AGRAPH, "thud", "thud placeholder");
    assert(attr != NULL);
    // assign it a value that was `agstrdup`ed
    const char *const waldo = agstrdup_text(g, "waldo value");
    const int rc = agset(g, "thud", waldo);
    assert(rc == 0);
    const char *const value = agget(g, "thud");
    assert(value != NULL);
    assert(!aghtmlstr(value) && "agset did not detect agstrdup-ed value");
  }

  // 8. creating an attribute through `agset` using something we
  // `agstrdup_html`ed should create an HTML-like attribute, even when it
  // aliases a previously created regular text value
  { // create an attribute initially as text
    Agsym_t *const attr =
        agattr_text(g, AGRAPH, "bazola", "bazola placeholder");
    assert(attr != NULL);
    // assign it a value that was `agstrdup_html`ed
    const char *const corge = agstrdup_html(g, "corge value");
    const int rc = agset(g, "bazola", corge);
    assert(rc == 0);
    const char *const value = agget(g, "bazola");
    assert(value != NULL);
    assert(aghtmlstr(value) && "agset did not detect agstrdup_html-ed value");
  }
  {
    // create an attribute initially as HTML-like
    Agsym_t *const attr =
        agattr_html(g, AGRAPH, "ztesch", "ztesch placeholder");
    assert(attr != NULL);
    // assign it a value that was `agstrdup_html`ed
    const char *const grault = agstrdup_html(g, "grault value");
    const int rc = agset(g, "ztesch", grault);
    assert(rc == 0);
    const char *const value = agget(g, "ztesch");
    assert(value != NULL);
    assert(aghtmlstr(value) && "agset did not detect agstrdup_html-ed value");
  }

  // 9. creating an attribute through `agset` using something we
  // `agstrdup_html`ed should create an HTML-like attribute, even when it
  // aliases a previously created HTML-like value
  { // create an attribute initially as text
    Agsym_t *const attr = agattr_text(g, AGRAPH, "grunt", "grunt placeholder");
    assert(attr != NULL);
    // assign it a value that was `agstrdup_html`ed
    const char *const garply = agstrdup_html(g, "garply value");
    const int rc = agset(g, "grunt", garply);
    assert(rc == 0);
    const char *const value = agget(g, "grunt");
    assert(value != NULL);
    assert(aghtmlstr(value) && "agset did not detect agstrdup_html-ed value");
  }
  {
    // create an attribute initially as HTML-like
    Agsym_t *const attr = agattr_html(g, AGRAPH, "gorp", "gorp placeholder");
    assert(attr != NULL);
    // assign it a value that was `agstrdup_html`ed
    const char *const waldo = agstrdup_html(g, "waldo value");
    const int rc = agset(g, "gorp", waldo);
    assert(rc == 0);
    const char *const value = agget(g, "gorp");
    assert(value != NULL);
    assert(aghtmlstr(value) && "agset did not detect agstrdup_html-ed value");
  }

  (void)agclose(g);

  return 0;
}
