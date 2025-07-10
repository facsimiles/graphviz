#!/usr/bin/env bash

# this script does something close to the work flow end users may follow when
# building Graphviz

if [ -z ${CI+x} ]; then
  echo "this script is only intended to run in CI" >&2
  exit 1
fi

set -e
set -o pipefail
set -u
set -x

# output some info for debugging
uname -rms
cat /etc/os-release

GV_VERSION=$(python3 gen_version.py)

# unpack the portable source tarball
tar xfz graphviz-${GV_VERSION}.tar.gz

# setup a directory for building in
mkdir build
cd build
../graphviz-${GV_VERSION}/configure

# build Graphviz
make

# install Graphviz
make install
