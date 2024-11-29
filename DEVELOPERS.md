# Developer's Guide

## Table of contents

[[_TOC_]]

## Directory overview

The main things of interest to you in the Graphviz repository are most likely:

* cmd/ – the command line binaries and gvedit
  * cmd/dot/ – the main `dot` binary, though most of its functionality actually
    lives in libraries
* lib/ – the Graphviz libraries
  * lib/cgraph, lib/gvc – the main user-facing Graphviz libraries
* plugin/ – the plugins, several of which implement core functionality

Once you have familiarized yourself with the above and want to go deeper or
validate changes you have made, you will likely want to look at:

* .gitlab-ci.yml, ci/ – continuous integration testing infrastructure
* tclpkg/ – despite its name, this contains Graphviz bindings for both TCL and
  many other languages
* tests/ – test cases

This is enough to get you started, but viewing `git log` and seeing what files
recent commits touch will also help to orient you.

## Building

Instructions for building Graphviz from source are available
[on the public website](https://graphviz.org/download/source/). However, if you
are building Graphviz from a repository clone for the purpose of testing changes
you are making, you will want to follow different steps.

### Autotools build system

```sh
# generate the configure script for setting up the build
./autogen.sh

# you probably do not want to install your development version of Graphviz over
# the top of your system binaries/libraries, so create a temporary directory as
# an install destination
PREFIX=$(mktemp -d)

# configure the build system
./configure --prefix=${PREFIX}

# if this is the first time you are building Graphviz from source, you should
# inspect the output of ./configure and make sure it is what you expected

# compile Graphviz binaries and libraries
make

# install everything to the temporary directory
make install
```

### CMake build system

Note that Graphviz’ CMake build system is incomplete. It only builds a subset
of the binaries and libraries. For most code-related changes, you will probably
want to test using Autotools instead.

However, if you do want to use CMake:

```sh
# you probably do not want to install your development version of Graphviz over
# the top of your system binaries/libraries, so create a temporary directory as
# an install destination
PREFIX=$(mktemp -d)

# configure the build system
cmake -DCMAKE_INSTALL_PREFIX=${PREFIX} -B build -S .

# compile Graphviz binaries and libraries
cmake --build build

# install everything to the temporary directory
cmake --install build
```

## Testing

The Graphviz test suite uses [pytest](https://pytest.org/). This is not because
of any Python-related specifics in Graphviz, but rather because pytest is a
convenient framework.

If you have compiled Graphviz and installed to a custom location, as described
above, then you can run the test suite from the root of a Graphviz checkout as
follows:

```sh
# run the Graphviz test suite, making the temporary installation visible to it
env PATH=${PREFIX}/bin:${PATH} C_INCLUDE_PATH=${PREFIX}/include \
  LD_LIBRARY_PATH=${PREFIX}/lib LIBRARY_PATH=${PREFIX}/lib \
  PYTHONPATH=${PREFIX}/lib/graphviz/python3 \
  TCLLIBPATH=${PREFIX}/lib/graphviz/tcl \
  PKG_CONFIG_PATH=${PREFIX}/lib/pkgconfig \
  python3 -m pytest tests
```

On macOS, use the same command except replacing `LD_LIBRARY_PATH` with
`DYLD_LIBRARY_PATH`.

You may want to exclude test cases that take a very long time (those marked with
the `@pytest.mark.slow` annotation) if you do not believe your changes affect
them. To do this, include `-m "not slow"` as a further option to Pytest.

To run a single test, you use its name qualified by the file it lives in. E.g.

```sh
env PATH=${PREFIX}/bin:${PATH} C_INCLUDE_PATH=${PREFIX}/include \
  LD_LIBRARY_PATH=${PREFIX}/lib LIBRARY_PATH=${PREFIX}/lib \
  PYTHONPATH=${PREFIX}/lib/graphviz/python3 \
  TCLLIBPATH=${PREFIX}/lib/graphviz/tcl \
  PKG_CONFIG_PATH=${PREFIX}/lib/pkgconfig \
  python3 -m pytest tests/test_regression::test_2225
```

*TODO: on Windows, you probably need to override different environment variables?*

### Writing tests

Graphviz’ use of Pytest is mostly standard. You can find documentation and
examples elsewhere on the internet about Pytest. The following paragraphs cover
Graphviz-specific quirks.

Most new tests should be written as functions in tests/test_regression.py.
Despite this file’s name, it is a home for all sorts of tests, not just
regression tests.

Common test helpers live in tests/gvtest.py, in particular the `dot` function
which wraps Graphviz command line execution in a way that covers most needs.

If you need to call a tool from the Graphviz suite via `subprocess`, use the
`which` function from tests/gvtest.py instead of `shutil.which` or the bare name
of the tool itself. This avoids accidentally invoking system utilities of the
same name or components of a prior Graphviz installation.

The vast majority of tests can be written in pure Python. However, sometimes it
is necessary to write a test in C/C++. In these cases, prefer C because it is
easier to compile and ensure consistent results on all supported platforms. Use
the `run_c` helper in tests/gvtest.py. Refer to existing calls to this function
for examples of how to use it.

The continuous integration tests include running Pylint on all Python files in
the repository. To check your additions are compliant,
`pylint --rcfile=.pylintrc tests/test_regression.py`.

## Performance and profiling

The runtime and memory usage of Graphviz is dependent on the user’s graph. It is
easy to create “expensive” graphs without realizing it using only moderately
sized input. For this reason, users regularly encounter performance bottlenecks
that they need help with. This situation is likely to persist even with hardware
advances, as the size and complexity of the graphs users construct will expand
as well.

This section documents how to build performance-optimized Graphviz binaries and
how to identify performance bottlenecks. Note that this information assumes you
are working in a Linux environment.

### Building an optimized Graphviz

The first step to getting an optimized build is to make sure you are using a
recent compiler. If you have not upgraded your C and C++ compilers for a while,
make sure you do this first.

The simplest way to change flags used during compilation is by setting the
`CFLAGS` and `CXXFLAGS` environment variables:

```sh
env CFLAGS="..." CXXFLAGS="..." ./configure
```

You should use the maximum optimization level for your compiler. E.g. `-O3` for
GCC and Clang. If your toolchain supports it, it is recommended to also enable
link-time optimization (`-flto`). You should also disable runtime assertions
with `-DNDEBUG`.

You can further optimize compilation for the machine you are building on with
`-march=native -mtune=native`. Be aware that the resulting binaries will no
longer be portable (may not run if copied to another machine). These flags are
also not recommended if you are debugging a user issue, because you will end up
profiling and optimizing different code than what may execute on their machine.

Most profilers need a symbol table and/or debugging metadata to give you useful
feedback. You can enable this on GCC and Clang with `-g`.

Putting this all together:

```sh
env CFLAGS="-O3 -flto -DNDEBUG -march=native -mtune=native -g" \
  CXXFLAGS="-O3 -flto -DNDEBUG -march=native -mtune=native -g" ./configure
```

### Profiling

#### [Callgrind](https://valgrind.org/docs/manual/cl-manual.html)

Callgrind is a tool of [Valgrind](https://valgrind.org/) that can measure how
many times a function is called and how expensive the execution of a function is
compared to overall runtime. When you have built an optimized binary according
to the above instructions, you can run it under Callgrind:

```sh
valgrind --tool=callgrind dot -Tsvg test.dot
```

This will produce a file like callgrind.out.2534 in the current directory. You
can use [Kcachegrind](https://kcachegrind.github.io/) to view the results by
running it in the same directory with no arguments:

```sh
kcachegrind
```

If you have multiple trace results in the current directory, Kcachegrind will
load all of them and even let you compare them to each other. See the
Kcachegrind documentation for more information about how to use this tool.

Be aware that execution under Callgrind will be a lot slower than a normal run.
If you need to see instruction-level execution costs, you can pass
`--dump-instr=yes` to Valgrind, but this will further slow execution and is
usually not necessary. To profile with less overhead, you can use a statistical
profiler like Linux Perf.

#### [Linux Perf](https://perf.wiki.kernel.org/index.php/Main_Page)

https://www.markhansen.co.nz/profiling-graphviz/ gives a good introduction. If
you need more, an alternative step-by-step follows.

First, compile Graphviz with debugging symbols enabled (`-ggdb` in the `CFLAGS`
and `CXXFLAGS` lists in the commands described above). Without this, `perf` will
struggle to give you helpful information.

Record a trace of Graphviz:

```sh
perf record -g --call-graph=dwarf --freq=max --events=cycles:u \
  dot -Tsvg test.dot
```

Some things to keep in mind:

* `--freq=max`: this will give you the most accurate profile, but can sometimes
  exceed the I/O capacity of your system. `perf record` will tell you when this
  occurs, but then it will also often corrupt the profile data it is writing.
  This will later cause `perf report` to lock up or crash. To work around this,
  you will need to reduce the frequency.
* `--events=cycles:u`: this will record user-level cycles. This is often what
  you are interested in, but there are many other events you can read about in
  the `perf` man page. `--events=duration_time` may be a more intuitive metric.

Now, examine the trace you recorded:

```sh
perf report -g --children
```

The reporting interface relies on a series of terse keyboard shortcuts. So hit
`?` if you are stuck.

## Processing contributors’ Merge Requests

The expected process contributors should follow is described in CONTRIBUTING.md.
Assuming a contributor has followed this, maintainers are expected to review
incoming MRs and leave feedback through Gitlab. Do your best to be open and
accepting in your language. It is OK to admit you do not know/understand the
code a contributor is modifying – many parts of the code base have only ever
been well understood by ex-maintainers who have since retired.

If you view recent Git history, you will note it is a series of diamonds. This
gives a greater chance of catching problems pre-merge in CI:

```
* A  ◄──── CI ran on this commit post-merge
|\
| * B  ◄── CI ran on this commit pre-merge
| |
| * C      `git diff B A` is empty
|/         I.e. pre-merge CI tested what was about to land in main
* D
|\
| * E
…
```

The alternative is riskier:

```
* A
|\
| * B  ◄── pre-merge CI on this commit did not include changes C, D
|\|
| |\       `git diff B A` is not empty
| | * C    It shows the changes of C and D that were never tested in combination
| | |      with B’s changes until post-merge CI on A
| | * D
…
```

To achieve the former diamond pattern, merge a contributor’s MR as follows:

1. Checking out their branch
2. Rebasing the current main, `git pull --rebase origin main`
3. Force-updating the MR,
   `git push --force-with-lease <their fork> HEAD:<their branch>`
4. Setting the MR to merge when CI passes
5. (do not merge anything else until this MR merges)

## How to make a release

### Downstream packagers/consumers

The following is a list of known downstream software that packages or
distributes Graphviz, along with the best known contact or maintainer we have
for the project. We do not have the resources to coordinate with all these
people prior to a release, but this information is here to give you an idea of
who will be affected by a new Graphviz release.

* [Alpine](https://git.alpinelinux.org/aports/tree/main/graphviz),
  Natanael Copa ncopa@alpinelinux.org
* [Chocolatey](https://chocolatey.org/packages/Graphviz/),
  [@RedBaron2 on Github](https://github.com/RedBaron2)
* [Debian](https://packages.debian.org/sid/graphviz),
  [Laszlo Boszormenyi (GCS) on Debian](https://qa.debian.org/developer.php?login=gcs%40debian.org)
* [.NET bindings](https://github.com/Rubjerg/Graphviz.NetWrapper),
  [@chtenb on Gitlab](https://gitlab.com/chtenb)
* [Fedora](https://src.fedoraproject.org/rpms/graphviz),
  [Jaroslav Škarvada](https://src.fedoraproject.org/user/jskarvad)
* [FreeBSD](https://svnweb.freebsd.org/ports/head/graphics/graphviz/),
  dinoex@FreeBSD.org
* [Homebrew](https://formulae.brew.sh/formula/graphviz#default),
  [@fxcoudert on Github](https://github.com/fxcoudert)
* [Gentoo](https://packages.gentoo.org/packages/media-gfx/graphviz),
  [@SoapGentoo on Gitlab](https://gitlab.com/SoapGentoo)
* [@hpcc-hs/wasm](https://www.npmjs.com/package/@hpcc-js/wasm),
  [@GordonSmith on Github](https://github.com/GordonSmith)
* [MacPorts](https://ports.macports.org/port/graphviz/summary),
  [@ryandesign on Github](https://github.com/ryandesign)
* [NetBSD](https://cdn.netbsd.org/pub/pkgsrc/current/pkgsrc/graphics/graphviz/index.html),
  Michael Bäuerle micha at NetBSD.org
* [PyGraphviz](https://github.com/pygraphviz/pygraphviz),
  [@jarrodmillman on Gitlab](https://gitlab.com/jarrodmillman)
* [Spack](https://spack.readthedocs.io/en/latest/package_list.html#graphviz),
  [@alecbcs on Github](https://github.com/alecbcs)
* [SUSE](https://software.opensuse.org/package/graphviz),
  Christian Vögl cvoegl@suse.de
* [Winget](https://github.com/microsoft/winget-pkgs),
  [@GordonSmith on Github](https://github.com/GordonSmith)

### A note about the examples below

The examples below are for the 2.44.1 release. Modify the version
number for the actual release.

### Using a fork or a clone of the original repo

The instructions below can be used from a fork (recommended) or from a
clone of the main repo.

### Deciding the release version number

This project adheres to
[Semantic Versioning](https://semver.org/spec/v2.0.0.html).
Before making the release, it must be decided if it is a *major*, *minor* or
*patch* release.

If you are making a change that will require an upcoming major or minor version
increment, update the planned version for the next release in parentheses after
the `Unreleased` heading in `CHANGELOG.md`. Remember to also update the diff
link for this heading at the bottom of `CHANGELOG.md`.

#### Stable release versions and development versions numbering convention

See [`gen_version.py`](https://gitlab.com/graphviz/graphviz/-/blob/main/gen_version.py).

### Instructions

#### Creating the release

1. Search the issue tracker for
   [anything tagged with the release-blocker label](https://gitlab.com/graphviz/graphviz/-/issues?label_name=release+blocker).
   If there are any such open issues, you cannot make a new release. Resolve
   these first.

1. Check that the
   [main pipeline](https://gitlab.com/graphviz/graphviz/-/pipelines?ref=main)
   is green

1. Create a local branch and name it e.g. `stable-release-<version>`

   Example: `stable-release-2.44.1`

1. Edit `CHANGELOG.md`

    Change the `[Unreleased (…)]` heading to the upcoming release version and
    target release date.

    At the bottom of the file, add an entry for the new version. These
    entries are not visible in the rendered page, but are essential
    for the version links to the GitLab commit comparisons to work.

    Example:

    ```diff
    -## [Unreleased (2.44.1)]
    +## [2.44.1] - 2020-06-29
    ```

    ```diff
    -[Unreleased (2.44.1)]: https://gitlab.com/graphviz/graphviz/compare/2.44.0...main
    +[2.44.1]: https://gitlab.com/graphviz/graphviz/compare/2.44.0...2.44.1
     [2.44.0]: https://gitlab.com/graphviz/graphviz/compare/2.42.4...2.44.0
     [2.42.4]: https://gitlab.com/graphviz/graphviz/compare/2.42.3...2.42.4
     [2.42.3]: https://gitlab.com/graphviz/graphviz/compare/2.42.2...2.42.3
    ```

1. Commit:

   `git add -p`

   `git commit -m "Stable Release 2.44.1"`

1. Push:

   Example: `git push origin stable-release-2.44.1`

1. Wait until the pipeline has run for your branch and check that it's green

1. Create a merge request

1. Merge the merge request

1. Wait for the
[main pipeline](https://gitlab.com/graphviz/graphviz/-/pipelines?ref=main)
  to run for the new commit and check that it's green

1. The “deployment” CI task will automatically create a release on the
   [Gitlab releases tab](https://gitlab.com/graphviz/graphviz/-/releases). If a
   release is not created, double check your steps and/or inspect
   `gen_version.py` to ensure it is operating correctly. The “deployment” CI
   task will also create a Git tag for the version, e.g. `2.44.1`.

#### Starting development of the next version

1. Decide the tentative next release version. This is normally the latest
   stable release version with the patch number incremented.

1. Create a new local branch and name it e.g. `start-<version>-dev`

   Example: `start-2.44.2-dev`

1. Add a new `[Unreleased (…)]` heading to `CHANGELOG.md`. At the bottom of
   the file, add a new entry for the next release.

   Example:

    ```diff
    +## [Unreleased (2.44.2)]
    +
     ## [2.44.1] - 2020-06-29
    ```

    ```diff
    +[Unreleased (2.44.2)]: https://gitlab.com/graphviz/graphviz/compare/2.44.1...main
     [2.44.1]: https://gitlab.com/graphviz/graphviz/compare/2.44.0...2.44.1
     [2.44.0]: https://gitlab.com/graphviz/graphviz/compare/2.42.4...2.44.0
     [2.42.4]: https://gitlab.com/graphviz/graphviz/compare/2.42.3...2.42.4
     [2.42.3]: https://gitlab.com/graphviz/graphviz/compare/2.42.2...2.42.3
    ```

1. Commit:

    `git add -p`

    Example: `git commit -m "Start 2.44.2 development"`

1. Push:

   Example: `git push origin start-2.44.2-dev`

1. Wait until the pipeline has run for your new branch and check that it's green

1. Create a merge request

1. Merge the merge request

#### Updating the website

1. Fork the
   [Graphviz website repository](https://gitlab.com/graphviz/graphviz.gitlab.io)
   if you do not already have a fork of it

1. Checkout the latest main branch

1. Create a local branch

1. Download and unpack the artifacts from the `main` pipeline `deploy`
   job and copy the `graphviz-<version>.json` file to `data/releases/`.

1. Commit this to your local branch

1. Push this to a branch in your fork

1. Create a merge request

1. Wait for CI to pass

1. Merge the merge request

## How to find which version of Graphviz a change arrived in

The repository history goes back a long way, so you can usually use `git blame`
to discover the origin of any line of code. This will lead you to a commit hash,
but for communicating with users it is more useful to give a Graphviz version
number.

Recent releases use Git tags, so you can use `git tag --contains <hash>` to list
all releases that included a particular commit. The release the change arrived
in is the oldest of the tags listed.

Prior to the release process using tags, versions were only documented in the
ChangeLog file (pre-dating the current CHANGELOG.md). For changes in this range,
run `git log --graph origin/main`, scroll/search to the commit you found from
`git blame` (`/<hash>` if you are using `less` as a pager), then search upwards
for a commit touching “ChangeLog” (`?ChangeLog` if you are using `less` as a
pager). Check the content of this commit to see if it added a new version
heading. If so, you have found the version. If not, return to the commit graph
and continue searching upwards for more commits touching ChangeLog.

## How to update this guide

### Markdown flavor used

This guide uses
[GitLab Flavored Markdown (GFM)](https://docs.gitlab.com/ce/user/markdown.html#gitlab-flavored-markdown-gfm]).

### Rendering this guide locally

This guide can be rendered locally with e.g. [Pandoc](https://pandoc.org/):

`pandoc DEVELOPERS.md --from=gfm -t html -o DEVELOPERS.html`

### Linting this guide locally

[markdownlint-cli](https://github.com/igorshubovych/markdownlint-cli)
can be used to lint it.
