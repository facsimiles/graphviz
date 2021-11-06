#!/bin/sh -e

set -x

if [ "$( uname -s )" = "Darwin" ]; then
    ID=$( uname -s )
    VERSION_ID=$( uname -r )
else
    cat /etc/os-release
    . /etc/os-release
fi
printf '%s\n' "${ID}" >OS_ID
GV_VERSION=$( cat GRAPHVIZ_VERSION )
COLLECTION=$( cat COLLECTION )
DIR=Packages/${COLLECTION}/${ID}/${VERSION_ID}
ARCH=$( uname -m )

# Remove possible old version of Graphviz so that we can detect if
# individual Graphviz tools are missing in the current release
if [ "${ID}" = "centos" -o "${ID}" = "fedora" ]; then
    # Both doxgen and asciidoc depend on graphviz and needs to be
    # removed to allow Graphviz to be removed
    if rpm -q doxygen; then
        rpm --erase doxygen
    fi
    if rpm -q asciidoc; then
        rpm --erase asciidoc
    fi
    if rpm -q graphviz; then
        rpm --erase graphviz
    fi
fi
if [ "${build_system}" = "cmake" ]; then
    if [ "${ID_LIKE}" = "debian" ]; then
        apt install ./${DIR}/os/Graphviz-${GV_VERSION}-Linux.deb
    elif [ "${ID}" = "Darwin" ]; then
        unzip ${DIR}/os/Graphviz-${GV_VERSION}-Darwin.zip
        cp -rp Graphviz-${GV_VERSION}-Darwin/* /usr/local
    else
        rpm --install --force ${DIR}/os/Graphviz-${GV_VERSION}-Linux.rpm
    fi
else
    if [ "${ID_LIKE}" = "debian" ]; then
        apt install ./${DIR}/os/libgraphviz4_${GV_VERSION}-1_amd64.deb
        apt install ./${DIR}/os/libgraphviz-dev_${GV_VERSION}-1_amd64.deb
        apt install ./${DIR}/os/graphviz_${GV_VERSION}-1_amd64.deb
    elif [ "${ID}" = "Darwin" ]; then
        tar xf ${DIR}/os/graphviz-${GV_VERSION}-${ARCH}.tar.gz
        cp -rp build/* /usr/local
    else
        rpm --install --force \
            ${DIR}/os/graphviz-${GV_VERSION}*.rpm \
            ${DIR}/os/graphviz-libs-${GV_VERSION}*.rpm \
            ${DIR}/os/graphviz-devel-${GV_VERSION}*.rpm \
            ${DIR}/os/graphviz-plugins-core-${GV_VERSION}*.rpm \
            ${DIR}/os/graphviz-plugins-x-${GV_VERSION}*.rpm \
            ${DIR}/os/graphviz-x-${GV_VERSION}*.rpm \
            ${DIR}/os/graphviz-gd-${GV_VERSION}*.rpm \
            ${DIR}/os/graphviz-qt-${GV_VERSION}*.rpm \
            ${DIR}/os/graphviz-plugins-gd-${GV_VERSION}*.rpm \
            ${DIR}/os/graphviz-nox-${GV_VERSION}*.rpm
    fi
fi
