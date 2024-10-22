#!/usr/bin/env bash

set -e
set -o pipefail
set -u
set -x

ci/test_coverage.py --init
pushd build
# Use GVBINDIR to specify where to generate and load config6
# since ctest sets LD_LIBRARY_PATH to point to all the
# locations where the libs reside before cpack copies them to
# the install directory. Without this Graphviz tries to use
# the directory lib/gvc/graphviz which does not exist.
export GVBINDIR=$(pwd)/plugin/dot_layout
cmd/dot/dot -v -c
ctest --output-on-failure
unset GVBINDIR
popd
# Many of the tests run by pytest currently trigger ASan
# memory leak detections. Disable those for now.
export ASAN_OPTIONS=detect_leaks=0
export UBSAN_OPTIONS=print_stacktrace=1
ci/test-job-script.sh
ci/test_coverage.py --analyze
