#!/usr/bin/env bash

echo "Starting cygwin-build.sh" >&2
echo "TEMP is $TEMP" >&2

if [ -z ${CI+x} ]; then
  echo "this script is only intended to run in CI" >&2
  exit 1
fi

set -e
set -o pipefail
set -u
set -x

setup_dir="$TEMP"
"$setup_dir"/setup-x86_64.exe --quiet-mode -v --wait --packages autoconf2.5
"$setup_dir"/setup-x86_64.exe --quiet-mode -v --wait --packages automake
"$setup_dir"/setup-x86_64.exe --quiet-mode -v --wait --packages bison
"$setup_dir"/setup-x86_64.exe --quiet-mode -v --wait --packages cmake
"$setup_dir"/setup-x86_64.exe --quiet-mode -v --wait --packages flex
"$setup_dir"/setup-x86_64.exe --quiet-mode -v --wait --packages gcc-core
"$setup_dir"/setup-x86_64.exe --quiet-mode -v --wait --packages gcc-g++
"$setup_dir"/setup-x86_64.exe --quiet-mode -v --wait --packages git
"$setup_dir"/setup-x86_64.exe --quiet-mode -v --wait --packages libcairo-devel
"$setup_dir"/setup-x86_64.exe --quiet-mode -v --wait --packages libexpat-devel
"$setup_dir"/setup-x86_64.exe --quiet-mode -v --wait --packages libpango1.0-devel
"$setup_dir"/setup-x86_64.exe --quiet-mode -v --wait --packages libgd-devel
"$setup_dir"/setup-x86_64.exe --quiet-mode -v --wait --packages libtool
"$setup_dir"/setup-x86_64.exe --quiet-mode -v --wait --packages make
"$setup_dir"/setup-x86_64.exe --quiet-mode -v --wait --packages python3
"$setup_dir"/setup-x86_64.exe --quiet-mode -v --wait --packages zlib-devel

# Use the libs installed with cygwinsetup instead of those in
# https://gitlab.com/graphviz/graphviz-windows-dependencies. Also disable GVEdit
# because we do not have Qt installed.
export CMAKE_OPTIONS="-Duse_win_pre_inst_libs=OFF -DWITH_GVEDIT=OFF"
export CMAKE_OPTIONS="$CMAKE_OPTIONS -DENABLE_LTDL=ON"
export CMAKE_OPTIONS="$CMAKE_OPTIONS -DWITH_EXPAT=ON"
export CMAKE_OPTIONS="$CMAKE_OPTIONS -DWITH_ZLIB=ON"

# make Git running under the Cygwin user trust files owned by the Windows user
git config --global --add safe.directory $(pwd)

echo "running ci/build.sh" >&2

ci/build.sh
