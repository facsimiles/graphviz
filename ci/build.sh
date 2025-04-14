#!/usr/bin/env bash

set -e
set -o pipefail
set -u
set -x

if [ -f /etc/os-release ]; then
    cat /etc/os-release
    . /etc/os-release
else
    ID=$( uname -s )
    # remove trailing text after actual version
    VERSION_ID=$( uname -r | sed "s/\([0-9\.]*\).*/\1/")
fi

# Make a lowercase equivalent of `${ID}`. Bash on macOS is 3.2, which does not
# support `${foo,,}`.
id=$(echo "${ID}" | tr '[:upper:]' '[:lower:]')

if [[ ${id} == msys* ]]; then
    # MSYS2/MinGW doesn't have VERSION_ID in /etc/os-release
    VERSION_ID=$( uname -r )
fi

# validate we have Git and echo version into log
git --version

META_DATA_DIR=Metadata/${ID}/${VERSION_ID}
mkdir -p ${META_DATA_DIR}
DIR=$(pwd)/Packages/${ID}/${VERSION_ID}
mkdir -p ${DIR}
ARCH=$( uname -m )
build_system=${build_system:-autotools}
if [ "${build_system}" = "cmake" ]; then
    cmake --version
    mkdir build
    pushd build
    cmake --log-level=VERBOSE --warn-uninitialized -Werror=dev \
      ${CMAKE_OPTIONS:-} ..
    cmake --build .
    cpack
    popd
    if [[ ${id} == ubuntu* ]]; then
        GV_VERSION=$(python3 gen_version.py)
        mv build/Graphviz-${GV_VERSION}-Linux.deb ${DIR}/graphviz-${GV_VERSION}-cmake.deb
    elif [[ ${id} == fedora* || ${id} == rocky* ]]; then
        GV_VERSION=$(python3 gen_version.py)
        mv build/Graphviz-${GV_VERSION}-Linux.rpm ${DIR}/graphviz-${GV_VERSION}-cmake.rpm
    elif [[ ${id} == darwin* ]]; then
        mv build/*.zip ${DIR}/
    elif [[ ${id} == msys* ]]; then
        mv build/*.zip ${DIR}/
        mv build/*.exe ${DIR}/
    elif [[ ${id} == cygwin* ]]; then
        mv build/*.zip ${DIR}/
        mv build/*.tar.bz2 ${DIR}/
    else
        echo "Error: ID=${ID} is unknown" >&2
        exit 1
    fi
elif [[ "${CONFIGURE_OPTIONS:-}" =~ "--enable-static" ]]; then
    GV_VERSION=$( cat GRAPHVIZ_VERSION )
    if [ "${use_autogen:-no}" = "yes" ]; then
        ./autogen.sh
        ./configure ${CONFIGURE_OPTIONS:-} --prefix=$( pwd )/build | tee >(./ci/extract-configure-log.sh >${META_DATA_DIR}/configure.log)
        make
        make install
        tar cf - -C build . | xz -9 -c - > ${DIR}/graphviz-${GV_VERSION}-${ARCH}.tar.xz
    else
        tar xfz graphviz-${GV_VERSION}.tar.gz
        pushd graphviz-${GV_VERSION}
        ./configure $CONFIGURE_OPTIONS --prefix=$( pwd )/build | tee >(../ci/extract-configure-log.sh >../${META_DATA_DIR}/configure.log)
        make
        make install
        popd
    fi
else
    GV_VERSION=$( cat GRAPHVIZ_VERSION )
    if [[ ${id} == ubuntu* ]]; then
        tar xfz graphviz-${GV_VERSION}.tar.gz
        (cd graphviz-${GV_VERSION}; fakeroot make -f debian/rules binary) | tee >(ci/extract-configure-log.sh >${META_DATA_DIR}/configure.log)
        tar cf - *.deb *.ddeb | xz -9 -c - >${DIR}/graphviz-${GV_VERSION}-debs.tar.xz
    elif [[ ${id} == fedora* || ${id} == rocky* ]]; then
        rm -rf ${HOME}/rpmbuild
        rpmbuild -ta graphviz-${GV_VERSION}.tar.gz | tee >(ci/extract-configure-log.sh >${META_DATA_DIR}/configure.log)
        pushd ${HOME}/rpmbuild/RPMS
        mv */*.rpm ./
        tar cf - *.rpm | xz -9 -c - >${DIR}/graphviz-${GV_VERSION}-rpms.tar.xz
        popd
    elif [[ ${id} == darwin* ]]; then
        tar xfz graphviz-${GV_VERSION}.tar.gz
        pushd graphviz-${GV_VERSION}
        ./configure --prefix=$( pwd )/build --with-quartz=yes
        make
        make install
        python3 ../ci/make_relocatable.py $( pwd )/build
        tar cfz ${DIR}/graphviz-${GV_VERSION}-${ARCH}.tar.gz --options gzip:compression-level=9 build
        popd
    elif [[ ${id} == cygwin* || ${id} == msys* ]]; then
        if [[ ${id} == msys* ]]; then
            # ensure that MinGW tcl shell is used in order to find tcl functions
            CONFIGURE_OPTIONS="${CONFIGURE_OPTIONS:-} --with-tclsh=${MSYSTEM_PREFIX}/bin/tclsh86"
        else # Cygwin
            # avoid platform detection problems
            CONFIGURE_OPTIONS="${CONFIGURE_OPTIONS:-} --build=x86_64-pc-cygwin"
        fi
        if [ "${use_autogen:-no}" = "yes" ]; then
            ./autogen.sh
            ./configure ${CONFIGURE_OPTIONS:-} --prefix=$( pwd )/build | tee >(./ci/extract-configure-log.sh >${META_DATA_DIR}/configure.log)
            make
            make install
            tar cf - -C build . | xz -9 -c - > ${DIR}/graphviz-${GV_VERSION}-${ARCH}.tar.xz
        else
            tar xfz graphviz-${GV_VERSION}.tar.gz
            pushd graphviz-${GV_VERSION}
            ./configure ${CONFIGURE_OPTIONS:-} --prefix=$( pwd )/build | tee >(../ci/extract-configure-log.sh >../${META_DATA_DIR}/configure.log)
            make
            make install
            popd
            tar cf - -C graphviz-${GV_VERSION}/build . | xz -9 -c - > ${DIR}/graphviz-${GV_VERSION}-${ARCH}.tar.xz
        fi
    else
        echo "Error: ID=${ID} is unknown" >&2
        exit 1
    fi
fi
