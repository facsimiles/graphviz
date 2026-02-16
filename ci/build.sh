#!/usr/bin/env bash

if [ -z ${CI+x} ]; then
  echo "this script is only intended to run in CI" >&2
  exit 1
fi

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

if [ "${ID}" = "msys2" ]; then
    # MSYS2/MinGW doesn't have VERSION_ID in /etc/os-release
    VERSION_ID=$( uname -r )
fi

# validate we have Git and echo version into log
git --version

GV_VERSION=$(python3 gen_version.py)
META_DATA_DIR=Metadata/${ID}/${VERSION_ID}
mkdir -p ${META_DATA_DIR}
DIR=$(pwd)/Packages/${CI_JOB_NAME}
mkdir -p ${DIR}
ARCH=$( uname -m )
build_system=${build_system:-autotools}
if [ "${build_system}" = "cmake" ]; then
    cmake --version
    mkdir build
    pushd build
    cmake --log-level=VERBOSE --warn-uninitialized -Werror=dev \
      ${CMAKE_OPTIONS:-} ..
    cmake --build . --verbose
    cpack
    popd
    if [ "${ID}" = "ubuntu" ]; then
        mv build/Graphviz-${GV_VERSION}-Linux.deb ${DIR}/${ID}_${VERSION_ID}_graphviz-${GV_VERSION}-cmake.deb
    elif [ "${ID}" = "fedora" -o "${ID}" = "rocky" ]; then
        mv build/Graphviz-${GV_VERSION}-Linux.rpm ${DIR}/${ID}_${VERSION_ID}_graphviz-${GV_VERSION}-cmake.rpm
    elif [ "${ID}" = "Darwin" ]; then
        mv build/Graphviz-${GV_VERSION}-Darwin.zip ${DIR}/${ID}_${VERSION_ID}_Graphviz-${GV_VERSION}-${ID}.zip
    elif [ "${ID}" = "msys2" ]; then
        mv build/Graphviz-${GV_VERSION}-win64.zip ${DIR}/${ID}_${VERSION_ID}_Graphviz-${GV_VERSION}-win64.zip
        mv build/Graphviz-${GV_VERSION}-win64.exe ${DIR}/${ID}_${VERSION_ID}_Graphviz-${GV_VERSION}-win64.exe
    elif [[ ${ID} == CYGWIN* ]]; then
        mv build/Graphviz-${GV_VERSION}-CYGWIN-1.tar.bz2 ${DIR}/${ID}_${VERSION_ID}_Graphviz-${GV_VERSION}-CYGWIN-1.tar.bz2
    else
        echo "Error: ID=${ID} is unknown" >&2
        exit 1
    fi
elif [[ "${CONFIGURE_OPTIONS:-}" =~ "--enable-static" ]]; then
    if [ "${use_autogen:-no}" = "yes" ]; then
        ./autogen.sh
        ./configure --disable-dependency-tracking ${CONFIGURE_OPTIONS:-} --prefix=$( pwd )/build | tee >(./ci/extract-configure-log.sh >${META_DATA_DIR}/configure.log)
        make
        make install
    else
        tar xfz graphviz-${GV_VERSION}.tar.gz
        pushd graphviz-${GV_VERSION}
        ./configure --disable-dependency-tracking $CONFIGURE_OPTIONS --prefix=$( pwd )/build | tee >(../ci/extract-configure-log.sh >../${META_DATA_DIR}/configure.log)
        make
        make install
        popd
    fi
else
    if [ "${ID}" = "ubuntu" ]; then
        tar xfz graphviz-${GV_VERSION}.tar.gz
        (cd graphviz-${GV_VERSION}; fakeroot make -f debian/rules binary) | tee >(ci/extract-configure-log.sh >${META_DATA_DIR}/configure.log)
        tar cf - *.deb *.ddeb | xz -9 -c - >${DIR}/${ID}_${VERSION_ID}_graphviz-${GV_VERSION}-debs.tar.xz
    elif [ "${ID}" = "fedora" -o "${ID}" = "rocky" ]; then
        rm -rf ${HOME}/rpmbuild
        rpmbuild -ta graphviz-${GV_VERSION}.tar.gz | tee >(ci/extract-configure-log.sh >${META_DATA_DIR}/configure.log)
        pushd ${HOME}/rpmbuild/RPMS
        mv */*.rpm ./
        tar cf - *.rpm | xz -9 -c - >${DIR}/${ID}_${VERSION_ID}_graphviz-${GV_VERSION}-rpms.tar.xz
        popd
    elif [ "${ID}" = "Darwin" ]; then
        tar xfz graphviz-${GV_VERSION}.tar.gz
        pushd graphviz-${GV_VERSION}
        ./configure --disable-dependency-tracking --prefix=/usr/local/graphviz --with-quartz=yes
        make pkg
        cp graphviz-${ARCH}.pkg ${DIR}/${ID}_${VERSION_ID}_graphviz-${GV_VERSION}-${ARCH}.pkg
        popd
    elif [[ ${ID} == CYGWIN* || ${ID} == msys* ]]; then
        if [ "${ID}" = "msys2" ]; then
            # ensure that MinGW tcl shell is used in order to find tcl functions
            CONFIGURE_OPTIONS="${CONFIGURE_OPTIONS:-} --with-tclsh=${MSYSTEM_PREFIX}/bin/tclsh86"
        else # Cygwin
            # avoid platform detection problems
            CONFIGURE_OPTIONS="${CONFIGURE_OPTIONS:-} --build=x86_64-pc-cygwin"
        fi
        if [ "${use_autogen:-no}" = "yes" ]; then
            ./autogen.sh
            ./configure --disable-dependency-tracking ${CONFIGURE_OPTIONS:-} --prefix=$( pwd )/build | tee >(./ci/extract-configure-log.sh >${META_DATA_DIR}/configure.log)
            make
            make install
            tar cf - -C build . | xz -9 -c - > ${DIR}/${ID}_${VERSION_ID}_graphviz-${GV_VERSION}-${ARCH}.tar.xz
        else
            tar xfz graphviz-${GV_VERSION}.tar.gz
            pushd graphviz-${GV_VERSION}
            ./configure --disable-dependency-tracking ${CONFIGURE_OPTIONS:-} --prefix=$( pwd )/build | tee >(../ci/extract-configure-log.sh >../${META_DATA_DIR}/configure.log)
            make
            make install
            popd
            tar cf - -C graphviz-${GV_VERSION}/build . | xz -9 -c - > ${DIR}/${ID}_${VERSION_ID}_graphviz-${GV_VERSION}-${ARCH}.tar.xz
        fi
    else
        echo "Error: ID=${ID} is unknown" >&2
        exit 1
    fi
fi
