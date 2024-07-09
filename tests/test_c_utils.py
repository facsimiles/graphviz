"""test ../lib/cgraph/ internal generic utility code"""

import os
import platform
import sys
from pathlib import Path

import pytest

sys.path.append(os.path.dirname(__file__))
from gvtest import compile_c, run, run_c  # pylint: disable=wrong-import-position


@pytest.mark.parametrize("utility", ("bitarray", "itos", "list", "tokenize"))
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
        cflags += ["-std=gnu17"]

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
        cflags += ["-std=gnu17"]

    run_c(src, cflags=cflags)


def test_gv_find_me(tmp_path: Path):
    """test ../lib/util/gv_find_me.c::gv_find_me works correctly"""

    # locate our support test file
    src = Path(__file__).parent.resolve() / "test_gv_find_me.c"
    assert src.exists()

    # locate lib directory that needs to be in the include path
    lib = Path(__file__).resolve().parents[1] / "lib"

    # extra C flags this compilation needs
    cflags = ["-I", lib]
    if platform.system() != "Windows":
        cflags += ["-std=gnu17"]

    # pick a path to compile to
    exe = tmp_path / "test_gv_find_me.exe"

    _ = compile_c(src, cflags, dst=exe)

    # run this
    output = run([exe])

    assert os.path.normpath(output) == os.path.normpath(
        f"{exe}\n"
    ), "gv_find_me did not determine executable absolute path"
