#!/usr/bin/env bash

set -e
set -o pipefail
set -u
set -x

/cygdrive/c/setup-x86_64.exe --quiet-mode --wait --packages autoconf2.5
/cygdrive/c/setup-x86_64.exe --quiet-mode --wait --packages automake
/cygdrive/c/setup-x86_64.exe --quiet-mode --wait --packages bison
/cygdrive/c/setup-x86_64.exe --quiet-mode --wait --packages cmake
/cygdrive/c/setup-x86_64.exe --quiet-mode --wait --packages flex
/cygdrive/c/setup-x86_64.exe --quiet-mode --wait --packages gcc-core
/cygdrive/c/setup-x86_64.exe --quiet-mode --wait --packages gcc-g++
/cygdrive/c/setup-x86_64.exe --quiet-mode --wait --packages libcairo-devel
/cygdrive/c/setup-x86_64.exe --quiet-mode --wait --packages libexpat-devel
/cygdrive/c/setup-x86_64.exe --quiet-mode --wait --packages libpango1.0-devel
/cygdrive/c/setup-x86_64.exe --quiet-mode --wait --packages libgd-devel
/cygdrive/c/setup-x86_64.exe --quiet-mode --wait --packages libtool
/cygdrive/c/setup-x86_64.exe --quiet-mode --wait --packages make
/cygdrive/c/setup-x86_64.exe --quiet-mode --wait --packages python3
/cygdrive/c/setup-x86_64.exe --quiet-mode --wait --packages zlib-devel

# Use the libs installed with cygwinsetup instead of those in
# https://gitlab.com/graphviz/graphviz-windows-dependencies. Also disable GVEdit
# because we do not have Qt installed.
export CMAKE_OPTIONS="-Duse_win_pre_inst_libs=OFF -DWITH_GVEDIT=OFF"
export CMAKE_OPTIONS="$CMAKE_OPTIONS -DENABLE_LTDL=ON"
export CMAKE_OPTIONS="$CMAKE_OPTIONS -DWITH_EXPAT=ON"
export CMAKE_OPTIONS="$CMAKE_OPTIONS -DWITH_ZLIB=ON"

ci/build.sh
