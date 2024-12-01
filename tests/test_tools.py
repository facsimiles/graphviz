"""
Graphviz tools tests

The test cases in this file are sanity checks on the various tools in the
Graphviz suite. A failure of one of these indicates that some basic functional
property of one of the tools has been broken.
"""

import os
import platform
import re
import subprocess
import sys
import xml.etree.ElementTree as ET
from pathlib import Path

import pytest

sys.path.append(os.path.dirname(__file__))
from gvtest import (  # pylint: disable=wrong-import-position
    has_sandbox,
    remove_xtype_warnings,
    which,
)


@pytest.mark.parametrize(
    "tool",
    [
        "acyclic",
        "bcomps",
        "ccomps",
        "circo",
        "cluster",
        "diffimg",
        "dijkstra",
        "dot",
        "dot2gxl",
        "dot_builtins",
        "edgepaint",
        "fdp",
        "gc",
        "gml2gv",
        "graphml2gv",
        "gv2gml",
        "gv2gxl",
        "gvcolor",
        "gvedit",
        "gvgen",
        "gvmap",
        "gvmap.sh",
        "gvpack",
        "gvpr",
        "gxl2dot",
        "gxl2gv",
        "mingle",
        "mm2gv",
        "neato",
        "nop",
        "osage",
        "patchwork",
        "prune",
        "sccmap",
        "sfdp",
        "smyrna",
        "tred",
        "twopi",
        "unflatten",
        "vimdot",
    ],
)
def test_tools(tool):
    """
    check the help/usage output of the given tool looks correct
    """

    if which(tool) is None:
        pytest.skip(f"{tool} not available")

    # exec-ing a POSIX shell script as-is does not work on Windows
    if tool == "gvmap.sh" and platform.system() == "Windows":
        pytest.skip("gvmap.sh cannot be run directly on Windows")

    # Ensure that X fails to open display
    environ_copy = os.environ.copy()
    environ_copy.pop("DISPLAY", None)

    # Test usage
    with subprocess.Popen(
        [tool, "-?"],
        env=environ_copy,
        stdin=subprocess.DEVNULL,
        stdout=subprocess.PIPE,
        stderr=subprocess.STDOUT,
        universal_newlines=True,
    ) as p:
        output, _ = p.communicate()
        ret = p.returncode

    assert ret == 0, f"`{tool} -?` failed. Output was: {output}"

    output = remove_xtype_warnings(output)
    assert (
        re.match("usage", output, flags=re.IGNORECASE) is not None
    ), f"{tool} -? did not show usage. Output was: {output}"

    # Test unsupported option
    returncode = subprocess.call(
        [tool, "-$"],
        env=environ_copy,
    )

    assert returncode != 0, f"{tool} accepted unsupported option -$"


@pytest.mark.skipif(which("edgepaint") is None, reason="edgepaint not available")
@pytest.mark.parametrize(
    "arg",
    (
        "-accuracy=0.01",
        "-angle=15",
        "-random_seed=42",
        "-random_seed=-42",
        "-lightness=0,70",
        "-share_endpoint",
        f"-o {os.devnull}",
        "-color_scheme=accent7",
        "-v",
    ),
)
def test_edgepaint_options(arg: str):
    """
    edgepaint should correctly understand all its command line flags
    """

    # a basic graph that edgepaint can process
    input = (
        "digraph {\n"
        '  graph [bb="0,0,54,108"];\n'
        '  node [label="\\N"];\n'
        "  a       [height=0.5,\n"
        '           pos="27,90",\n'
        "           width=0.75];\n"
        "  b       [height=0.5,\n"
        '           pos="27,18",\n'
        "           width=0.75];\n"
        '  a -> b  [pos="e,27,36.104 27,71.697 27,63.983 27,54.712 27,46.112"];\n'
        "}"
    )

    # run edgepaint on this
    args = ["edgepaint"] + arg.split(" ")
    try:
        subprocess.run(args, input=input, check=True, universal_newlines=True)
    except subprocess.CalledProcessError as e:
        raise RuntimeError(f"edgepaint rejected command line option '{arg}'") from e


@pytest.mark.skipif(not has_sandbox(), reason="no supported sandbox available")
def test_sandbox_noop():
    """check trivial functionality works when sandboxed"""
    sandbox = which("dot_sandbox")
    subprocess.check_call([sandbox, "-V"])


@pytest.mark.skipif(not has_sandbox(), reason="no supported sandbox available")
def test_sandbox_basic():
    """check processing a simple graph when sandboxed"""
    sandbox = which("dot_sandbox")
    subprocess.run(
        [sandbox], input="graph { a -- b; }", universal_newlines=True, check=True
    )


@pytest.mark.skipif(not has_sandbox(), reason="no supported sandbox available")
def test_sandbox_render():
    """check rendering works when sandboxed"""
    sandbox = which("dot_sandbox")
    proc = subprocess.run(
        [sandbox, "-Tsvg"],
        stdout=subprocess.PIPE,
        input="graph { a -- b; }",
        universal_newlines=True,
        check=True,
    )
    ET.fromstring(proc.stdout)


@pytest.mark.skipif(not has_sandbox(), reason="no supported sandbox available")
def test_sandbox_write(tmp_path: Path):
    """check file writing is prevented when sandboxed"""
    sandbox = which("dot_sandbox")
    proc = subprocess.run(
        [sandbox, "-o", "probe.dot"],
        input="graph { a -- b; }",
        universal_newlines=True,
        cwd=tmp_path,
        check=False,
    )
    assert not (
        tmp_path / "probe.dot"
    ).exists(), "file writing within sandbox was not prevented"
    assert (
        proc.returncode != 0
    ), "failed file writing did not cause a non-zero exit status"


@pytest.mark.skipif(not has_sandbox(), reason="no supported sandbox available")
def test_sandbox_write_2(tmp_path: Path):
    """check file writing via `-O` is prevented when sandboxed"""
    sandbox = which("dot_sandbox")
    proc = subprocess.run(
        [sandbox, "-Tsvg", "-O"],
        input="graph { a -- b; }",
        universal_newlines=True,
        cwd=tmp_path,
        check=False,
    )
    assert not (
        tmp_path / "noname.gv.svg"
    ).exists(), "file writing within sandbox was not prevented"
    assert (
        proc.returncode != 0
    ), "failed file writing did not cause a non-zero exit status"
