#!/usr/bin/env bash

set -e
set -o pipefail
set -u
set -x

ci/install-packages.sh

if [ -f /etc/os-release ]; then
    cat /etc/os-release
    . /etc/os-release
else
    ID=$( uname -s )
    # remove trailing text after actual version
    VERSION_ID=$( uname -r | sed "s/\([0-9\.]*\).*/\1/")
fi

if [ "${build_system}" = "cmake" ]; then
  if [ "${OSTYPE}" = "linux-gnu" ]; then
    if [ "${ID_LIKE:-}" = "debian" ]; then
      export TCLLIBPATH=/usr/lib/graphviz/tcl
    else
      # Fedora, Rocky
      export TCLLIBPATH=/usr/lib64/graphviz/tcl
    fi
  fi
else
  # Autotools
  if [ "${ID_LIKE:-}" = "debian" ]; then
    export TCLLIBPATH=/usr/lib/x86_64-linux-gnu/graphviz/tcl
  fi
fi

export GV_VERSION=$( cat GRAPHVIZ_VERSION )
python3 -m pytest --strict-markers --verbose --verbose --junit-xml=report.xml \
  ci/tests.py tests
