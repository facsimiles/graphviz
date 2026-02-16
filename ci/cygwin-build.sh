#!/usr/bin/env bash

echo "Starting cygwin-build.sh" >&2

if [ -z ${CI+x} ]; then
  echo "this script is only intended to run in CI" >&2
  exit 1
fi

set -e
set -o pipefail
set -u
set -x

# address warning we may see later due to lack of available randomness
diskperf -y

$Env:CYGWIN_SETUP = dependencies/cygwin/setup-x86_64.exe

setup_exe=$(cygpath -u "$CYGWIN_SETUP")
${setup_exe} --quiet-mode --wait --local-package-dir dependencies/cygwin/packages --packages autoconf2.5
${setup_exe} --quiet-mode --wait --local-package-dir dependencies/cygwin/packages --packages automake
${setup_exe} --quiet-mode --wait --local-package-dir dependencies/cygwin/packages --packages bison
${setup_exe} --quiet-mode --wait --local-package-dir dependencies/cygwin/packages --packages cmake
${setup_exe} --quiet-mode --wait --local-package-dir dependencies/cygwin/packages --packages flex
${setup_exe} --quiet-mode --wait --local-package-dir dependencies/cygwin/packages --packages gcc-core
${setup_exe} --quiet-mode --wait --local-package-dir dependencies/cygwin/packages --packages gcc-g++
${setup_exe} --quiet-mode --wait --local-package-dir dependencies/cygwin/packages --packages git
${setup_exe} --quiet-mode --wait --local-package-dir dependencies/cygwin/packages --packages libcairo-devel
${setup_exe} --quiet-mode --wait --local-package-dir dependencies/cygwin/packages --packages libexpat-devel
${setup_exe} --quiet-mode --wait --local-package-dir dependencies/cygwin/packages --packages libpango1.0-devel
${setup_exe} --quiet-mode --wait --local-package-dir dependencies/cygwin/packages --packages libgd-devel
${setup_exe} --quiet-mode --wait --local-package-dir dependencies/cygwin/packages --packages libtool
${setup_exe} --quiet-mode --wait --local-package-dir dependencies/cygwin/packages --packages make
${setup_exe} --quiet-mode --wait --local-package-dir dependencies/cygwin/packages --packages python3
${setup_exe} --quiet-mode --wait --local-package-dir dependencies/cygwin/packages --packages zlib-devel

# Use the libs installed with cygwinsetup instead of those in
# https://gitlab.com/graphviz/graphviz-windows-dependencies. Also disable GVEdit
# because we do not have Qt installed.
export CMAKE_OPTIONS="-Duse_win_pre_inst_libs=OFF -DWITH_GVEDIT=OFF"
export CMAKE_OPTIONS="$CMAKE_OPTIONS -DENABLE_LTDL=ON"
export CMAKE_OPTIONS="$CMAKE_OPTIONS -DWITH_EXPAT=ON"
export CMAKE_OPTIONS="$CMAKE_OPTIONS -DWITH_ZLIB=ON"

# temporarily don't fail on warnings.
# export CFLAGS="-Werror -Wno-error=implicit-fallthrough -Wno-error=sign-conversion -Wno-error=attributes"
# export CXXFLAGS="-Werror"

# make Git running under the Cygwin user trust files owned by the Windows user
git config --global --add safe.directory $(pwd)

echo "running ci/build.sh" >&2

ci/build.sh
