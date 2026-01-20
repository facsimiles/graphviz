"""test ../lib/cgraph/ internal generic utility code"""

import os
import platform
import subprocess
import sys
from pathlib import Path

import pytest

sys.path.append(os.path.dirname(__file__))
from gvtest import (  # pylint: disable=wrong-import-position
    compile_c,
    has_cflag,
    is_macos,
    is_mingw,
    is_rocky_8,
    run,
    run_c,
)


@pytest.mark.parametrize("threads", ("C11 threads", "pthreads", "no threads"))
@pytest.mark.parametrize("cas", ("FORCE_CAS=1", "FORCE_CAS=0", "FORCE_CAS=auto"))
def test_dword(threads: str, cas: str):
    """
    run the unit tests of ../lib/util/dword.c

    Args:
        threads: What library to back multi-threading with
        cas: Whether/how to force the CAS workaround in dword.c
    """

    # skip inapplicable combinations
    if threads == "C11 threads":
        if is_macos():
            pytest.skip("macOS does not support C11 threads")
        if is_mingw():
            pytest.skip("MinGW does not support C11 threads")
        if (
            platform.system() == "Windows"
            and os.environ.get("configuration") == "Debug"
            and os.environ.get("project_platform") == "Win32"
        ):
            pytest.skip("C11 threads do not work in 32-bit Windows Debug builds")
    if threads == "pthreads":
        if platform.system() == "Windows" and not is_mingw():
            pytest.skip("Windows does not support pthreads")
    if cas == "FORCE_CAS=1":
        if is_macos():
            pytest.skip("Clang does not support __sync built-ins on _Atomic types")
        # libatomic on MinGW was built without support for 128-bit types
        # https://github.com/msys2/MINGW-packages/issues/13831
        if is_mingw():
            pytest.skip("MinGW does not support out-of-line 128-bit ops")
        if platform.system() == "Windows":
            pytest.skip("MSVC does not support __sync built-ins")

    # locate the unit tests
    src = Path(__file__).parent.resolve() / "../lib/util/test_dword.c"
    assert src.exists()

    # locate lib directory that needs to be in the include path
    lib = Path(__file__).parent.resolve() / "../lib"

    # extra C flags this compilation needs
    cflags = ["-I", lib]

    if threads == "C11 threads":
        cflags += ["-DUSE_C11_THREADS"]
        # Rocky 8’s compiler (GCC 8.5.0) seems to back C11 threads with pthreads, so we
        # need that too
        if is_rocky_8():
            cflags += ["-pthread"]
    elif threads == "pthreads":
        cflags += ["-pthread", "-DUSE_PTHREADS"]

    if cas == "FORCE_CAS=1":
        cflags += ["-DFORCE_CAS=1"]
    elif cas == "FORCE_CAS=0":
        cflags += ["-DFORCE_CAS=0"]

    # if this platform supports `-mcx16`, conservatively assumed we need it
    if has_cflag("-mcx16"):
        cflags += ["-mcx16"]

    if platform.system() == "Windows" and not is_mingw():
        cflags += ["/volatile:ms"]

    # if this platform supports libatomic, conservatively assume we need it
    link = []
    if platform.system() != "Windows" or is_mingw():
        cc = os.environ.get("CC", "cc")
        try:
            libatomic = run([cc, "-print-file-name=libatomic.so"])
        except subprocess.CalledProcessError:
            libatomic = ""
        if Path(libatomic).is_absolute():
            link += [libatomic.strip()]

    run_c(src, cflags=cflags, link=link)


@pytest.mark.parametrize("utility", ("arena", "bitarray", "itos", "list", "tokenize"))
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


@pytest.mark.parametrize(
    "bad_test",
    (
        1,
        2,
        3,
        4,
        pytest.param(
            5,
            marks=pytest.mark.skipif(
                platform.system() == "Windows" and not is_mingw(),
                reason="this test has not been ported to Windows",
            ),
        ),
        pytest.param(
            6,
            marks=pytest.mark.skipif(
                platform.system() == "Windows" and not is_mingw(),
                reason="this test has not been ported to Windows",
            ),
        ),
    ),
)
def test_list_type_safetyutility(bad_test: int):
    """
    check the type safety of ../lib/util/list.h’s interfaces

    Args:
        bad_test: Which type-violating test in ../lib/util/test_list.c to compile.
    """

    # locate the unit tests
    src = Path(__file__).parent.resolve() / "../lib/util/test_list.c"
    assert src.exists()

    # locate lib directory that needs to be in the include path
    lib = Path(__file__).parent.resolve() / "../lib"

    # extra C flags this compilation needs
    cflags = [f"-DBAD_TEST={bad_test}", "-I", lib]
    if platform.system() != "Windows":
        cflags += ["-std=gnu17"]

    # this should fail to compile if strong type safety is preserved
    with pytest.raises(subprocess.CalledProcessError):
        compile_c(src, cflags=cflags)


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

    assert output == f"{exe}\n", "gv_find_me did not determine executable absolute path"
