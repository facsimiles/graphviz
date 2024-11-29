#!/usr/bin/env bash

set -e
set -o pipefail
set -u
set -x

ci/mingw-install.sh
python3 -m pip install --requirement requirements.txt

export PATH=$PATH:/c/Git/cmd

# we need the absolute path since pytest cd somewhere else

# we need the logical value of the directory for the PATH
DIR_LABS="/c/Graphviz"

# needed to find headers and libs at compile time. Must use absolute
# Windows path for libs (why?)
export CFLAGS="-I$DIR_LABS/include"
export LDFLAGS="-L$DIR_LABS/lib"

# make TCL packages visible for importing
export TCLLIBPATH="$DIR_LABS/lib/graphviz/tcl"

# needed to find e.g. libgvc.dll at run time. Windows does not use
# LD_LIBRARY_PATH. Must be the logical directory
export PATH="${PATH}:$DIR_LABS/bin"

python gen_version.py --output GRAPHVIZ_VERSION
export GV_VERSION=$( cat GRAPHVIZ_VERSION )

python3 -m pytest -m "not slow" ci/tests.py tests
