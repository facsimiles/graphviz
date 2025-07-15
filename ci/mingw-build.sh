#!/usr/bin/env bash

if [ -z ${CI+x} ]; then
  echo "this script is only intended to run in CI" >&2
  exit 1
fi

set -e
set -o pipefail
set -u
set -x

ci/mingw-install.sh

export PATH=$PATH:/c/Git/cmd

if [ "${build_system}" = "cmake" ]; then
    # Use the libs installed with pacman instead of those in
    # https://gitlab.com/graphviz/graphviz-windows-dependencies.
    export CMAKE_OPTIONS="${CMAKE_OPTIONS:-} -Duse_win_pre_inst_libs=OFF"
    export CMAKE_OPTIONS="$CMAKE_OPTIONS -DENABLE_LTDL=ON"
    export CMAKE_OPTIONS="$CMAKE_OPTIONS -DWITH_EXPAT=ON"
    export CMAKE_OPTIONS="$CMAKE_OPTIONS -DWITH_GVEDIT=ON"
    export CMAKE_OPTIONS="$CMAKE_OPTIONS -DWITH_ZLIB=ON"
fi

ci/build.sh
