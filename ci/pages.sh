#!/usr/bin/env bash

# build documentation website

if [ -z ${CI+x} ]; then
  echo "this script is only intended to run in CI" >&2
  exit 1
fi

set -e
set -o pipefail
set -u
set -x

# build and install Graphviz, which will be used by Doxygen
GV_VERSION=$(python3 gen_version.py)
tar xfz graphviz-${GV_VERSION}.tar.gz
mkdir build
cd build
../graphviz-${GV_VERSION}/configure
make
make install

# Generate the Doxygen docs
make doxygen

# move the output artifacts to where ../.gitlab-ci.yml expects them
mv public ../
