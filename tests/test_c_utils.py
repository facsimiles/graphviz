"""test ../lib/cgraph/ internal generic utility code"""

import os
import platform
import sys
from pathlib import Path

import pytest

sys.path.append(os.path.dirname(__file__))
from gvtest import run_c  # pylint: disable=wrong-import-position


def test_list():
    """run ../lib/cgraph/list.h’s unit tests"""

    # locate the unit tests
    src = Path(__file__).parent.resolve() / "../lib/cgraph/test_list.c"
    assert src.exists()

    # locate lib directory that needs to be in the include path
    lib = Path(__file__).parent.resolve() / "../lib"

    # extra C flags this compilation needs
    cflags = ["-I", lib]
    if platform.system() != "Windows":
        cflags += ["-std=gnu99", "-Wall", "-Wextra", "-Werror"]

    _, _ = run_c(src, cflags=cflags)


@pytest.mark.parametrize("utility", ("bitarray", "tokenize"))
def test_utility(utility: str):
    """run the given utility’s unit tests"""

    # locate the unit tests
    src = Path(__file__).parent.resolve() / f"../lib/util/test_{utility}.c"
    assert src.exists()

    # locate lib directory that needs to be in the include path
    lib = Path(__file__).parent.resolve() / "../lib"

    # extra C flags this compilation needs
    cflags = ["-I", lib]
    if platform.system() != "Windows":
        cflags += ["-std=gnu99", "-Wall", "-Wextra", "-Werror"]

    _, _ = run_c(src, cflags=cflags)


@pytest.mark.parametrize("builtins", (False, True))
def test_overflow_h(builtins: bool):
    """test ../lib/util/overflow.h"""

    # locate the unit test
    src = Path(__file__).parent.resolve() / "../lib/util/test_overflow.c"
    assert src.exists()

    # locate lib directory that needs to be in the include path
    lib = Path(__file__).parent.resolve() / "../lib"

    # extra C flags this compilation needs
    cflags = ["-I", lib]
    if not builtins:
        cflags += ["-DSUPPRESS_BUILTINS"]
    if platform.system() != "Windows":
        cflags += ["-std=gnu99", "-Wall", "-Wextra", "-Werror"]

    run_c(src, cflags=cflags)
