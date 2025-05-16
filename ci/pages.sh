#!/usr/bin/env bash

# build documentation website

set -e
set -o pipefail
set -u
set -x

# build and install Graphviz, which will be used by Doxygen
GV_VERSION=$( cat GRAPHVIZ_VERSION )
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
