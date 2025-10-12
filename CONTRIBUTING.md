# Contributor’s Guide

[[_TOC_]]

Welcome to Graphviz and thank you for your interest in helping! We accept many
different types of contributions:

* Bug reports
* New feature requests
* Documentation improvements
* New test cases
* Code fixes

## Code of Conduct

TODO

## We use Gitlab for development

Graphviz is hosted at https://gitlab.com/graphviz/graphviz. We use Gitlab for
issue tracking, merge requests, code review, and continuous integration.

The Graphviz website is also hosted on Gitlab at
https://gitlab.com/graphviz/graphviz.gitlab.io. Fixes and improvements to
website content can be submitted as merge requests to this repository.

## We use Discourse for discussion

There is a forum for Graphviz at https://forum.graphviz.org/. Here you can
post questions or open Graphviz-related topics, as well as search for answers to
common questions.

## Development style and guidelines

If you want to make a change to Graphviz code, please feel free to submit a
[Merge Request](https://gitlab.com/graphviz/graphviz/-/merge_requests/new) (MR).
Make sure to:

1. Structure your MR as a series of small, self-contained commits with
   explanatory commit messages. Each commit should be doing a single thing. The
   goal for commit messages should be to write something that can be understood
   by a stranger 10 or more years in the future. The intermediate state at each
   commit in your MR should be capable of passing the test suite.

2. Write an informative top level MR description during submission. This is the
   cover letter that orients reviewers to what your MR is changing, and _more
   importantly_ why.

The policy is to merge a request when:

no maintainer has objected AND ((the request was submitted by a maintainer AND
was opened ≥ 5 days ago) OR the request is approved by ≥ 1 maintainer)

Changes that have user-facing effects will need a CHANGELOG.md entry, to be
appended to the relevant section (Added, Changed, or Fixed) after any existing
entries for the upcoming release. Please ask in the MR description if you are
unsure if your changes need an entry. An API-breaking change (e.g. a functional
change to a C header mentioned in `pkginclude_HEADERS` in a Makefile.am file)
will probably require discussion of the relative cost and benefit.

When addressing MR feedback, rebase your branch and edit the commits in your
series to apply reviewers’ suggestions. If you have not used `git rebase`
before, it is recommended to work through some tutorials on it first. It is not
complex to learn but requires some practice to become adept. When rebasing your
MR, if possible avoid pulling in new unrelated changes that have landed in the
main branch. When rebasing interactively, you can achieve this with
`git rebase --interactive $(git merge-base origin/main HEAD)`.

### C and C++ style

Graphviz is written predominantly in C and C++. C code is compiled under
[C17](https://en.wikipedia.org/wiki/C17_(C_standard_revision)) and C++ code is
compiled under [C++17](https://en.cppreference.com/w/cpp/17). It is generally
possible to rely on a standards-conformant compiler with the following
exceptions:

* The `printf` format specifier `"%zu"` cannot be used. See
  lib/util/prisize_t.h for an explanation of why.
* The `exit` function should not be called directly. See lib/util/exit.h for an
  explanation of why and what to do instead.
* Static array dimensions (`void f(int x[static 42])`) cannot be used because
  MSVC does not support this syntax. Use the equivalent decayed-to-pointer type
  instead (`void f(int *x)`).
* The ISO8601 Format specifiers to `strftime` like `"%F"` and `"%T"` cannot be
  used. These are not implemented in MinGW’s runtime.

POSIX and Linux-/Unix-specific extensions are generally not usable, with a few
exceptions. E.g. lib/util/strcasecmp.h provides a way to use `strcasecmp`
portably.

New code should be written to conform with LLVM style, enforceable with the
.clang-format file in the root of the repository. Changes to existing code
should attempt to conform to the surrounding style for ease of review.
