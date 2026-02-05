"""tests that examples provided with Graphviz compile correctly"""

import os
import platform
import subprocess
import sys
from pathlib import Path

import pytest

sys.path.append(os.path.dirname(__file__))
from gvtest import (  # pylint: disable=wrong-import-position
    is_asan_instrumented,
    is_static_build,
    run,
    run_c,
    run_raw,
    which,
)


@pytest.mark.parametrize(
    "src", ["demo.c", "dot.c", "example.c", "neatopack.c", "simple.c"]
)
@pytest.mark.skipif(
    is_static_build(),
    reason="dynamic libraries are unavailable to link against in static builds",
)
def test_compile_example(src):
    """try to compile the example"""

    # construct an absolute path to the example
    filepath = Path(__file__).parent.resolve() / ".." / "dot.demo" / src

    libs = ["cgraph", "gvc"]

    # run the example
    args = ["-Kneato"] if src in ["demo.c", "dot.c"] else []

    if platform.system() == "Windows":
        cflags = ["-DGVDLL"]
    else:
        cflags = None

    _, _ = run_c(filepath, args, "graph {a -- b}", cflags=cflags, link=libs)


@pytest.mark.parametrize(
    "src",
    [
        "addrings",
        "attr",
        "bbox",
        "bipart",
        "chkedges",
        "clustg",
        "collapse",
        "cycle",
        "deghist",
        "delmulti",
        "depath",
        "flatten",
        "group",
        "indent",
        "path",
        "scale",
        "span",
        "treetoclust",
        "addranks",
        "anon",
        "bb",
        "chkclusters",
        "cliptree",
        "col",
        "color",
        "dechain",
        "deledges",
        "delnodes",
        "dijkstra",
        "get-layers-list",
        "knbhd",
        "maxdeg",
        "rotate",
        "scalexy",
        "topon",
    ],
)
@pytest.mark.skipif(which("gvpr") is None, reason="GVPR not available")
def test_gvpr_example(src):
    """check GVPR can parse the given example"""

    # construct a relative path to the example because gvpr on Windows does not
    # support absolute paths (#1780)
    path = Path("cmd/gvpr/lib") / src
    wd = Path(__file__).parent.parent.resolve()

    # run GVPR with the given script
    run_raw(["gvpr", "-f", path], stdin=subprocess.DEVNULL, cwd=wd)


@pytest.mark.skipif(which("gvpr") is None, reason="GVPR not available")
@pytest.mark.xfail(
    which("dot") is not None
    and is_asan_instrumented(which("dot"))
    and platform.system() != "Windows",
    reason="memory leaks",
    strict=True,
)
def test_gvpr_clustg():
    """check cmd/gvpr/lib/clustg works"""

    # construct a relative path to clustg because gvpr on Windows does not
    # support absolute paths (#1780)
    path = Path("cmd/gvpr/lib/clustg")
    wd = Path(__file__).parent.parent.resolve()

    # some sample input
    input = "digraph { N1; N2; N1 -> N2; N3; }"

    # run GVPR on this input
    output = run(["gvpr", "-f", path], input=input, cwd=wd)

    assert (
        output.strip() == 'strict digraph "clust%1" {\n'
        "\tnode [_cnt=0];\n"
        "\tedge [_cnt=0];\n"
        "\tN1 -> N2\t[_cnt=1];\n"
        "\tN3;\n"
        "}"
    ), "unexpected output"
