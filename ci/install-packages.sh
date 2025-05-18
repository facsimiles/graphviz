#!/usr/bin/env bash

if [ -z ${CI+x} ]; then
  echo "this script is only intended to run in CI" >&2
  exit 1
fi

set -e
set -o pipefail
set -u
set -x

if [ "$( uname -s )" = "Darwin" ]; then
    ID=$( uname -s )
    VERSION_ID=$( uname -r )
else
    cat /etc/os-release
    . /etc/os-release
fi
GV_VERSION=$(python3 gen_version.py)
DIR=Packages/${ID}/${VERSION_ID}
ARCH=$( uname -m )

if [ "${build_system}" = "cmake" ]; then
    if [ "${ID_LIKE:-}" = "debian" ]; then
        apt install ./${DIR}/graphviz-${GV_VERSION}-cmake.deb
    elif [ "${ID}" = "Darwin" ]; then
        unzip ${DIR}/Graphviz-${GV_VERSION}-Darwin.zip
        sudo cp -rp Graphviz-${GV_VERSION}-Darwin/* /usr/local
    else
        rpm --install --force -vv ${DIR}/graphviz-${GV_VERSION}-cmake.rpm
    fi
else
    if [ "${ID_LIKE:-}" = "debian" ]; then
        tar xf ${DIR}/graphviz-${GV_VERSION}-debs.tar.xz
        apt install ./libgraphviz4_${GV_VERSION}-1_amd64.deb
        apt install ./libgraphviz-dev_${GV_VERSION}-1_amd64.deb
        apt install ./graphviz_${GV_VERSION}-1_amd64.deb
        apt install ./libgv-python_${GV_VERSION}-1_amd64.deb
        apt install ./libgv-ruby_${GV_VERSION}-1_amd64.deb
        apt install ./libgv-tcl_${GV_VERSION}-1_amd64.deb
    elif [ "${ID}" = "Darwin" ]; then
        sudo installer -verbose -target / -pkg ${DIR}/graphviz-${GV_VERSION}-${ARCH}.pkg
    else
        tar xvf ${DIR}/graphviz-${GV_VERSION}-rpms.tar.xz
        rpm --install --force -vv graphviz-*${GV_VERSION}*.rpm
    fi
fi
