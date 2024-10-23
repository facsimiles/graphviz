#!/usr/bin/env bash

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
GV_VERSION=$( cat GRAPHVIZ_VERSION )
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
        apt install ./libgv-ruby_${GV_VERSION}-1_amd64.deb
        apt install ./libgv-tcl_${GV_VERSION}-1_amd64.deb
    elif [ "${ID}" = "Darwin" ]; then
        tar xf ${DIR}/graphviz-${GV_VERSION}-${ARCH}.tar.gz
        sudo cp -rp build/* /usr/local
    else
        tar xvf ${DIR}/graphviz-${GV_VERSION}-rpms.tar.xz
        rpm --install --force -vv graphviz-*${GV_VERSION}*.rpm
    fi
fi
