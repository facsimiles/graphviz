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

# Make a lowercase equivalent of `${ID}`. Bash on macOS is 3.2, which does not
# support `${foo,,}`.
id=$(echo "${ID}" | tr '[:upper:]' '[:lower:]')

if [[ ${id} == msys* ]]; then
    # MSYS2/MinGW doesn't have VERSION_ID in /etc/os-release
    VERSION_ID=$( uname -r )
fi

GV_VERSION=$(python3 gen_version.py)
if [ "${CI_JOB_NAME}" = "lint_python" ]; then
  DIR=Packages/ubuntu-24.04-build
elif [ "${CI_JOB_NAME}" = "ubuntu-24.04-cmake-ASan-build-and-test-including-ctest" ]; then
  DIR=Packages/ubuntu-24.04-cmake-ASan-build-and-test-including-ctest
else
  DIR=Packages/${CI_JOB_NAME%-test}-build
fi
ARCH=$( uname -m )

if [ "${build_system}" = "cmake" ]; then
    if [ "${ID_LIKE:-}" = "debian" ]; then
        apt install ./${DIR}/${ID}_${VERSION_ID}_graphviz-${GV_VERSION}-cmake.deb
    elif [ "${ID}" = "Darwin" ]; then
        unzip ${DIR}/${ID}_${VERSION_ID}_Graphviz-${GV_VERSION}-Darwin.zip
        sudo cp -rp Graphviz-${GV_VERSION}-Darwin/* /usr/local
    else
        rpm --install --force -vv ${DIR}/${ID}_${VERSION_ID}_graphviz-${GV_VERSION}-cmake.rpm
    fi
else
    if [ "${ID_LIKE:-}" = "debian" ]; then
        tar xf ${DIR}/${ID}_${VERSION_ID}_graphviz-${GV_VERSION}-debs.tar.xz
        apt install ./libgraphviz4_${GV_VERSION}-1_amd64.deb
        apt install ./libgraphviz-dev_${GV_VERSION}-1_amd64.deb
        apt install ./graphviz_${GV_VERSION}-1_amd64.deb
        apt install ./libgv-python_${GV_VERSION}-1_amd64.deb
        apt install ./libgv-ruby_${GV_VERSION}-1_amd64.deb
        apt install ./libgv-tcl_${GV_VERSION}-1_amd64.deb
    elif [ "${ID}" = "Darwin" ]; then
        sudo installer -verbose -target / -pkg ${DIR}/${ID}_${VERSION_ID}_graphviz-${GV_VERSION}-${ARCH}.pkg
    else
        tar xvf ${DIR}/${ID}_${VERSION_ID}_graphviz-${GV_VERSION}-rpms.tar.xz
        rpm --install --force -vv graphviz-*${GV_VERSION}*.rpm
    fi
fi
