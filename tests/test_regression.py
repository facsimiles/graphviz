"""
Graphviz regression tests

The test cases in this file relate to previously observed bugs. A failure of one
of these indicates that a past bug has been reintroduced.
"""

import dataclasses
import io
import json
import math
import os
import platform
import re
import shlex
import shutil
import signal
import stat
import statistics
import subprocess
import sys
import tempfile
import textwrap
import time
import xml.etree.ElementTree as ET
from pathlib import Path
from typing import Iterator, List, Set

import pexpect
import pytest

sys.path.append(os.path.dirname(__file__))
from gvtest import (  # pylint: disable=wrong-import-position
    dot,
    gvpr,
    is_autotools,
    is_cmake,
    is_macos,
    is_mingw,
    is_rocky,
    is_rocky_8,
    is_static_build,
    is_ubuntu_2004,
    remove_xtype_warnings,
    run_c,
    which,
)


def is_ndebug_defined() -> bool:
    """
    are assertions disabled in the Graphviz build under test?
    """

    # the Windows release builds set NDEBUG
    if os.environ.get("configuration") == "Release":
        return True

    return False


@pytest.mark.xfail(
    platform.system() == "Windows", reason="#56", strict=not is_ndebug_defined()
)  # FIXME
def test_14():
    """
    using ortho and twopi in combination should not cause an assertion failure
    https://gitlab.com/graphviz/graphviz/-/issues/14
    """

    # locate our associated test case in this directory
    input = Path(__file__).parent / "14.dot"
    assert input.exists(), "unexpectedly missing test case"

    # process it with Graphviz
    dot("svg", input)


@pytest.mark.skipif(which("neato") is None, reason="neato not available")
def test_42():
    """
    check for a former crash in neatogen
    https://gitlab.com/graphviz/graphviz/-/issues/42
    """

    # locate our associated test case in this directory
    input = Path(__file__).parent / "42.dot"
    assert input.exists(), "unexpectedly missing test case"

    # process it with Graphviz
    subprocess.check_call(["neato", "-n2", "-Tpng", input], stdout=subprocess.DEVNULL)


def test_56():
    """
    parsing a particular graph should not cause a Trapezoid-table overflow
    assertion failure
    https://gitlab.com/graphviz/graphviz/-/issues/56
    """

    # locate our associated test case in this directory
    input = Path(__file__).parent / "56.dot"
    assert input.exists(), "unexpectedly missing test case"

    # process it with Graphviz
    dot("svg", input)


def test_121():
    """
    test a graph that previously caused an assertion failure in `merge_chain`
    https://gitlab.com/graphviz/graphviz/-/issues/121
    """

    # locate our associated test case in this directory
    input = Path(__file__).parent / "121.dot"
    assert input.exists(), "unexpectedly missing test case"

    # process it with Graphviz
    dot("pdf", input)


def test_131():
    """
    PIC back end should produce valid output
    https://gitlab.com/graphviz/graphviz/-/issues/131
    """

    # a basic graph
    src = "digraph { a -> b; c -> d; }"

    # ask Graphviz to process this to PIC
    pic = dot("pic", source=src)

    if which("gpic") is None:
        pytest.skip("GNU PIC not available")

    # ask GNU PIC to process the Graphviz output
    subprocess.run(
        ["gpic"],
        input=pic,
        stdout=subprocess.DEVNULL,
        check=True,
        universal_newlines=True,
    )


@pytest.mark.parametrize(
    "testcase",
    (
        "144_no_ortho.dot",
        pytest.param(
            "144_ortho.dot",
            marks=pytest.mark.xfail(platform.system() == "Windows", reason="flaky"),
        ),
    ),
)  # FIXME
def test_144(testcase: str):
    """
    using ortho should not result in head/tail confusion
    https://gitlab.com/graphviz/graphviz/-/issues/144
    """

    # locate our associated test cases in this directory
    input = Path(__file__).parent / testcase
    assert input.exists(), "unexpectedly missing test case"

    # process the non-ortho one into JSON
    out = dot("json", input)
    data = json.loads(out)

    # find the nodes “A”, “B” and “C”
    A = [x for x in data["objects"] if x["name"] == "A"][0]
    B = [x for x in data["objects"] if x["name"] == "B"][0]
    C = [x for x in data["objects"] if x["name"] == "C"][0]

    # find the straight A→B and the angular A→C edges
    straight_edge = [
        x for x in data["edges"] if x["tail"] == A["_gvid"] and x["head"] == B["_gvid"]
    ][0]
    angular_edge = [
        x for x in data["edges"] if x["tail"] == A["_gvid"] and x["head"] == C["_gvid"]
    ][0]

    # the A→B edge should have been routed vertically down
    straight_points = straight_edge["_draw_"][1]["points"]
    xs = [x for x, _ in straight_points]
    ys = [y for _, y in straight_points]
    assert all(x == xs[0] for x in xs), "A->B not routed vertically"
    assert ys == sorted(ys, reverse=True), "A->B is not routed down"

    # determine Graphviz’ idea of head and tail ends
    straight_head_point = straight_edge["_hdraw_"][3]["points"][0]
    straight_tail_point = straight_edge["_tdraw_"][3]["points"][0]
    assert straight_head_point[1] < straight_tail_point[1], "A->B head/tail confusion"

    # the A→C edge should have been routed in zigzag down and right
    angular_points = angular_edge["_draw_"][1]["points"]
    xs = [x for x, _ in angular_points]
    ys = [y for _, y in angular_points]
    assert xs == sorted(xs), "A->B is not routed down"
    assert ys == sorted(ys, reverse=True), "A->B is not routed right"

    # determine Graphviz’ idea of head and tail ends
    angular_head_point = angular_edge["_hdraw_"][3]["points"][0]
    angular_tail_point = angular_edge["_tdraw_"][3]["points"][0]
    assert angular_head_point[0] > angular_tail_point[0], "A->C head/tail confusion"


def test_146():
    """
    dot should respect an alpha channel value of 0 when writing SVG
    https://gitlab.com/graphviz/graphviz/-/issues/146
    """

    # a graph using white text but with 0 alpha
    source = (
        "graph {\n"
        '  n[style="filled", fontcolor="#FFFFFF00", label="hello world"];\n'
        "}"
    )

    # ask Graphviz to process this
    svg = dot("svg", source=source)

    # the SVG should be setting opacity
    opacity = re.search(r'\bfill-opacity="(\d+(\.\d+)?)"', svg)
    assert opacity is not None, "transparency not set for alpha=0 color"

    # it should be zeroed
    assert (
        float(opacity.group(1)) == 0
    ), "alpha=0 color set to something non-transparent"


def test_165():
    """
    dot should be able to produce properly escaped xdot output
    https://gitlab.com/graphviz/graphviz/-/issues/165
    """

    # locate our associated test case in this directory
    input = Path(__file__).parent / "165.dot"
    assert input.exists(), "unexpectedly missing test case"

    # ask Graphviz to translate it to xdot
    output = dot("xdot", input)

    # find the line containing the _ldraw_ attribute
    ldraw = re.search(r"^\s*_ldraw_\s*=(?P<value>.*?)$", output, re.MULTILINE)
    assert ldraw is not None, "no _ldraw_ attribute in graph"

    # this should contain the label correctly escaped
    assert r"hello \\\" world" in ldraw.group("value"), "unexpected ldraw contents"


def test_165_2():
    """
    variant of test_165() that checks a similar problem for edges
    https://gitlab.com/graphviz/graphviz/-/issues/165
    """

    # locate our associated test case in this directory
    input = Path(__file__).parent / "165_2.dot"
    assert input.exists(), "unexpectedly missing test case"

    # ask Graphviz to translate it to xdot
    output = dot("xdot", input)

    # find the lines containing _ldraw_ attributes
    ldraw = re.findall(r"^\s*_ldraw_\s*=(.*?)$", output, re.MULTILINE)
    assert ldraw is not None, "no _ldraw_ attributes in graph"

    # one of these should contain the label correctly escaped
    assert any(r"hello \\\" world" in l for l in ldraw), "unexpected ldraw contents"


def test_165_3():
    """
    variant of test_165() that checks a similar problem for graph labels
    https://gitlab.com/graphviz/graphviz/-/issues/165
    """

    # locate our associated test case in this directory
    input = Path(__file__).parent / "165_3.dot"
    assert input.exists(), "unexpectedly missing test case"

    # ask Graphviz to translate it to xdot
    output = dot("xdot", input)

    # find the lines containing _ldraw_ attributes
    ldraw = re.findall(r"^\s*_ldraw_\s*=(.*?)$", output, re.MULTILINE)
    assert ldraw is not None, "no _ldraw_ attributes in graph"

    # one of these should contain the label correctly escaped
    assert any(r"hello \\\" world" in l for l in ldraw), "unexpected ldraw contents"


def test_167():
    """
    using concentrate=true should not result in a segfault
    https://gitlab.com/graphviz/graphviz/-/issues/167
    """

    # locate our associated test case in this directory
    input = Path(__file__).parent / "167.dot"
    assert input.exists(), "unexpectedly missing test case"

    # process this with dot
    ret = subprocess.call(["dot", "-Tpdf", "-o", os.devnull, input])

    # Graphviz should not have caused a segfault
    assert ret != -signal.SIGSEGV, "Graphviz segfaulted"


def test_191():
    """
    a comma-separated list without quotes should cause a hard error, not a warning
    https://gitlab.com/graphviz/graphviz/-/issues/191
    """

    source = (
        "graph {\n"
        '  "Trackable" [fontcolor=grey45,labelloc=c,fontname=Vera Sans, '
        "DejaVu Sans, Liberation Sans, Arial, Helvetica, sans,shape=box,"
        'height=0.3,align=center,fontsize=10,style="setlinewidth(0.5)"];\n'
        "}"
    )

    with subprocess.Popen(
        ["dot", "-Tdot"],
        stdin=subprocess.PIPE,
        stderr=subprocess.PIPE,
        universal_newlines=True,
    ) as p:
        _, stderr = p.communicate(source)

        assert "syntax error" in stderr, "missing error message for unquoted list"

        assert p.returncode != 0, "syntax error was only a warning, not an error"


def test_218():
    """
    out-of-spec font names should cause warnings in the core PS renderer
    https://gitlab.com/graphviz/graphviz/-/issues/218
    """

    # a graph using a font name with a space in it
    source = 'graph { a[fontname="PT Sans"]; }'

    # render it to PS
    warnings = subprocess.check_output(
        ["dot", "-Tps", "-o", os.devnull],
        stderr=subprocess.STDOUT,
        input=source,
        universal_newlines=True,
    )

    assert warnings.strip() != "", "no warning issued for a font name containing space"


@pytest.mark.parametrize("test_case", ("241_0.dot", "241_1.dot"))
@pytest.mark.xfail(
    strict=True, reason="https://gitlab.com/graphviz/graphviz/-/issues/241"
)
def test_241(test_case: str):
    """
    processing a graph with a `splines=…` setting should not causes warnings
    https://gitlab.com/graphviz/graphviz/-/issues/241
    """
    # locate our associated test case in this directory
    input = Path(__file__).parent / test_case
    assert input.exists(), "unexpectedly missing test case"

    proc = subprocess.run(
        ["dot", "-Tsvg", "-o", os.devnull, input],
        stderr=subprocess.PIPE,
        universal_newlines=True,
        check=True,
    )

    assert (
        "Something is probably seriously wrong" not in proc.stderr
    ), "splines setting caused warnings"


def test_358():
    """
    setting xdot version to 1.7 should enable font characteristics
    https://gitlab.com/graphviz/graphviz/-/issues/358
    """

    # locate our associated test case in this directory
    input = Path(__file__).parent / "358.dot"
    assert input.exists(), "unexpectedly missing test case"

    # process this with dot
    xdot = dot("xdot", input)

    for i in range(6):
        m = re.search(f"\\bt {1 << i}\\b", xdot)
        assert m is not None, f"font characteristic {1 << i} not enabled in xdot 1.7"


@pytest.mark.parametrize("attribute", ("samehead", "sametail"))
def test_452(attribute: str):
    """
    more than 5 unique `samehead` and `sametail` values should be usable
    https://gitlab.com/graphviz/graphviz/-/issues/452
    """

    # a graph using more than 5 of the same attribute with the same node on one
    # side of each edge
    graph = io.StringIO()
    graph.write("digraph {\n")
    for i in range(6):
        if attribute == "samehead":
            graph.write(f"  m{i} -> n1")
        else:
            graph.write(f"  n1 -> m{i}")
        graph.write(f'[{attribute}="foo{i}"];\n')
    graph.write("}\n")

    # process this with dot
    dot("svg", source=graph.getvalue())


def test_510():
    """
    HSV colors should also support an alpha channel
    https://gitlab.com/graphviz/graphviz/-/issues/510
    """

    # a graph with a turquoise, partially transparent node
    source = 'digraph { a [color="0.482 0.714 0.878 0.5"]; }'

    # process this with dot
    svg = dot("svg", source=source)

    # see if we can locate an opacity adjustment
    m = re.search(r'\bstroke-opacity="(?P<opacity>\d*.\d*)"', svg)
    assert m is not None, "no stroke-opacity set; alpha channel ignored?"

    # it should be something in-between transparent and opaque
    opacity = float(m.group("opacity"))
    assert opacity > 0, "node set transparent; misinterpreted alpha channel?"
    assert opacity < 1, "node set opaque; misinterpreted alpha channel?"


@pytest.mark.skipif(
    which("gv2gxl") is None or which("gxl2gv") is None, reason="GXL tools not available"
)
def test_517():
    """
    round tripping a graph through gv2gxl should not lose HTML labels
    https://gitlab.com/graphviz/graphviz/-/issues/517
    """

    # our test case input
    input = (
        "digraph{\n"
        "  A[label=<<TABLE><TR><TD>(</TD><TD>A</TD><TD>)</TD></TR></TABLE>>]\n"
        '  B[label="<TABLE><TR><TD>(</TD><TD>B</TD><TD>)</TD></TR></TABLE>"]\n'
        "}"
    )

    # translate it to GXL
    gv2gxl = which("gv2gxl")
    gxl = subprocess.check_output([gv2gxl], input=input, universal_newlines=True)

    # translate this back to Dot
    gxl2gv = which("gxl2gv")
    dot_output = subprocess.check_output([gxl2gv], input=gxl, universal_newlines=True)

    # the result should have both expected labels somewhere
    assert (
        "label=<<TABLE><TR><TD>(</TD><TD>A</TD><TD>)</TD></TR></TABLE>>" in dot_output
    ), "HTML label missing"
    assert (
        'label="<TABLE><TR><TD>(</TD><TD>B</TD><TD>)</TD></TR></TABLE>"' in dot_output
    ), "regular label missing"


def test_793():
    """
    Graphviz should not crash when using VRML output with a non-writable current
    directory
    https://gitlab.com/graphviz/graphviz/-/issues/793
    """

    # create a non-writable directory
    with tempfile.TemporaryDirectory() as tmp:
        t = Path(tmp)
        t.chmod(t.stat().st_mode & ~stat.S_IWRITE)

        # ask the VRML back end to handle a simple graph, using the above as the
        # current working directory
        with subprocess.Popen(["dot", "-Tvrml", "-o", os.devnull], cwd=t) as p:
            p.communicate("digraph { a -> b; }")

            # Graphviz should not have caused a segfault
            assert p.returncode != -signal.SIGSEGV, "Graphviz segfaulted"


def test_797():
    """
    “&;” should not be considered an XML escape sequence
    https://gitlab.com/graphviz/graphviz/-/issues/797
    """

    # some input containing the invalid escape
    input = 'digraph tree {\n"1" [shape="box", label="&amp; &amp;;", URL="a"];\n}'

    # process this with the client-side imagemap back end
    output = dot("cmapx", source=input)

    # the escape sequences should have been preserved
    assert "&amp; &amp;" in output


@pytest.mark.xfail(
    strict=True, reason="https://gitlab.com/graphviz/graphviz/-/issues/813"
)
def test_813():
    """
    nodes with multiple peripheries should still have a stable rendering
    https://gitlab.com/graphviz/graphviz/-/issues/813
    """

    # locate our associated test case in this directory
    input = Path(__file__).parent / "813.dot"
    assert input.exists(), "unexpectedly mising test case"

    # render this to dot
    reference = dot("dot", input)

    # run it through multiple passes
    iterated = reference
    for _ in range(4):
        iterated = dot("dot", source=iterated)

    assert (
        reference == iterated
    ), "rendering of shapes with multiple peripheries is unstable"


def test_827():
    """
    Graphviz should not crash when processing the b15.gv example
    https://gitlab.com/graphviz/graphviz/-/issues/827
    """

    b15gv = Path(__file__).parent / "graphs/b15.gv"
    assert b15gv.exists(), "missing test case file"

    dot("svg", b15gv)


def test_925():
    """
    spaces should be handled correctly in UTF-8-containing labels in record shapes
    https://gitlab.com/graphviz/graphviz/-/issues/925
    """

    # locate our associated test case in this directory
    input = Path(__file__).parent / "925.dot"
    assert input.exists(), "unexpectedly mising test case"

    # process this with dot
    svg = dot("svg", input)

    # The output should include the correctly spaced UTF-8 label. Note that these
    # are not ASCII capital As in this string, but rather UTF-8 Cyrillic Capital
    # Letter As.
    assert "ААА ААА ААА" in svg, "incorrect spacing in UTF-8 label"


@pytest.mark.parametrize("testcase", ("1213-1.dot", "1213-2.dot"))
@pytest.mark.xfail(
    strict=True, reason="https://gitlab.com/graphviz/graphviz/-/issues/1213"
)
def test_1213(testcase: str):
    """
    clustering should not trigger “trouble in init_rank” errors
    https://gitlab.com/graphviz/graphviz/-/issues/1213
    """

    # locate our associated test case in this directory
    input = Path(__file__).parent / testcase
    assert input.exists(), "unexpectedly mising test case"

    # process this with dot
    dot("png", input)


def test_1221():
    """
    assigning a node to two clusters with newrank should not cause a crash
    https://gitlab.com/graphviz/graphviz/-/issues/1221
    """

    # locate our associated test case in this directory
    input = Path(__file__).parent / "1221.dot"
    assert input.exists(), "unexpectedly missing test case"

    # process this with dot
    dot("svg", input)


@pytest.mark.skipif(which("gv2gml") is None, reason="gv2gml not available")
def test_1276():
    """
    quotes within a label should be escaped in translation to GML
    https://gitlab.com/graphviz/graphviz/-/issues/1276
    """

    # DOT input containing a label with quotes
    src = 'digraph test {\n  x[label=<"Label">];\n}'

    # process this to GML
    gml = subprocess.check_output(["gv2gml"], input=src, universal_newlines=True)

    # the unescaped label should not appear in the output
    assert '""Label""' not in gml, "quotes not escaped in label"

    # the escaped label should appear in the output
    assert (
        '"&quot;Label&quot;"' in gml or '"&#34;Label&#34;"' in gml
    ), "escaped label not found in GML output"


def test_1308():
    """
    processing a minimized graph found by Google Autofuzz should not crash
    https://gitlab.com/graphviz/graphviz/-/issues/1308
    """

    # locate our associated test case in this directory
    input = Path(__file__).parent / "1308.dot"
    assert input.exists(), "unexpectedly missing test case"

    # run it through Graphviz
    dot("svg", input)


def test_1308_1():
    """
    processing a malformed graph found by Google Autofuzz should not crash
    https://gitlab.com/graphviz/graphviz/-/issues/1308
    """

    # locate our associated test case in this directory
    input = Path(__file__).parent / "1308_1.dot"
    assert input.exists(), "unexpectedly missing test case"

    # run it through Graphviz
    ret = subprocess.call(["dot", "-Tsvg", "-o", os.devnull, input])

    assert ret in (0, 1), "Graphviz crashed when processing malformed input"
    assert ret == 1, "Graphviz did not reject malformed input"


def test_1314():
    """
    test that a large font size that produces an overflow in Pango is rejected
    https://gitlab.com/graphviz/graphviz/-/issues/1314
    """

    # locate our associated test case in this directory
    input = Path(__file__).parent / "1314.dot"
    assert input.exists(), "unexpectedly missing test case"

    # ask Graphviz to process it, which should fail
    try:
        dot("svg", input)
    except subprocess.CalledProcessError:
        return

    # the execution did not fail as expected
    pytest.fail("dot incorrectly exited with success")


def test_1318():
    """
    processing a large number in a comment should not trigger integer overflow
    https://gitlab.com/graphviz/graphviz/-/issues/1318
    """

    # sample input consisting of a large number in a comment
    source = "#8828066547613302784"

    # processing this should succeed
    dot("svg", source=source)


@pytest.mark.xfail(
    strict=True, reason="https://gitlab.com/graphviz/graphviz/-/issues/1328"
)
def test_1328():
    """
    a node with conflicting rank constraints should not cause a crash
    https://gitlab.com/graphviz/graphviz/-/issues/1328
    """

    # locate our associated test case in this directory
    input = Path(__file__).parent / "1328.dot"
    assert input.exists(), "unexpectedly missing test case"

    proc = subprocess.run(
        ["dot", "-Tsvg", "-o", os.devnull, input],
        stderr=subprocess.PIPE,
        universal_newlines=True,
    )

    assert proc.returncode in (0, 1), "multiple rank constraints caused a crash"
    assert (
        "trouble in init_rank" not in proc.stderr
    ), "multiple rank constraints caused ranking failure"


def test_1332():
    """
    Triangulation calculation on the associated example should succeed.

    A prior change that was intended to increase accuracy resulted in the
    example in this test now failing some triangulation calculations. It is not
    clear whether the outcome before or after is correct, but this test ensures
    that the older behavior users are accustomed to is preserved.
    """

    # locate our associated test case in this directory
    input = Path(__file__).parent / "1332.dot"
    assert input.exists(), "unexpectedly missing test case"

    # process it with Graphviz
    warnings = subprocess.check_output(
        ["dot", "-Tpdf", "-o", os.devnull, input],
        stderr=subprocess.STDOUT,
        universal_newlines=True,
    )

    # work around macOS warnings
    warnings = remove_xtype_warnings(warnings).strip()

    # no warnings should have been printed
    assert (
        warnings == ""
    ), "warnings were printed when processing graph involving triangulation"


def test_1408():
    """
    parsing particular ortho layouts should not cause an assertion failure
    https://gitlab.com/graphviz/graphviz/-/issues/1408
    """

    # locate our associated test case in this directory
    input = Path(__file__).parent / "1408.dot"
    assert input.exists(), "unexpectedly missing test case"

    # process it with Graphviz
    dot("svg", input)


def test_1411():
    """
    parsing strings containing newlines should not disrupt line number tracking
    https://gitlab.com/graphviz/graphviz/-/issues/1411
    """

    # locate our associated test case in this directory
    input = Path(__file__).parent / "1411.dot"
    assert input.exists(), "unexpectedly missing test case"

    # process it with Graphviz (should fail)
    with subprocess.Popen(
        ["dot", "-Tsvg", "-o", os.devnull, input],
        stderr=subprocess.PIPE,
        universal_newlines=True,
    ) as p:
        _, output = p.communicate()

        assert p.returncode != 0, "Graphviz accepted broken input"

    assert (
        "syntax error in line 17 near '\\'" in output
    ), "error message did not identify correct location"


def test_1425():
    """
    tooltips should propagate to SVG even without an HREF
    https://gitlab.com/graphviz/graphviz/-/issues/1425
    """

    # locate our associated test case in this directory
    input = Path(__file__).parent / "1425.dot"
    assert input.exists(), "unexpectedly missing test case"

    # translate this to SVG
    svg = dot("svg", input)

    assert re.search(r"\btable tip\b", svg) is not None, "tooltip not propagated to SVG"


def test_1425_1():
    """
    tooltips should propagate to SVG even without an HREF
    https://gitlab.com/graphviz/graphviz/-/issues/1425
    """

    # locate our associated test case in this directory
    input = Path(__file__).parent / "1425_1.dot"
    assert input.exists(), "unexpectedly missing test case"

    # translate this to SVG
    svg = dot("svg", input)

    assert (
        re.search(r"\bgreek to me\b", svg) is not None
    ), "tooltip not propagated to SVG"
    assert re.search(r"\benglish\b", svg) is not None, "tooltip not propagated to SVG"
    assert (
        re.search(r"\bleave a tip\b", svg) is not None
    ), "tooltip not propagated to SVG"
    assert (
        re.search(r"\bcell tool tip\b", svg) is not None
    ), "tooltip not propagated to SVG"
    assert re.search(r"\btd tip\b", svg) is not None, "tooltip not propagated to SVG"
    assert re.search(r"\btable tip\b", svg) is not None, "tooltip not propagated to SVG"


@pytest.mark.xfail(
    strict=True, reason="https://gitlab.com/graphviz/graphviz/-/issues/1435"
)
def test_1435():
    """
    triangulation paths should be findable on this graph
    https://gitlab.com/graphviz/graphviz/-/issues/1435
    """

    # locate our associated test case in this directory
    input = Path(__file__).parent / "1435.dot"
    assert input.exists(), "unexpectedly missing test case"

    # process it with Graphviz
    err = subprocess.check_output(
        ["dot", "-Tpng", "-o", os.devnull, input],
        stderr=subprocess.STDOUT,
        universal_newlines=True,
    )

    assert err.strip() == "", "errors were printed"


def test_1436():
    """
    test a segfault from https://gitlab.com/graphviz/graphviz/-/issues/1436 has
    not reappeared
    """

    # locate our associated test case in this directory
    input = Path(__file__).parent / "1436.dot"
    assert input.exists(), "unexpectedly missing test case"

    # ask Graphviz to process it, which should generate a segfault if this bug
    # has been reintroduced
    dot("svg", input)


def test_1444():
    """
    specifying 'headport' as an edge attribute should work regardless of what
    order attributes appear in
    https://gitlab.com/graphviz/graphviz/-/issues/1444
    """

    # locate the first of our associated tests
    input1 = Path(__file__).parent / "1444.dot"
    assert input1.exists(), "unexpectedly missing test case"

    # ask Graphviz to process it
    with subprocess.Popen(
        ["dot", "-Tsvg", input1],
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        universal_newlines=True,
    ) as p:
        stdout1, stderr = p.communicate()

        assert p.returncode == 0, "failed to process a headport edge"

    stderr = remove_xtype_warnings(stderr).strip()
    assert stderr == "", "emitted an error for a legal graph"

    # now locate our second variant, that simply has the attributes swapped
    input2 = Path(__file__).parent / "1444-2.dot"
    assert input2.exists(), "unexpectedly missing test case"

    # process it identically
    with subprocess.Popen(
        ["dot", "-Tsvg", input2],
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        universal_newlines=True,
    ) as p:
        stdout2, stderr = p.communicate()

        assert p.returncode == 0, "failed to process a headport edge"

    stderr = remove_xtype_warnings(stderr).strip()
    assert stderr == "", "emitted an error for a legal graph"

    assert stdout1 == stdout2, "swapping edge attributes altered the output graph"


def test_1449():
    """
    using the SVG color scheme should not cause warnings
    https://gitlab.com/graphviz/graphviz/-/issues/1449
    """

    # start Graphviz
    with subprocess.Popen(
        ["dot", "-Tsvg", "-o", os.devnull],
        stdin=subprocess.PIPE,
        stderr=subprocess.PIPE,
        universal_newlines=True,
    ) as p:
        # pass it some input that uses the SVG color scheme
        _, stderr = p.communicate('graph g { colorscheme="svg"; }')

        assert p.returncode == 0, "Graphviz exited with non-zero status"

    assert stderr.strip() == "", "SVG color scheme use caused warnings"


@pytest.mark.xfail(
    strict=platform.system() != "Linux",
    reason="https://gitlab.com/graphviz/graphviz/-/issues/1453",
)
def test_1453():
    """
    `splines=curved` should not result in segfaults
    https://gitlab.com/graphviz/graphviz/-/issues/1453
    """

    # locate our associated test case in this directory
    input = Path(__file__).parent / "1453.dot"
    assert input.exists(), "unexpectedly missing test case"

    # run it through Graphviz
    dot("svg", input)


@pytest.mark.xfail(
    strict=True, reason="https://gitlab.com/graphviz/graphviz/-/issues/1472"
)
def test_1472():
    """
    processing a malformed graph found by Google Autofuzz should not crash
    https://gitlab.com/graphviz/graphviz/-/issues/1472
    """

    # locate our associated test case in this directory
    input = Path(__file__).parent / "1472.dot"
    assert input.exists(), "unexpectedly missing test case"

    # run it through Graphviz
    dot("svg", input)


def test_1554():
    """
    small distances between nodes should not cause a crash in majorization
    https://gitlab.com/graphviz/graphviz/-/issues/1554
    """

    # locate our associated test case in this directory
    input = Path(__file__).parent / "1554.dot"
    assert input.exists(), "unexpectedly missing test case"

    # run it through Graphviz
    output = dot("svg", input)

    # the output should not have NaN values, indicating out of bounds computation
    assert (
        re.search(r"\bnan\b", output, flags=re.IGNORECASE) is None
    ), "computation exceeded bounds"


def test_1585():
    """
    clustering nodes should not reverse their horizontal layout
    https://gitlab.com/graphviz/graphviz/-/issues/1585
    """

    # locate our associated test cases in this directory
    no_cluster = Path(__file__).parent / "1585_0.dot"
    assert no_cluster.exists(), "unexpectedly missing test case"
    cluster = Path(__file__).parent / "1585_1.dot"
    assert cluster.exists(), "unexpectedly missing test case"

    def find_node_xs(svg_output: str) -> Iterator[float]:
        """
        yield 3 floats representing the X positions of nodes b, c, d in the
        given graph
        """

        # parse the SVG
        root = ET.fromstring(svg_output)

        # find `b`
        b = root.findall(
            ".//{http://www.w3.org/2000/svg}title[.='b']../{http://www.w3.org/2000/svg}ellipse"
        )
        assert len(b) == 1, "could not find node 'b'"
        yield float(b[0].attrib["cx"])

        # find `c`
        c = root.findall(
            ".//{http://www.w3.org/2000/svg}title[.='c']../{http://www.w3.org/2000/svg}ellipse"
        )
        assert len(c) == 1, "could not find node 'c'"
        yield float(c[0].attrib["cx"])

        # find `d`
        d = root.findall(
            ".//{http://www.w3.org/2000/svg}title[.='d']../{http://www.w3.org/2000/svg}ellipse"
        )
        assert len(d) == 1, "could not find node 'd'"
        yield float(d[0].attrib["cx"])

    # render the one without clusters and get its nodes’ X positions
    no_cluster_out = dot("svg", no_cluster)
    b, c, d = list(find_node_xs(no_cluster_out))

    # confirm we got a left → right ordering
    assert b < c, "unexpected horizontal node ordering"
    assert c < d, "unexpected horizontal node ordering"

    # now try the same thing with the clustered graph
    cluster_out = dot("svg", cluster)
    b, c, d = list(find_node_xs(cluster_out))
    assert b < c, "clustering altered nodes’ horizontal ordering"
    assert c < d, "clustering altered nodes’ horizontal ordering"


@pytest.mark.skipif(which("gvpr") is None, reason="GVPR not available")
def test_1594():
    """
    GVPR should give accurate line numbers in error messages
    https://gitlab.com/graphviz/graphviz/-/issues/1594
    """

    # locate our associated test case in this directory
    input = Path(__file__).parent / "1594.gvpr"

    # run GVPR with our (malformed) input program
    with subprocess.Popen(
        ["gvpr", "-f", input],
        stdin=subprocess.PIPE,
        stdout=subprocess.DEVNULL,
        stderr=subprocess.PIPE,
        universal_newlines=True,
    ) as p:
        _, stderr = p.communicate()

        assert p.returncode != 0, "GVPR did not reject malformed program"

    assert "line 3:" in stderr, "GVPR did not identify correct line of syntax error"


@pytest.mark.parametrize("long,short", (("--help", "-?"), ("--version", "-V")))
def test_1618(long: str, short: str):
    """
    Graphviz should understand `--help` and `--version`
    https://gitlab.com/graphviz/graphviz/-/issues/1618
    """

    # run Graphviz with the short form of the argument
    p1 = subprocess.run(
        ["dot", short], stdout=subprocess.PIPE, stderr=subprocess.PIPE, check=True
    )

    # run it with the long form of the argument
    p2 = subprocess.run(
        ["dot", long], stdout=subprocess.PIPE, stderr=subprocess.PIPE, check=True
    )

    # output from both should match
    assert (
        p1.stdout == p2.stdout
    ), f"`dot {long}` wrote differing output than `dot {short}`"
    assert (
        p1.stderr == p2.stderr
    ), f"`dot {long}` wrote differing output than `dot {short}`"


@pytest.mark.parametrize(
    "test_case", ("1622_0.dot", "1622_1.dot", "1622_2.dot", "1622_3.dot")
)
def test_1622(test_case: str):
    """
    Narrow HTML table cells should not cause assertion failures
    https://gitlab.com/graphviz/graphviz/-/issues/1622
    """

    # locate our associated test case in this directory
    input = Path(__file__).parent / test_case
    assert input.exists(), "unexpectedly missing test case"

    # process it with Graphviz
    dot("png:cairo:cairo", input)


def test_1624():
    """
    record shapes should be usable
    https://gitlab.com/graphviz/graphviz/-/issues/1624
    """

    # locate our associated test case in this directory
    input = Path(__file__).parent / "1624.dot"
    assert input.exists(), "unexpectedly missing test case"

    # process it with Graphviz
    dot("svg", input)


def test_1644():
    """
    neato results should be deterministic
    https://gitlab.com/graphviz/graphviz/-/issues/1644
    """

    # get our baseline reference
    input = Path(__file__).parent / "1644.dot"
    assert input.exists(), "unexpectedly missing test case"
    ref = subprocess.check_output(["neato", input], universal_newlines=True)

    # now repeat this, expecting it not to change
    for _ in range(20):
        out = subprocess.check_output(["neato", input], universal_newlines=True)
        assert ref == out, "repeated rendering changed output"


def test_1658():
    """
    the graph associated with this test case should not crash Graphviz
    https://gitlab.com/graphviz/graphviz/-/issues/1658
    """

    # locate our associated test case in this directory
    input = Path(__file__).parent / "1658.dot"
    assert input.exists(), "unexpectedly missing test case"

    # process it with Graphviz
    dot("png", input)


def test_1676():
    """
    https://gitlab.com/graphviz/graphviz/-/issues/1676
    """

    # locate our associated test case in this directory
    input = Path(__file__).parent / "1676.dot"
    assert input.exists(), "unexpectedly missing test case"

    # run Graphviz with this input
    ret = subprocess.call(["dot", "-Tsvg", "-o", os.devnull, input])

    # this malformed input should not have caused Graphviz to crash
    assert ret != -signal.SIGSEGV, "Graphviz segfaulted"


def test_1724():
    """
    passing malformed node and newrank should not cause segfaults
    https://gitlab.com/graphviz/graphviz/-/issues/1724
    """

    # locate our associated test case in this directory
    input = Path(__file__).parent / "1724.dot"
    assert input.exists(), "unexpectedly missing test case"

    # run Graphviz with this input
    ret = subprocess.call(["dot", "-Tsvg", "-o", os.devnull, input])

    assert ret != -signal.SIGSEGV, "Graphviz segfaulted"


@pytest.mark.skipif(
    is_static_build(),
    reason="dynamic libraries are unavailable to link against in static builds",
)
def test_1767():
    """
    using the Pango plugin multiple times should produce consistent results
    https://gitlab.com/graphviz/graphviz/-/issues/1767
    """

    # find co-located test source
    c_src = (Path(__file__).parent / "1767.c").resolve()
    assert c_src.exists(), "missing test case"

    # find our co-located dot input
    src = (Path(__file__).parent / "1767.dot").resolve()
    assert src.exists(), "missing test case"

    stdout, _ = run_c(c_src, [src], link=["cgraph", "gvc"])

    assert stdout.splitlines() == [
        "Loaded graph:clusters",
        "cluster_0 contains 5 nodes",
        "cluster_1 contains 1 nodes",
        "cluster_2 contains 3 nodes",
        "cluster_3 contains 3 nodes",
        "Loaded graph:clusters",
        "cluster_0 contains 5 nodes",
        "cluster_1 contains 1 nodes",
        "cluster_2 contains 3 nodes",
        "cluster_3 contains 3 nodes",
    ]


@pytest.mark.skipif(which("gvpr") is None, reason="GVPR not available")
@pytest.mark.skipif(platform.system() != "Windows", reason="only relevant on Windows")
def test_1780():
    """
    GVPR should accept programs at absolute paths
    https://gitlab.com/graphviz/graphviz/-/issues/1780
    """

    # get absolute path to an arbitrary GVPR program
    clustg = Path(__file__).resolve().parent.parent / "cmd/gvpr/lib/clustg"

    # GVPR should not fail when given this path
    gvpr(clustg)


def test_1783():
    """
    Graphviz should not segfault when passed large edge weights
    https://gitlab.com/graphviz/graphviz/-/issues/1783
    """

    # locate our associated test case in this directory
    input = Path(__file__).parent / "1783.dot"
    assert input.exists(), "unexpectedly missing test case"

    # run Graphviz with this input
    ret = subprocess.call(["dot", "-Tsvg", "-o", os.devnull, input])

    assert ret != 0, "Graphviz accepted illegal edge weight"

    assert ret != -signal.SIGSEGV, "Graphviz segfaulted"


@pytest.mark.skipif(which("gvedit") is None, reason="Gvedit not available")
def test_1813():
    """
    gvedit -? should show usage
    https://gitlab.com/graphviz/graphviz/-/issues/1813
    """

    environ_copy = os.environ.copy()
    environ_copy.pop("DISPLAY", None)
    output = subprocess.check_output(
        ["gvedit", "-?"], env=environ_copy, universal_newlines=True
    )

    assert "Usage" in output, "gvedit -? did not show usage"


def test_1845():
    """
    rendering sequential graphs to PS should not segfault
    https://gitlab.com/graphviz/graphviz/-/issues/1845
    """

    # locate our associated test case in this directory
    input = Path(__file__).parent / "1845.dot"
    assert input.exists(), "unexpectedly missing test case"

    # generate a multipage PS file from this input
    dot("ps", input)


@pytest.mark.xfail(strict=True)  # FIXME
def test_1856():
    """
    headports and tailports should be respected
    https://gitlab.com/graphviz/graphviz/-/issues/1856
    """

    # locate our associated test case in this directory
    input = Path(__file__).parent / "1856.dot"
    assert input.exists(), "unexpectedly missing test case"

    # process it into JSON
    out = dot("json", input)
    data = json.loads(out)

    # find the two nodes, “3” and “5”
    three = [x for x in data["objects"] if x["name"] == "3"][0]
    five = [x for x in data["objects"] if x["name"] == "5"][0]

    # find the edge from “3” to “5”
    edge = [
        x
        for x in data["edges"]
        if x["tail"] == three["_gvid"] and x["head"] == five["_gvid"]
    ][0]

    # The edge should look something like:
    #
    #        ┌─┐
    #        │3│
    #        └┬┘
    #    ┌────┘
    #   ┌┴┐
    #   │5│
    #   └─┘
    #
    # but a bug causes port constraints to not be respected and the edge comes out
    # more like:
    #
    #        ┌─┐
    #        │3│
    #        └┬┘
    #         │
    #   ┌─┐   │
    #   ├5̶┼───┘
    #   └─┘
    #
    # So validate that the edge’s path does not dip below the top of the “5” node.

    top_of_five = max(y for _, y in five["_draw_"][1]["points"])

    waypoints_y = [y for _, y in edge["_draw_"][1]["points"]]

    assert all(y >= top_of_five for y in waypoints_y), "edge dips below 5"


@pytest.mark.skipif(which("fdp") is None, reason="fdp not available")
def test_1865():
    """
    fdp should not read out of bounds when processing node names
    https://gitlab.com/graphviz/graphviz/-/issues/1865
    Note, the crash this test tries to provoke may only occur when run under
    Address Sanitizer or Valgrind
    """

    # locate our associated test case in this directory
    input = Path(__file__).parent / "1865.dot"
    assert input.exists(), "unexpectedly missing test case"

    # fdp should not crash when processing this file
    subprocess.check_call(["fdp", "-o", os.devnull, input])


@pytest.mark.skipif(which("gv2gml") is None, reason="gv2gml not available")
@pytest.mark.skipif(which("gml2gv") is None, reason="gml2gv not available")
@pytest.mark.parametrize(
    "penwidth",
    (pytest.param("1.0", id="penwidth=1.0"), pytest.param("1", id="pendwidth=1")),
)
def test_1871(penwidth: str):
    """
    round tripping something with either an integer or real `penwidth` through
    gv2gml→gml2gv should return the correct `penwidth`
    """

    # a trivial graph
    input = f"graph {{ a [penwidth={penwidth}] }}"

    # pass it through gv2gml
    gv = subprocess.check_output(["gv2gml"], input=input, universal_newlines=True)

    # pass this through gml2gv
    gml = subprocess.check_output(["gml2gv"], input=gv, universal_newlines=True)

    # the result should have a `penwidth` of 1
    has_1 = re.search(r"\bpenwidth\s*=\s*1[^\.]", gml) is not None
    has_1_0 = re.search(r"\bpenwidth\s*=\s*1\.0\b", gml) is not None
    assert (
        has_1 or has_1_0
    ), f"incorrect penwidth from round tripping through GML (output {gml})"


@pytest.mark.skipif(which("fdp") is None, reason="fdp not available")
def test_1876():
    """
    fdp should not rename nodes with internal names
    https://gitlab.com/graphviz/graphviz/-/issues/1876
    """

    # a trivial graph to provoke this issue
    input = "graph { a }"

    # process this with fdp
    try:
        output = subprocess.check_output(["fdp"], input=input, universal_newlines=True)
    except subprocess.CalledProcessError as e:
        raise RuntimeError("fdp failed to process trivial graph") from e

    # we should not see any internal names like "%3"
    assert "%" not in output, "internal name in fdp output"


@pytest.mark.skipif(which("fdp") is None, reason="fdp not available")
def test_1877():
    """
    fdp should not fail an assertion when processing cluster edges
    https://gitlab.com/graphviz/graphviz/-/issues/1877
    """

    # simple input with a cluster edge
    input = "graph {subgraph cluster_a {}; cluster_a -- b}"

    # fdp should be able to process this
    subprocess.run(
        ["fdp", "-o", os.devnull], input=input, check=True, universal_newlines=True
    )


def test_1880():
    """
    parsing a particular graph should not cause a Trapezoid-table overflow
    assertion failure
    https://gitlab.com/graphviz/graphviz/-/issues/1880
    """

    # locate our associated test case in this directory
    input = Path(__file__).parent / "1880.dot"
    assert input.exists(), "unexpectedly missing test case"

    # process it with Graphviz
    dot("png", input)


@pytest.mark.xfail(
    strict=True, reason="https://gitlab.com/graphviz/graphviz/-/issues/1887"
)
def test_1887():
    """
    empty strings as labels should be propagated to dot output
    https://gitlab.com/graphviz/graphviz/-/issues/1887
    """

    # find co-located test source
    c_src = (Path(__file__).parent / "1887.c").resolve()
    assert c_src.exists(), "missing test case"

    # generate a graph and pass it through dot
    stdout, _ = run_c(c_src, link=["cgraph"])

    assert (
        re.search(r'label\s*=\s*""', stdout) is not None
    ), "empty label missing in output"


def test_1898():
    """
    test a segfault from https://gitlab.com/graphviz/graphviz/-/issues/1898 has
    not reappeared
    """

    # locate our associated test case in this directory
    input = Path(__file__).parent / "1898.dot"
    assert input.exists(), "unexpectedly missing test case"

    # ask Graphviz to process it, which should generate a segfault if this bug
    # has been reintroduced
    dot("svg", input)


def test_1902():
    """
    test a segfault from https://gitlab.com/graphviz/graphviz/-/issues/1902 has
    not reappeared
    """

    # locate our associated test case in this directory
    input = Path(__file__).parent / "1902.dot"
    assert input.exists(), "unexpectedly missing test case"

    # ask Graphviz to process it, which should generate a segfault if this bug
    # has been reintroduced
    dot("svg", input)


# root directory of this checkout
ROOT = Path(__file__).parent.parent.resolve()


def test_1855():
    """
    SVGs should have a scale with sufficient precision
    https://gitlab.com/graphviz/graphviz/-/issues/1855
    """

    # locate our associated test case in this directory
    src = Path(__file__).parent / "1855.dot"
    assert src.exists(), "unexpectedly missing test case"

    # run it through Graphviz
    svg = dot("svg", src)

    # find the graph element
    root = ET.fromstring(svg)
    graph = root[0]
    assert graph.get("class") == "graph", "could not find graph element"

    # extract its `transform` attribute
    transform = graph.get("transform")

    # this should begin with a scale directive
    m = re.match(r"scale\((?P<x>\d+(\.\d*)?) (?P<y>\d+(\.\d*))\)", transform)
    assert m is not None, f"failed to find 'scale' in '{transform}'"

    x = m.group("x")
    y = m.group("y")

    # the scale should be somewhere in reasonable range of what is expected
    assert float(x) >= 0.32 and float(x) <= 0.34, "inaccurate x scale"
    assert float(y) >= 0.32 and float(y) <= 0.34, "inaccurate y scale"

    # two digits of precision are insufficient for this example, so require a
    # greater number of digits in both scale components
    assert len(x) > 4, "insufficient precision in x scale"
    assert len(y) > 4, "insufficient precision in y scale"


@pytest.mark.parametrize("variant", [1, 2])
@pytest.mark.skipif(which("gml2gv") is None, reason="gml2gv not available")
def test_1869(variant: int):
    """
    gml2gv should be able to parse the style, outlineStyle, width and
    outlineWidth GML attributes and map them to the DOT attributes
    style and penwidth respectively
    https://gitlab.com/graphviz/graphviz/-/issues/1869
    """

    # locate our associated test case in this directory
    input = Path(__file__).parent / f"1869-{variant}.gml"
    assert input.exists(), "unexpectedly missing test case"

    # ask gml2gv to translate it to DOT
    output = subprocess.check_output(["gml2gv", input], universal_newlines=True)

    assert "style=dashed" in output, "style=dashed not found in DOT output"
    assert "penwidth=2" in output, "penwidth=2 not found in DOT output"


def test_1879():
    """https://gitlab.com/graphviz/graphviz/-/issues/1879"""

    # locate our associated test case in this directory
    input = Path(__file__).parent / "1879.dot"
    assert input.exists(), "unexpectedly missing test case"

    # process it with DOT
    stdout = subprocess.check_output(
        ["dot", "-Tsvg", "-o", os.devnull, input],
        cwd=Path(__file__).parent,
        stderr=subprocess.STDOUT,
        universal_newlines=True,
    )

    # check we did not trigger an assertion failure
    assert re.search(r"\bAssertion\b.*\bfailed\b", stdout) is None


def test_1879_2():
    """
    another variant of lhead/ltail + compound
    https://gitlab.com/graphviz/graphviz/-/issues/1879
    """

    # locate our associated test case in this directory
    input = Path(__file__).parent / "1879-2.dot"
    assert input.exists(), "unexpectedly missing test case"

    # process it with DOT
    subprocess.check_call(["dot", "-Gmargin=0", "-Tpng", "-o", os.devnull, input])


def test_1893():
    """
    an HTML label containing just a ] should work
    https://gitlab.com/graphviz/graphviz/-/issues/1893
    """

    # a graph containing a node with an HTML label with a ] in a table cell
    input = "digraph { 0 [label=<<TABLE><TR><TD>]</TD></TR></TABLE>>] }"

    # ask Graphviz to process this
    dot("svg", source=input)

    # we should be able to do the same with an escaped ]
    input = "digraph { 0 [label=<<TABLE><TR><TD>&#93;</TD></TR></TABLE>>] }"

    dot("svg", source=input)


def test_1906():
    """
    graphs that generate large rectangles should be accepted
    https://gitlab.com/graphviz/graphviz/-/issues/1906
    """

    # one of the rtest graphs is sufficient to provoke this
    input = Path(__file__).parent / "graphs/root.gv"
    assert input.exists(), "unexpectedly missing test case"

    # use Circo to translate it to DOT
    subprocess.check_call(["dot", "-Kcirco", "-Tgv", "-o", os.devnull, input])


@pytest.mark.skipif(which("twopi") is None, reason="twopi not available")
def test_1907():
    """
    SVG edges should have title elements that match their names
    https://gitlab.com/graphviz/graphviz/-/issues/1907
    """

    # a trivial graph to provoke this issue
    input = "digraph { A -> B -> C }"

    # generate an SVG from this input with twopi
    output = subprocess.check_output(
        ["twopi", "-Tsvg"], input=input, universal_newlines=True
    )

    assert "<title>A&#45;&gt;B</title>" in output, "element title not found in SVG"


@pytest.mark.skipif(which("gvpr") is None, reason="gvpr not available")
def test_1909():
    """
    GVPR should not output internal names
    https://gitlab.com/graphviz/graphviz/-/issues/1909
    """

    # locate our associated test case in this directory
    prog = Path(__file__).parent / "1909.gvpr"
    graph = Path(__file__).parent / "1909.dot"

    # run GVPR with the given input
    output = subprocess.check_output(
        ["gvpr", "-c", "-f", prog, graph], universal_newlines=True
    )

    # we should have produced this graph without names like "%2" in it
    assert output == "// begin\n" "digraph bug {\n" "	a -> b;\n" "	b -> c;\n" "}\n"


@pytest.mark.skipif(
    is_static_build(),
    reason="dynamic libraries are unavailable to link against in static builds",
)
def test_1910():
    """
    Repeatedly using agmemread() should have consistent results
    https://gitlab.com/graphviz/graphviz/-/issues/1910
    """

    # find co-located test source
    c_src = (Path(__file__).parent / "1910.c").resolve()
    assert c_src.exists(), "missing test case"

    # run the test
    _, _ = run_c(c_src, link=["cgraph", "gvc"])


def test_1913():
    """
    ALIGN attributes in <BR> tags should be parsed correctly
    https://gitlab.com/graphviz/graphviz/-/issues/1913
    """

    # a template of a trivial graph using an ALIGN attribute
    graph = (
        "digraph {{\n"
        '  table1[label=<<table><tr><td align="text">hello world'
        '<br align="{}"/></td></tr></table>>];\n'
        "}}"
    )

    def run(input):
        """
        run Dot with the given input and return its exit status and stderr
        """
        with subprocess.Popen(
            ["dot", "-Tsvg", "-o", os.devnull],
            stdin=subprocess.PIPE,
            stderr=subprocess.PIPE,
            universal_newlines=True,
        ) as p:
            _, stderr = p.communicate(input)
            return p.returncode, remove_xtype_warnings(stderr)

    # Graphviz should accept all legal values for this attribute
    for align in ("left", "right", "center"):
        input = align
        ret, stderr = run(graph.format(input))
        assert ret == 0
        assert stderr.strip() == ""

        # these attributes should also be valid when title cased
        input = f"{align[0].upper()}{align[1:]}"
        ret, stderr = run(graph.format(input))
        assert ret == 0
        assert stderr.strip() == ""

        # similarly, they should be valid when upper cased
        input = align.upper()
        ret, stderr = run(graph.format(input))
        assert ret == 0
        assert stderr.strip() == ""

    # various invalid things that have the same prefix or suffix as a valid
    # alignment should be rejected
    for align in ("lamp", "deft", "round", "might", "circle", "venter"):
        input = align
        _, stderr = run(graph.format(input))
        assert f"Warning: Illegal value {input} for ALIGN - ignored" in stderr

        # these attributes should also fail when title cased
        input = f"{align[0].upper()}{align[1:]}"
        _, stderr = run(graph.format(input))
        assert f"Warning: Illegal value {input} for ALIGN - ignored" in stderr

        # similarly, they should fail when upper cased
        input = align.upper()
        _, stderr = run(graph.format(input))
        assert f"Warning: Illegal value {input} for ALIGN - ignored" in stderr


def test_1931():
    """
    New lines within strings should not be discarded during parsing

    """

    # a graph with \n inside of strings
    graph = (
        "graph {\n"
        '  node1 [label="line 1\n'
        "line 2\n"
        '"];\n'
        '  node2 [label="line 3\n'
        'line 4"];\n'
        "  node1 -- node2\n"
        '  node2 -- "line 5\n'
        'line 6"\n'
        "}"
    )

    # ask Graphviz to process this to dot output
    xdot = dot("xdot", source=graph)

    # all new lines in strings should have been preserved
    assert "line 1\nline 2\n" in xdot
    assert "line 3\nline 4" in xdot
    assert "line 5\nline 6" in xdot


@pytest.mark.xfail(
    strict=True, reason="https://gitlab.com/graphviz/graphviz/-/issues/1939"
)
def test_1939():
    """
    clustering should not cause “trouble in init_rank” errors
    https://gitlab.com/graphviz/graphviz/-/issues/1939
    """

    # locate our associated test case in this directory
    input = Path(__file__).parent / "1939.dot"
    assert input.exists(), "unexpectedly missing test case"

    # run it through Graphviz
    dot("svg", input)


@pytest.mark.xfail()  # FIXME
def test_1949():
    """
    rankdir=LR + compound=true should not lead to an assertion failure
    https://gitlab.com/graphviz/graphviz/-/issues/1949
    """

    # locate our associated test case in this directory
    input = Path(__file__).parent / "1949.dot"
    assert input.exists(), "unexpectedly missing test case"

    # run it through Graphviz
    dot("png", input)


@pytest.mark.skipif(which("edgepaint") is None, reason="edgepaint not available")
def test_1971():
    """
    edgepaint should reject invalid command line options
    https://gitlab.com/graphviz/graphviz/-/issues/1971
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

    # run edgepaint with an invalid option, `-rabbit`, that happens to have the
    # same first character as valid options
    args = ["edgepaint", "-rabbit"]
    with subprocess.Popen(args, stdin=subprocess.PIPE, universal_newlines=True) as p:
        p.communicate(input)

        assert p.returncode != 0, "edgepaint incorrectly accepted '-rabbit'"


def test_1990():
    """
    using ortho and circo in combination should not cause an assertion failure
    https://gitlab.com/graphviz/graphviz/-/issues/14
    """

    # locate our associated test case in this directory
    input = Path(__file__).parent / "1990.dot"
    assert input.exists(), "unexpectedly missing test case"

    # process it with Graphviz
    subprocess.check_call(["circo", "-Tsvg", "-o", os.devnull, input])


@pytest.mark.skipif(
    is_static_build(),
    reason="dynamic libraries are unavailable to link against in static builds",
)
def test_2057():
    """
    gvToolTred should be usable by user code
    https://gitlab.com/graphviz/graphviz/-/issues/2057
    """

    # find co-located test source
    c_src = (Path(__file__).parent / "2057.c").resolve()
    assert c_src.exists(), "missing test case"

    # run the test
    _, _ = run_c(c_src, link=["gvc"])


def test_2078():
    """
    Incorrectly using the "layout" attribute on a subgraph should result in a
    sensible error.
    https://gitlab.com/graphviz/graphviz/-/issues/2078
    """

    # our sample graph that incorrectly uses layout
    input = "graph {\n  subgraph {\n    layout=osage\n  }\n}"

    # run it through Graphviz
    with subprocess.Popen(
        ["dot", "-Tcanon", "-o", os.devnull],
        stdin=subprocess.PIPE,
        stderr=subprocess.PIPE,
        universal_newlines=True,
    ) as p:
        _, stderr = p.communicate(input)

        assert p.returncode != 0, "layout on subgraph was incorrectly accepted"

    assert (
        "layout attribute is invalid except on the root graph" in stderr
    ), "expected warning not found"

    # a graph that correctly uses layout
    input = "graph {\n  layout=osage\n  subgraph {\n  }\n}"

    # ensure this one does not trigger warnings
    with subprocess.Popen(
        ["dot", "-Tcanon", "-o", os.devnull],
        stdin=subprocess.PIPE,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        universal_newlines=True,
    ) as p:
        stdout, stderr = p.communicate(input)

        assert p.returncode == 0, "correct layout use was rejected"

    assert stdout.strip() == "", "unexpected output"
    assert (
        "layout attribute is invalid except on the root graph" not in stderr
    ), "incorrect warning output"


def test_2082():
    """
    Check a bug in inside_polygon has not been reintroduced.
    https://gitlab.com/graphviz/graphviz/-/issues/2082
    """

    # locate our associated test case in this directory
    input = Path(__file__).parent / "2082.dot"
    assert input.exists(), "unexpectedly missing test case"

    # ask Graphviz to process it, which should generate an assertion failure if
    # this bug has been reintroduced
    dot("png", input)


def test_2087():
    """
    spline routing should be aware of and ignore concentrated edges
    https://gitlab.com/graphviz/graphviz/-/issues/2087
    """

    # locate our associated test case in this directory
    input = Path(__file__).parent / "2087.dot"
    assert input.exists(), "unexpectedly missing test case"

    # process it with Graphviz
    warnings = subprocess.check_output(
        ["dot", "-Tpng", "-o", os.devnull, input],
        stderr=subprocess.STDOUT,
        universal_newlines=True,
    )

    # work around macOS warnings
    warnings = remove_xtype_warnings(warnings).strip()

    # no warnings should have been printed
    assert (
        warnings == ""
    ), "warnings were printed when processing concentrated duplicate edges"


@pytest.mark.xfail(strict=True)
@pytest.mark.parametrize("html_like_first", (False, True))
def test_2089(html_like_first: bool):  # FIXME
    """
    HTML-like and non-HTML-like strings should peacefully coexist
    https://gitlab.com/graphviz/graphviz/-/issues/2089
    """

    # a graph using an HTML-like string and a non-HTML-like string
    if html_like_first:
        graph = 'graph {\n  a[label=<foo>];\n  b[label="foo"];\n}'
    else:
        graph = 'graph {\n  a[label="foo"];\n  b[label=<foo>];\n}'

    # normalize the graph
    canonical = dot("dot", source=graph)

    assert "label=foo" in canonical, "non-HTML-like label not found"
    assert "label=<foo>" in canonical, "HTML-like label not found"


@pytest.mark.xfail(strict=True)  # FIXME
def test_2089_2():
    """
    HTML-like and non-HTML-like strings should peacefully coexist
    https://gitlab.com/graphviz/graphviz/-/issues/2089
    """

    # find co-located test source
    c_src = (Path(__file__).parent / "2089.c").resolve()
    assert c_src.exists(), "missing test case"

    # run it
    _, _ = run_c(c_src, link=["cgraph"])


@pytest.mark.skipif(which("dot2gxl") is None, reason="dot2gxl not available")
def test_2092():
    """
    an empty node ID should not cause a dot2gxl NULL pointer dereference
    https://gitlab.com/graphviz/graphviz/-/issues/2092
    """
    p = subprocess.run(["dot2gxl", "-d"], input='<node id="">', universal_newlines=True)

    assert p.returncode != 0, "dot2gxl accepted invalid input"

    assert p.returncode == 1, "dot2gxl crashed"


@pytest.mark.skipif(which("dot2gxl") is None, reason="dot2gxl not available")
def test_2093():
    """
    dot2gxl should handle elements with no ID
    https://gitlab.com/graphviz/graphviz/-/issues/2093
    """
    with subprocess.Popen(
        ["dot2gxl", "-d"], stdin=subprocess.PIPE, universal_newlines=True
    ) as p:
        p.communicate('<graph x="">')

        assert p.returncode == 1, "dot2gxl did not reject missing ID"


@pytest.mark.skipif(which("dot2gxl") is None, reason="dot2gxl not available")
def test_2094():
    """
    dot2gxl should not crash when decoding a closing node tag after a closing
    graph tag
    https://gitlab.com/graphviz/graphviz/-/issues/2094
    """

    # locate our associated test case in this directory
    input = Path(__file__).parent / "2094.xml"
    assert input.exists(), "unexpectedly missing test case"

    dot2gxl = which("dot2gxl")
    ret = subprocess.call([dot2gxl, "-d", input])

    assert ret in (
        0,
        1,
    ), "dot2gxl crashed when processing a closing node tag after a closing graph tag"
    assert ret == 1, "dot2gxl did not reject malformed XML"


def test_2095():
    """
    Exceeding 1000 boxes during computation should not cause a crash
    https://gitlab.com/graphviz/graphviz/-/issues/2095
    """

    # locate our associated test case in this directory
    input = Path(__file__).parent / "2095.dot"
    assert input.exists(), "unexpectedly missing test case"

    # ask Graphviz to process it
    dot("pdf", input)


@pytest.mark.skipif(which("gv2gml") is None, reason="gv2gml not available")
def test_2131():
    """
    gv2gml should be able to process basic Graphviz input
    https://gitlab.com/graphviz/graphviz/-/issues/2131
    """

    # a trivial graph
    input = "digraph { a -> b; }"

    # ask gv2gml what it thinks of this
    try:
        subprocess.run(["gv2gml"], input=input, check=True, universal_newlines=True)
    except subprocess.CalledProcessError as e:
        raise RuntimeError("gv2gml rejected a basic graph") from e


@pytest.mark.skipif(which("gvpr") is None, reason="gvpr not available")
@pytest.mark.parametrize("examine", ("indices", "tokens"))
def test_2138(examine: str):
    """
    gvpr splitting and tokenizing should not result in trailing garbage
    https://gitlab.com/graphviz/graphviz/-/issues/2138
    """

    # find our co-located GVPR program
    script = (Path(__file__).parent / "2138.gvpr").resolve()
    assert script.exists(), "missing test case"

    # run it with NUL input
    out = subprocess.check_output(["gvpr", "-f", script], stdin=subprocess.DEVNULL)

    # Decode into text. We do this instead of `universal_newlines=True` above
    # because the trailing garbage can contain invalid UTF-8 data causing cryptic
    # failures. We want to correctly surface this as trailing garbage, not an
    # obscure UTF-8 decoding error.
    result = out.decode("utf-8", "replace")

    if examine == "indices":
        # check no indices are miscalculated
        index_re = (
            r"^// index of space \(st\) :\s*(?P<index>-?\d+)\s*<< must "
            r"NOT be less than -1$"
        )
        for m in re.finditer(index_re, result, flags=re.MULTILINE):
            index = int(m.group("index"))
            assert index >= -1, "illegal index computed"

    if examine == "tokens":
        # check for text the author of 2138.gvpr expected to find
        assert (
            "// tok[3]    >>3456789<<   should NOT include trailing spaces or "
            "junk chars" in result
        ), "token 3456789 not found or has trailing garbage"
        assert (
            "// tok[7]    >>012<<   should NOT include trailing spaces or "
            "junk chars" in result
        ), "token 012 not found or has trailing garbage"


def test_2159():
    """
    space for HTML TDs should be allocated equally when expanding to fill a TR
    https://gitlab.com/graphviz/graphviz/-/issues/2159
    """

    # locate our associated test case in this directory
    input = Path(__file__).parent / "2159.dot"
    assert input.exists(), "unexpectedly missing test case"

    # translate this to SVG
    svg = dot("svg", input)

    # load it as XML
    root = ET.fromstring(svg)

    # the first node is expected to contain:
    #   • 1 polygon for the top column-spanning cell
    #   • 5 polygons for the bottom row’s cells
    #   • 1 polygon for the outer table border
    polygons = root.findall(
        ".//{http://www.w3.org/2000/svg}title[.='node1']/../{http://www.w3.org/2000/svg}polygon"
    )
    assert len(polygons) == 7

    # extract the points delimiting the top row
    top_row = polygons[0]
    points = [
        [float(n) for n in p.split(",")] for p in top_row.get("points").split(" ")
    ]
    assert len(points) == 5, "polygon not rectangular"
    (ul_x, _), (ll_x, _), (lr_x, _), (ur_x, _), (orig_x, _) = points
    assert ul_x == ll_x, "polygon left edge is not vertical"
    assert lr_x == ur_x, "polygon right edge is not vertical"
    assert orig_x == ul_x, "polygon is not closed"
    left = ul_x
    right = ur_x

    # extract the points for each cell in the bottom row
    bottom_row = []
    for cell in polygons[1:-1]:
        bottom_row += [
            [[float(n) for n in p.split(",")] for p in cell.get("points").split(" ")]
        ]
        assert len(bottom_row[-1]) == 5, "polygon not rectangular"

    # extract the widths of each cell in the bottom row
    widths = []
    for cell in bottom_row:
        (ul_x, _), _, _, (ur_x, _), _ = cell
        widths += [ur_x - ul_x]

    # these should approximately sum to the width of the top row
    assert math.isclose(
        sum(widths), right - left, abs_tol=10
    ), "bottom row not expanded to fill the space"

    # the width of each cell should be approximately equal
    for width in widths[1:]:
        assert math.isclose(width, widths[0], abs_tol=5), "cells not evenly expanded"


def test_2168():
    """
    using spline routing should not cause fdp/neato to infinite loop
    https://gitlab.com/graphviz/graphviz/-/issues/2168
    """

    # locate our associated test case in this directory
    input = Path(__file__).parent / "2168.dot"
    assert input.exists(), "unexpectedly missing test case"

    subprocess.check_call(["fdp", "-o", os.devnull, input], timeout=5)


def test_2168_1():
    """
    using spline routing should not cause fdp/neato to infinite loop
    https://gitlab.com/graphviz/graphviz/-/issues/2168
    """

    # locate our associated test case in this directory
    input = Path(__file__).parent / "2168_1.dot"
    assert input.exists(), "unexpectedly missing test case"

    subprocess.check_call(["fdp", "-o", os.devnull, input], timeout=5)


def test_2168_2():
    """
    using spline routing should not cause fdp/neato to infinite loop
    https://gitlab.com/graphviz/graphviz/-/issues/2168
    """

    # locate our associated test case in this directory
    input = Path(__file__).parent / "2168_2.dot"
    assert input.exists(), "unexpectedly missing test case"

    subprocess.check_call(["fdp", "-o", os.devnull, input], timeout=5)


def test_2168_3():
    """
    using spline routing should not cause fdp/neato to infinite loop
    https://gitlab.com/graphviz/graphviz/-/issues/2168
    """

    # locate our associated test case in this directory
    input = Path(__file__).parent / "2168_3.dot"
    assert input.exists(), "unexpectedly missing test case"

    subprocess.check_call(["fdp", "-o", os.devnull, input], timeout=5)


def test_2168_4():
    """
    using spline routing should not cause fdp/neato to infinite loop
    https://gitlab.com/graphviz/graphviz/-/issues/2168
    """

    # locate our associated test case in this directory
    input = Path(__file__).parent / "2168_4.dot"
    assert input.exists(), "unexpectedly missing test case"

    subprocess.check_call(["fdp", "-o", os.devnull, input], timeout=5)


def test_2168_5():
    """
    using spline routing should not cause fdp/neato to infinite loop
    https://gitlab.com/graphviz/graphviz/-/issues/2168
    """

    # locate our associated test case in this directory
    input = Path(__file__).parent / "2168_5.dot"
    assert input.exists(), "unexpectedly missing test case"

    out = subprocess.check_output(
        ["fdp", "-o", os.devnull, input],
        stderr=subprocess.STDOUT,
        universal_newlines=True,
    )

    assert (
        "Warning: the bounding boxes of some nodes touch - falling back to straight line edges"
        in out
    )


def test_2179():
    """
    processing a label with an empty line should not yield a warning
    https://gitlab.com/graphviz/graphviz/-/issues/2179
    """

    # a graph containing a label with an empty line
    input = 'digraph "" {\n  0 -> 1 [fontname="Lato",label=<<br/>1>]\n}'

    # run a graph with an empty label through Graphviz
    with subprocess.Popen(
        ["dot", "-Tsvg", "-o", os.devnull],
        stdin=subprocess.PIPE,
        stderr=subprocess.PIPE,
        universal_newlines=True,
    ) as p:
        _, stderr = p.communicate(input)

        assert p.returncode == 0

    assert (
        "Warning: no hard-coded metrics for" not in stderr
    ), "incorrect warning triggered"


def test_2179_1():
    """
    processing a label with a line containing only a space should not yield a
    warning
    https://gitlab.com/graphviz/graphviz/-/issues/2179
    """

    # a graph containing a label with a line containing only a space
    input = 'digraph "" {\n  0 -> 1 [fontname="Lato",label=< <br/>1>]\n}'

    # run a graph with an empty label through Graphviz
    with subprocess.Popen(
        ["dot", "-Tsvg", "-o", os.devnull],
        stdin=subprocess.PIPE,
        stderr=subprocess.PIPE,
        universal_newlines=True,
    ) as p:
        _, stderr = p.communicate(input)

        assert p.returncode == 0

    assert (
        "Warning: no hard-coded metrics for" not in stderr
    ), "incorrect warning triggered"


@pytest.mark.skipif(which("nop") is None, reason="nop not available")
def test_2184_1():
    """
    nop should not reposition labelled graph nodes
    https://gitlab.com/graphviz/graphviz/-/issues/2184
    """

    # run `nop` on a sample with a labelled graph node at the end
    source = Path(__file__).parent / "2184.dot"
    assert source.exists(), "missing test case"
    nopped = subprocess.check_output(["nop", source], universal_newlines=True)

    # the normalized output should have a graph with no label within
    # `clusterSurround1`
    m = re.search(
        r"\bclusterSurround1\b.*\bgraph\b.*\bcluster1\b", nopped, flags=re.DOTALL
    )
    assert m is not None, "nop rearranged a graph in a not-semantically-preserving way"


def test_2184_2():
    """
    canonicalization should not reposition labelled graph nodes
    https://gitlab.com/graphviz/graphviz/-/issues/2184
    """

    # canonicalize a sample with a labelled graph node at the end
    source = Path(__file__).parent / "2184.dot"
    assert source.exists(), "missing test case"
    canonicalized = dot("canon", source)

    # the canonicalized output should have a graph with no label within
    # `clusterSurround1`
    m = re.search(
        r"\bclusterSurround1\b.*\bgraph\b.*\bcluster1\b", canonicalized, flags=re.DOTALL
    )
    assert (
        m is not None
    ), "`dot -Tcanon` rearranged a graph in a not-semantically-preserving way"


def test_2185_1():
    """
    GVPR should deal with strings correctly
    https://gitlab.com/graphviz/graphviz/-/issues/2185
    """

    # find our collocated GVPR program
    script = Path(__file__).parent / "2185.gvpr"
    assert script.exists(), "missing test case"

    # run this with NUL input, checking output is valid UTF-8
    gvpr(script)


def test_2185_2():
    """
    GVPR should deal with strings correctly
    https://gitlab.com/graphviz/graphviz/-/issues/2185
    """

    # find our collocated GVPR program
    script = Path(__file__).parent / "2185.gvpr"
    assert script.exists(), "missing test case"

    # run this with NUL input
    out = subprocess.check_output(["gvpr", "-f", script], stdin=subprocess.DEVNULL)

    # decode output in a separate step to gracefully cope with garbage unicode
    out = out.decode("utf-8", "replace")

    # deal with Windows eccentricities
    eol = "\r\n" if platform.system() == "Windows" else "\n"
    expected = f"one two three{eol}"

    # check the first line is as expected
    assert out.startswith(expected), "incorrect GVPR interpretation"


def test_2185_3():
    """
    GVPR should deal with strings correctly
    https://gitlab.com/graphviz/graphviz/-/issues/2185
    """

    # find our collocated GVPR program
    script = Path(__file__).parent / "2185.gvpr"
    assert script.exists(), "missing test case"

    # run this with NUL input
    out = subprocess.check_output(["gvpr", "-f", script], stdin=subprocess.DEVNULL)

    # decode output in a separate step to gracefully cope with garbage unicode
    out = out.decode("utf-8", "replace")

    # deal with Windows eccentricities
    eol = "\r\n" if platform.system() == "Windows" else "\n"
    expected = f"one two three{eol}one  five three{eol}"

    # check the first two lines are as expected
    assert out.startswith(expected), "incorrect GVPR interpretation"


def test_2185_4():
    """
    GVPR should deal with strings correctly
    https://gitlab.com/graphviz/graphviz/-/issues/2185
    """

    # find our collocated GVPR program
    script = Path(__file__).parent / "2185.gvpr"
    assert script.exists(), "missing test case"

    # run this with NUL input
    out = subprocess.check_output(["gvpr", "-f", script], stdin=subprocess.DEVNULL)

    # decode output in a separate step to gracefully cope with garbage unicode
    out = out.decode("utf-8", "replace")

    # deal with Windows eccentricities
    eol = "\r\n" if platform.system() == "Windows" else "\n"
    expected = f"one two three{eol}one  five three{eol}99{eol}"

    # check the first three lines are as expected
    assert out.startswith(expected), "incorrect GVPR interpretation"


def test_2185_5():
    """
    GVPR should deal with strings correctly
    https://gitlab.com/graphviz/graphviz/-/issues/2185
    """

    # find our collocated GVPR program
    script = Path(__file__).parent / "2185.gvpr"
    assert script.exists(), "missing test case"

    # run this with NUL input
    out = subprocess.check_output(["gvpr", "-f", script], stdin=subprocess.DEVNULL)

    # decode output in a separate step to gracefully cope with garbage unicode
    out = out.decode("utf-8", "replace")

    # deal with Windows eccentricities
    eol = "\r\n" if platform.system() == "Windows" else "\n"
    expected = f"one two three{eol}one  five three{eol}99{eol}Constant{eol}"

    # check the first four lines are as expected
    assert out.startswith(expected), "incorrect GVPR interpretation"


@pytest.mark.xfail(strict=True)  # FIXME
def test_2193():
    """
    the canonical format should be stable
    https://gitlab.com/graphviz/graphviz/-/issues/2193
    """

    # find our collocated test case
    input = Path(__file__).parent / "2193.dot"
    assert input.exists(), "unexpectedly missing test case"

    # derive the initial canonicalization
    canonical = dot("canon", input)

    # now canonicalize this again to see if it changes
    new = dot("canon", source=canonical)
    assert canonical == new, "canonical translation is not stable"


@pytest.mark.skipif(which("gvpr") is None, reason="GVPR not available")
def test_2211():
    """
    GVPR’s `index` function should return correct results
    https://gitlab.com/graphviz/graphviz/-/issues/2211
    """

    # find our collocated test case
    program = Path(__file__).parent / "2211.gvpr"
    assert program.exists(), "unexpectedly missing test case"

    # run it through GVPR
    output = gvpr(program)

    # it should have found the right string indices for characters
    assert (
        output == "index: 9  should be 9\n"
        "index: 3  should be 3\n"
        "index: -1  should be -1\n"
    )


def test_2215():
    """
    Graphviz should not crash with `-v`
    https://gitlab.com/graphviz/graphviz/-/issues/2215
    """

    # try it on a simple graph
    input = "graph g { a -- b; }"
    subprocess.run(["dot", "-v"], input=input, check=True, universal_newlines=True)

    # try the same on a labelled version of this graph
    input = 'graph g { node[label=""] a -- b; }'
    subprocess.run(["dot", "-v"], input=input, check=True, universal_newlines=True)


@pytest.mark.xfail(
    is_rocky(),
    strict=True,
    reason="https://gitlab.com/graphviz/graphviz/-/issues/2241",
)
def test_2241():
    """
    a graph with two nodes and one edge in each direction should be rendered
    with two visually distinct edges when using the neato engine and
    splines=true, not two edges on top of each other, visually looking like a
    single edge with both head and tail arrowheads.
    https://gitlab.com/graphviz/graphviz/-/issues/2241
    """

    # find our collocated test case
    input = Path(__file__).parent / "2241.dot"
    assert input.exists(), "unexpectedly missing test case"

    # run it through Graphviz
    svg = dot("svg", input)

    # load this as XML
    root = ET.fromstring(svg)

    # the output is expected to contain two paths which are well separated
    paths = root.findall(".//{http://www.w3.org/2000/svg}path")
    assert len(paths) == 2, "expected two paths in output"
    ellipses = root.findall(".//{http://www.w3.org/2000/svg}ellipse")
    assert len(ellipses) == 2, "expected two ellipses in output"

    # calculate the x coordinate of a vertical line which is equidistant from the two nodes
    x = statistics.mean(float(ellipse.get("cx")) for ellipse in ellipses)

    # for each edge path, get the y coordinate of a point on a line between the edge's endpoints
    # where the line intersects the node equidistant vertical line
    y_coordinates = []
    for path in paths:
        d_attribute = path.get("d")
        points_str = re.split("[ C]", d_attribute.replace("M", ""))
        assert (
            len(points_str) == 4
        ), "expected four points in the 'd' attribute of the 'path' element"
        points = [
            (float(x_str), float(y_str))
            for x_str, y_str in [point_str.split(",") for point_str in points_str]
        ]
        dx = points[3][0] - points[0][0]
        dy = points[3][1] - points[0][1]
        y = points[0][1] + dy / dx * (x - points[0][0])
        y_coordinates.append(y)

    # check that the lines are well separated vertically where they intersect the node equidistant
    # vertical line
    y_coordinates_abs_difference = abs(y_coordinates[1] - y_coordinates[0])
    y_coordinates_abs_difference_when_ok = 11.5004437538844
    y_coordinates_abs_difference_when_not_ok = 0.00658290568043185
    min_y_coordinates_abs_difference = (
        y_coordinates_abs_difference_when_ok + y_coordinates_abs_difference_when_not_ok
    ) / 2
    assert y_coordinates_abs_difference > min_y_coordinates_abs_difference


def test_2242():
    """
    repeated runs of a graph with subgraphs should yield a stable result
    https://gitlab.com/graphviz/graphviz/-/issues/2242
    """

    # get our baseline reference
    input = Path(__file__).parent / "2242.dot"
    assert input.exists(), "unexpectedly missing test case"
    ref = dot("png", input)

    # now repeat this, expecting it not to change
    for _ in range(20):
        png = dot("png", input)
        assert ref == png, "repeated rendering changed output"


def test_2342():
    """
    using an arrow with size 0 should not trigger an assertion failure
    https://gitlab.com/graphviz/graphviz/-/issues/2342
    """

    # find our collocated test case
    input = Path(__file__).parent / "2342.dot"
    assert input.exists(), "unexpectedly missing test case"

    # run it through Graphviz
    dot("svg", input)


@pytest.mark.skipif(
    is_static_build(),
    reason="dynamic libraries are unavailable to link against in static builds",
)
def test_2356():
    """
    Using `mindist` programmatically in a loop should not cause Windows crashes
    https://gitlab.com/graphviz/graphviz/-/issues/2356
    """

    # find co-located test source
    c_src = (Path(__file__).parent / "2356.c").resolve()
    assert c_src.exists(), "missing test case"

    # run the test
    run_c(c_src, link=["cgraph", "gvc"])


def test_2361():
    """
    using `ortho` and `concentrate` in combination should not cause a crash
    https://gitlab.com/graphviz/graphviz/-/issues/2361
    """

    # find our collocated test case
    input = Path(__file__).parent / "2361.dot"
    assert input.exists(), "unexpectedly missing test case"

    # run it through Graphviz
    dot("png", input)


@pytest.mark.xfail(
    strict=True, reason="https://gitlab.com/graphviz/graphviz/-/issues/2295"
)
def test_2295():
    """
    tooltips should work in PDFs
    https://gitlab.com/graphviz/graphviz/-/issues/2295
    """

    # find our collocated test case
    input = Path(__file__).parent / "2295.dot"
    assert input.exists(), "unexpectedly missing test case"

    # translate it to PDF
    pdf = dot("pdf", input)

    assert re.search(rb"\bhi mom\b", pdf) is not None, "tooltip not propagated to PDF"


@pytest.mark.parametrize("arg", ("--filepath", "-Gimagepath"))
def test_2396(arg: str):
    """
    `--filepath` should work as a replacement for `$GV_FILE_PATH`
    https://gitlab.com/graphviz/graphviz/-/issues/2396
    """

    # use an arbitrary image we have in the tree
    image = Path(__file__).parent / "../cmd/gvedit/images/save.png"
    assert image.exists(), "missing test data"

    # a graph that tries to use the image by relative path
    slash = "/" if arg == "--filepath" else ""
    source = f'graph {{ N[image="{slash}save.png"]; }}'

    # run this through Graphviz
    proc = subprocess.run(
        ["dot", "-Tsvg", f"{arg}={image.parent}"],
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        input=source,
        cwd=Path(__file__).parent,
        universal_newlines=True,
        check=True,
    )

    # work around macOS warnings
    stderr = remove_xtype_warnings(proc.stderr).strip()

    assert stderr == "", "loading an image by relative path produced warnings"

    # whether we used `imagepath` or `filepath` should affect whether we get a leading
    # slash
    if arg == "-Gimagepath":
        assert '"save.png"' in proc.stdout, "incorrect relative path in output"
    else:
        assert '"/save.png"' in proc.stdout, "incorrect relative path in output"


def test_2481():
    """
    `dot` should not exit with a syntax error if keywords are mixed-case
    https://gitlab.com/graphviz/graphviz/-/issues/2481
    """

    # try a simple graph with uppercase characters in 'digraph'
    input = "diGraph { }"

    # ensure this does not trigger warnings
    with subprocess.Popen(
        ["dot"],
        stdin=subprocess.PIPE,
        stderr=subprocess.PIPE,
        universal_newlines=True,
    ) as p:
        _, stderr = p.communicate(input)
        assert p.returncode == 0, "mixed-case keyword was rejected"

    assert "syntax error" not in stderr, "dot displayed a syntax error message"


@pytest.mark.skipif(
    is_static_build(),
    reason="dynamic libraries are unavailable to link against in static builds",
)
def test_2484():
    """
    Graphviz context should not preserve state across calls
    https://gitlab.com/graphviz/graphviz/-/issues/2484
    """

    # find our co-located driver
    c_src = (Path(__file__).parent / "2484.c").resolve()
    assert c_src.exists(), "missing test case"

    # find co-located input to the driver
    dot_src = (Path(__file__).parent / "2484.dot").resolve()
    assert dot_src.exists(), "missing test case"

    # compile and run it
    run_c(
        c_src,
        ["-Kdot", "-Tpng", str(dot_src), "-o", os.devnull],
        link=["cgraph", "gvc"],
    )


def test_package_version():
    """
    The graphviz_version.h header should define a non-empty PACKAGE_VERSION
    """

    # find co-located test source
    c_src = (Path(__file__).parent / "get-package-version.c").resolve()
    assert c_src.exists(), "missing test case"

    # run the test
    package_version, _ = run_c(c_src)

    assert (
        package_version.strip() != ""
    ), "invalid PACKAGE_VERSION in graphviz_version.h"


def test_user_shapes():
    """
    Graphviz should understand how to embed a custom SVG image as a node’s shape
    """

    # find our collocated test case
    input = Path(__file__).parent / "usershape.dot"
    assert input.exists(), "unexpectedly missing test case"

    # ask Graphviz to translate this to SVG
    output = subprocess.check_output(
        ["dot", "-Tsvg", input], cwd=os.path.dirname(__file__), universal_newlines=True
    )

    # the external SVG should have been parsed and is now referenced
    assert '<image xlink:href="usershape.svg" width="62px" height="44px" ' in output


def test_xdot_json():
    """
    check the output of xdot’s JSON API
    """

    # find our collocated C helper
    c_src = Path(__file__).parent / "xdot2json.c"

    # some valid xdot commands to process
    input = "c 9 -#fffffe00 C 7 -#ffffff P 4 0 0 0 36 54 36 54 0"

    # ask our C helper to process this
    output, err = run_c(c_src, input=input, link=["xdot"])
    assert err == ""

    # confirm the output was what we expected
    data = json.loads(output)
    assert data == [
        {"c": "#fffffe00"},
        {"C": "#ffffff"},
        {"P": [0.0, 0.0, 0.0, 36.0, 54.0, 36.0, 54.0, 0.0]},
    ]


@pytest.mark.skipif(which("gvmap") is None, reason="gvmap not available")
def test_gvmap_fclose():
    """
    gvmap should not attempt to fclose(NULL). This example will trigger a crash if
    this bug has been reintroduced and Graphviz is built with ASan support.
    """

    # a reasonable input graph
    input = (
        'graph "Alík: Na vlastní oči" {\n'
        '	graph [bb="0,0,128.9,36",\n'
        "		concentrate=true,\n"
        "		overlap=prism,\n"
        "		start=3\n"
        "	];\n"
        '	node [label="\\N"];\n'
        "	{\n"
        "		bob	[height=0.5,\n"
        '			pos="100.95,18",\n'
        "			width=0.77632];\n"
        "	}\n"
        "	{\n"
        "		alice	[height=0.5,\n"
        '			pos="32.497,18",\n'
        "			width=0.9027];\n"
        "	}\n"
        '	alice -- bob	[pos="65.119,18 67.736,18 70.366,18 72.946,18"];\n'
        "	bob -- alice;\n"
        "}"
    )

    # pass this through gvmap
    proc = subprocess.run(["gvmap"], input=input.encode("utf-8"))

    assert proc.returncode in (0, 1), "gvmap crashed"


@pytest.mark.skipif(which("gvpr") is None, reason="gvpr not available")
def test_gvpr_usage():
    """
    gvpr usage information should be included when erroring on a malformed command
    """

    # create a temporary directory, under which we know no files will exist
    with tempfile.TemporaryDirectory() as tmp:
        # ask GVPR to process a non-existent file
        with subprocess.Popen(
            ["gvpr", "-f", "nofile"],
            stderr=subprocess.PIPE,
            cwd=tmp,
            universal_newlines=True,
        ) as p:
            _, stderr = p.communicate()

            assert p.returncode != 0, "GVPR accepted a non-existent file"

    # the stderr output should have contained full usage instructions
    assert (
        "-o <ofile> - write output to <ofile>; stdout by default" in stderr
    ), "truncated or malformed GVPR usage information"


def test_2225():
    """
    sfdp should not segfault with curved splines
    https://gitlab.com/graphviz/graphviz/-/issues/2225
    """

    # locate our associated test case in this directory
    input = Path(__file__).parent / "2225.dot"
    assert input.exists(), "unexpectedly missing test case"

    # run this through sfdp
    p = subprocess.run(
        ["sfdp", "-Gsplines=curved", "-o", os.devnull, input],
        stderr=subprocess.PIPE,
        universal_newlines=True,
    )

    # if sfdp was built without libgts, it will not handle anything non-trivial
    no_gts_error = "remove_overlap: Graphviz not built with triangulation library"
    if no_gts_error in p.stderr:
        assert p.returncode != 0, "sfdp returned success after an error message"
        return

    p.check_returncode()


def test_2257():
    """
    `$GV_FILE_PATH` being set should prevent Graphviz from running

    `$GV_FILE_PATH` was an environment variable formerly used to implement a file
    system sandboxing policy when Graphviz was exposed to the internet via a web
    server. These days, there are safer and more robust techniques to sandbox
    Graphviz and so `$GV_FILE_PATH` usage has been removed. But if someone
    attempts to use this legacy mechanism, we do not want Graphviz to
    “fail-open,” starting anyway and silently ignoring `$GV_FILE_PATH` giving
    the user the false impression the sandboxing is in force.

    https://gitlab.com/graphviz/graphviz/-/issues/2257
    """

    # locate our associated test case in this directory
    input = Path(__file__).parent / "2257.dot"
    assert input.exists(), "unexpectedly missing test case"

    env = os.environ.copy()
    env["GV_FILE_PATH"] = "/tmp"

    # Graphviz should refuse to process an input file
    with pytest.raises(subprocess.CalledProcessError):
        subprocess.check_call(["dot", "-Tsvg", input, "-o", os.devnull], env=env)


def test_2258():
    """
    'id' attribute should be propagated to all graph children in output
    https://gitlab.com/graphviz/graphviz/-/issues/2258
    """

    # locate our associated test case in this directory
    input = Path(__file__).parent / "2258.dot"
    assert input.exists(), "unexpectedly missing test case"

    # translate this to SVG
    svg = dot("svg", input)

    # load this as XML
    root = ET.fromstring(svg)

    # the output is expected to contain a number of linear gradients, all of which
    # are semantic children of graph marked `id = "G2"`
    gradients = root.findall(".//{http://www.w3.org/2000/svg}linearGradient")
    assert len(gradients) > 0, "no gradients in output"

    for gradient in gradients:
        assert "G2" in gradient.get("id"), "ID was not applied to linear gradients"


def test_2270(tmp_path: Path):
    """
    `-O` should result in the expected output filename
    https://gitlab.com/graphviz/graphviz/-/issues/2270
    """

    # write a simple graph
    input = tmp_path / "hello.gv"
    input.write_text("digraph { hello -> world }", encoding="utf-8")

    # process it with Graphviz
    subprocess.check_call(
        ["dot", "-T", "plain:dot:core", "-O", "hello.gv"], cwd=tmp_path
    )

    # it should have produced output in the expected location
    output = tmp_path / "hello.gv.core.dot.plain"
    assert output.exists(), "-O resulted in an unexpected output filename"


@pytest.mark.skipif(
    is_static_build(),
    reason="dynamic libraries are unavailable to link against in static builds",
)
def test_2272():
    """
    using `agmemread` with an unterminated string should not fail assertions
    https://gitlab.com/graphviz/graphviz/-/issues/2272
    """

    # find co-located test source
    c_src = (Path(__file__).parent / "2272.c").resolve()
    assert c_src.exists(), "missing test case"

    # run the test
    run_c(c_src, link=["cgraph", "gvc"])


def test_2272_2():
    """
    An unterminated string in the source should not crash Graphviz. Variant of
    `test_2272`.
    """

    # a graph with an open string
    graph = 'graph { a[label="abc'

    # process it with Graphviz, which should not crash
    p = subprocess.run(["dot", "-o", os.devnull], input=graph, universal_newlines=True)
    assert p.returncode != 0, "dot accepted invalid input"
    assert p.returncode == 1, "dot crashed"


def test_2282():
    """
    using the `fdp` layout with JSON output should result in valid JSON
    https://gitlab.com/graphviz/graphviz/-/issues/2282
    """

    # locate our associated test case in this directory
    input = Path(__file__).parent / "2282.dot"
    assert input.exists(), "unexpectedly missing test case"

    # translate this to JSON
    output = dot("json", input)

    # confirm this is valid JSON
    json.loads(output)


def test_2283():
    """
    `beautify=true` should correctly space nodes
    https://gitlab.com/graphviz/graphviz/-/issues/2283
    """

    # find our collocated test case
    input = Path(__file__).parent / "2283.dot"
    assert input.exists(), "unexpectedly missing test case"

    # translate this to SVG
    p = subprocess.run(
        ["dot", "-Tsvg", input],
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        universal_newlines=True,
    )

    # if sfdp was built without libgts, it will not handle anything non-trivial
    no_gts_error = "remove_overlap: Graphviz not built with triangulation library"
    if no_gts_error in p.stderr:
        assert p.returncode != 0, "sfdp returned success after an error message"
        return
    p.check_returncode()

    svg = p.stdout

    # parse this into something we can inspect
    root = ET.fromstring(svg)

    # find node N0
    n0s = root.findall(
        ".//{http://www.w3.org/2000/svg}title[.='N0']/../{http://www.w3.org/2000/svg}ellipse"
    )
    assert len(n0s) == 1, "failed to locate node N0"
    n0 = n0s[0]

    # find node N1
    n1s = root.findall(
        ".//{http://www.w3.org/2000/svg}title[.='N1']/../{http://www.w3.org/2000/svg}ellipse"
    )
    assert len(n1s) == 1, "failed to locate node N1"
    n1 = n1s[0]

    # find node N6
    n6s = root.findall(
        ".//{http://www.w3.org/2000/svg}title[.='N6']/../{http://www.w3.org/2000/svg}ellipse"
    )
    assert len(n6s) == 1, "failed to locate node N6"
    n6 = n6s[0]

    # N1 and N6 should not have been drawn on top of each other
    n1_x = float(n1.attrib["cx"])
    n1_y = float(n1.attrib["cy"])
    n6_x = float(n6.attrib["cx"])
    n6_y = float(n6.attrib["cy"])

    def sameish(a: float, b: float) -> bool:
        EPSILON = 0.2
        return -EPSILON < abs(a - b) < EPSILON

    assert not (
        sameish(n1_x, n6_x) and sameish(n1_y, n6_y)
    ), "N1 and N6 placed identically"

    # use the Law of Cosines to compute the angle between N0→N1 and N0→N6
    n0_x = float(n0.attrib["cx"])
    n0_y = float(n0.attrib["cy"])
    n0_n1_dist = math.dist((n0_x, n0_y), (n1_x, n1_y))
    n0_n6_dist = math.dist((n0_x, n0_y), (n6_x, n6_y))
    n1_n6_dist = math.dist((n1_x, n1_y), (n6_x, n6_y))
    angle = math.acos(
        (n0_n1_dist**2 + n0_n6_dist**2 - n1_n6_dist**2) / (2 * n0_n1_dist * n0_n6_dist)
    )

    number_of_radial_nodes = 6
    assert sameish(
        angle, 2 * math.pi / number_of_radial_nodes
    ), "nodes not placed evenly"


def test_2285():
    """
    using the `svg_inline` output should result in SVG you can inline to HTML
    https://gitlab.com/graphviz/graphviz/-/issues/2285
    """

    # locate our associated test case in this directory
    input = Path(__file__).parent / "2285.dot"
    assert input.exists(), "unexpectedly missing test case"

    # translate this to JSON
    output = dot("svg_inline", input)

    assert "<?xml" not in output, "<?xml in output"
    assert "<!DOCTYPE" not in output, "<?xml in output"
    assert "xmlns" not in output, "xmlns in output"
    assert "<svg" in output, "<svg not in output"


@pytest.mark.skipif(which("gxl2gv") is None, reason="gxl2gv not available")
def test_2300_1():
    """
    translating GXL with an attribute `name` should not crash
    https://gitlab.com/graphviz/graphviz/-/issues/2300
    """

    # locate our associated test case containing a node attribute `name`
    input = Path(__file__).parent / "2300.gxl"
    assert input.exists(), "unexpectedly missing test case"

    # ask `gxl2gv` to process this
    subprocess.check_call(["gxl2gv", input])


def test_2307():
    """
    'id' attribute should be propagated to 'url' links in SVG output
    https://gitlab.com/graphviz/graphviz/-/issues/2307
    """

    # locate our associated test case in this directory
    input = Path(__file__).parent / "2258.dot"
    assert input.exists(), "unexpectedly missing test case"

    # translate this to SVG
    svg = dot("svg", input)

    # load this as XML
    root = ET.fromstring(svg)

    # the output is expected to contain a number of polygons, any of which have
    # `url` fills should include the ID “G2”
    polygons = root.findall(".//{http://www.w3.org/2000/svg}polygon")
    assert len(polygons) > 0, "no polygons in output"

    for polygon in polygons:
        m = re.match(r"url\((?P<url>.*)\)$", polygon.get("fill"))
        if m is None:
            continue
        assert (
            re.search(r"\bG2_", m.group("url")) is not None
        ), "ID G2 was not applied to polygon fill url"


def test_2325():
    """
    using more than 63 styles and/or more than 128 style bytes should not trigger
    an out-of-bounds memory read
    https://gitlab.com/graphviz/graphviz/-/issues/2325
    """

    # locate our associated test case in this directory
    input = Path(__file__).parent / "2325.dot"
    assert input.exists(), "unexpectedly missing test case"

    # run it through Graphviz
    dot("svg", input)


@pytest.mark.skipif(shutil.which("groff") is None, reason="groff not available")
def test_2341():
    """
    PIC backend should generate correct comments
    https://gitlab.com/graphviz/graphviz/-/issues/2341
    """

    # a simple graph
    source = "digraph { a -> b; }"

    # generate PIC from this
    pic = dot("pic", source=source)

    # run this through groff
    groffed = subprocess.check_output(
        ["groff", "-Tascii", "-p"], input=pic, universal_newlines=True
    )

    # it should not contain any comments
    assert (
        re.search(r"^\s*#", groffed) is None
    ), "Graphviz comment remains in groff output"


def test_2352():
    """
    referencing an all-one-line external SVG file should work
    https://gitlab.com/graphviz/graphviz/-/issues/2352
    """

    # locate our associated test case in this directory
    input = Path(__file__).parent / "2352.dot"
    assert input.exists(), "unexpectedly missing test case"

    # translate it to SVG
    svg = subprocess.check_output(
        ["dot", "-Tsvg", input], cwd=Path(__file__).parent, universal_newlines=True
    )

    assert '<image xlink:href="EDA.svg" ' in svg, "external file reference missing"


def test_2352_1():
    """
    variant of 2352 with a leading space in front of `<svg`
    https://gitlab.com/graphviz/graphviz/-/issues/2352
    """

    # locate our associated test case in this directory
    input = Path(__file__).parent / "2352_1.dot"
    assert input.exists(), "unexpectedly missing test case"

    # translate it to SVG
    svg = subprocess.check_output(
        ["dot", "-Tsvg", input], cwd=Path(__file__).parent, universal_newlines=True
    )

    assert '<image xlink:href="EDA_1.svg" ' in svg, "external file reference missing"


def test_2352_2():
    """
    variant of 2352 that spaces viewBox such that it is on a 200-character line
    boundary
    https://gitlab.com/graphviz/graphviz/-/issues/2352
    """

    # locate our associated test case in this directory
    input = Path(__file__).parent / "2352_2.dot"
    assert input.exists(), "unexpectedly missing test case"

    # translate it to SVG
    svg = subprocess.check_output(
        ["dot", "-Tsvg", input], cwd=Path(__file__).parent, universal_newlines=True
    )

    assert '<image xlink:href="EDA_2.svg" ' in svg, "external file reference missing"


def test_2355():
    """
    Using >127 layers should not crash Graphviz
    https://gitlab.com/graphviz/graphviz/-/issues/2355
    """

    # construct a graph with 128 layers
    graph = io.StringIO()
    graph.write("digraph {\n")
    layers = ":".join(f"l{i}" for i in range(128))
    graph.write(f'  layers="{layers}";\n')
    for i in range(128):
        graph.write(f'  n{i}[layer="l{i}"];\n')
    graph.write("}\n")

    # process this with dot
    dot("svg", source=graph.getvalue())


@pytest.mark.xfail(strict=True)  # FIXME
def test_2368():
    """
    routesplines should not corrupt its `prev` and `next` indices
    https://gitlab.com/graphviz/graphviz/-/issues/2368
    """

    # locate our associated test case in this directory
    input = Path(__file__).parent / "2368.dot"
    assert input.exists(), "unexpectedly missing test case"

    # run it through Graphviz
    dot("svg", input)


@pytest.mark.skipif(shutil.which("tclsh") is None, reason="tclsh not available")
def test_2370():
    """
    tcldot should have a version number TCL accepts
    https://gitlab.com/graphviz/graphviz/-/issues/2370
    """

    # if this appears to be an ASan-enabled CI job, teach `tclsh` to load ASan’s
    # supporting library because it is otherwise unaware that Tcldot depends on this
    # being loaded first
    env = os.environ.copy()
    if re.search(r"\basan\b", os.environ.get("CI_JOB_NAME", "").lower()):
        cc = os.environ.get("CC", "gcc")
        libasan = subprocess.check_output(
            [cc, "-print-file-name=libasan.so"], universal_newlines=True
        ).strip()
        print(f"setting LD_PRELOAD={libasan}")
        env["LD_PRELOAD"] = libasan

    # ask TCL to import the Graphviz package
    response = subprocess.check_output(
        ["tclsh"],
        stderr=subprocess.STDOUT,
        input="package require Tcldot;",
        universal_newlines=True,
        env=env,
    )

    assert (
        "error reading package index file" not in response
    ), "tcldot cannot be loaded by TCL"


def test_2371():
    """
    Large graphs should not cause rectangle area calculation overflows
    https://gitlab.com/graphviz/graphviz/-/issues/2371
    """

    # locate our associated test case in this directory
    input = Path(__file__).parent / "2371.dot"
    assert input.exists(), "unexpectedly missing test case"

    # run it through Graphviz
    subprocess.check_call(["dot", "-Tsvg", "-Knop2", "-o", os.devnull, input])


@pytest.mark.skipif(
    platform.system() == "Windows",
    reason="gvplugin_list symbol is not exposed on Windows",
)
def test_2375():
    """
    `gvplugin_list` should return full plugin names
    https://gitlab.com/graphviz/graphviz/-/issues/2375
    """

    # find co-located test source
    c_src = (Path(__file__).parent / "2375.c").resolve()
    assert c_src.exists(), "missing test case"

    # run the test
    run_c(c_src, link=["gvc"])


def test_2377():
    """
    3 letter hex color codes should be accepted
    https://gitlab.com/graphviz/graphviz/-/issues/2377
    """

    # run some 6 letter color input through Graphviz
    input = 'digraph { n [color="#cc0000" fillcolor="#ffcc00" style=filled] }'
    svg1 = dot("svg", source=input)

    # try the equivalent with 3 letter colors
    input = 'digraph { n [color="#c00" fillcolor="#fc0" style=filled] }'
    svg2 = dot("svg", source=input)

    assert svg1 == svg2, "3 letter hex colors were not translated correctly"


def test_2390():
    """
    using an out of range `xdotversion` should not crash Graphviz
    https://gitlab.com/graphviz/graphviz/-/issues/2390
    """

    # some input with an invalid large `xdotversion`
    input = 'graph { xdotversion=99; n[label="hello world"]; }'

    # run this through Graphviz
    dot("xdot", source=input)


def test_2391():
    """
    `nslimit1=0` should not cause Graphviz to crash
    https://gitlab.com/graphviz/graphviz/-/issues/2391
    """

    # locate our associated test case in this directory
    input = Path(__file__).parent / "2391.dot"
    assert input.exists(), "unexpectedly missing test case"

    # run it through Graphviz
    dot("svg", input)


def test_2391_1():
    """
    `nslimit1=0` with a label should not cause Graphviz to crash
    https://gitlab.com/graphviz/graphviz/-/issues/2391
    """

    # locate our associated test case in this directory
    input = Path(__file__).parent / "2391_1.dot"
    assert input.exists(), "unexpectedly missing test case"

    # run it through Graphviz
    dot("svg", input)


@pytest.mark.xfail(
    platform.system() == "Windows" and not is_mingw(),
    reason="cannot link Agdirected on Windows",
    strict=True,
)  # FIXME
def test_2397():
    """
    escapes in strings should be handled correctly
    https://gitlab.com/graphviz/graphviz/-/issues/2397
    """

    # find co-located test source
    c_src = (Path(__file__).parent / "2397.c").resolve()
    assert c_src.exists(), "missing test case"

    # run this to generate a graph
    source, _ = run_c(c_src, link=["cgraph", "gvc"])

    # this should have produced a valid graph
    dot("svg", source=source)


def test_2397_1():
    """
    a variant of test_2397 that confirms the same works via the command line
    https://gitlab.com/graphviz/graphviz/-/issues/2397
    """

    source = 'digraph { a[label="foo\\\\\\"bar"]; }'

    # run this through dot
    output = dot("dot", source=source)

    # the output should be valid dot
    dot("svg", source=output)


@pytest.mark.skipif(shutil.which("shellcheck") is None, reason="shellcheck unavailable")
def test_2404():
    """
    shell syntax used by gvmap should be correct
    https://gitlab.com/graphviz/graphviz/-/issues/2404
    """
    gvmap_sh = Path(__file__).parent / "../cmd/gvmap/gvmap.sh"
    subprocess.check_call(["shellcheck", "-S", "error", gvmap_sh])


def test_2406():
    """
    arrow types like `invdot` and `onormalonormal` should be displayed correctly
    https://gitlab.com/graphviz/graphviz/-/issues/2406
    """

    # locate our associated test case in this directory
    input = Path(__file__).parent / "2406.dot"
    assert input.exists(), "unexpectedly missing test case"

    # run it through Graphviz
    output = dot("svg", input)

    # the rounded hollows should be present
    assert re.search(r"\bellipse\b", output), "missing element of invdot arrow"


@pytest.mark.parametrize("source", ("2413_1.dot", "2413_2.dot"))
def test_2413(source: str):
    """
    graphs that induce an edge length > 65535 should be supported
    https://gitlab.com/graphviz/graphviz/-/issues/2413
    """

    # locate our associated test case in this directory
    input = Path(__file__).parent / source
    assert input.exists(), "unexpectedly missing test case"

    # run it through Graphviz
    proc = subprocess.run(
        ["dot", "-Tsvg", "-o", os.devnull, input],
        stderr=subprocess.PIPE,
        check=True,
        universal_newlines=True,
    )

    # work around macOS warnings
    stderr = remove_xtype_warnings(proc.stderr).strip()

    # no warnings should have been generated
    assert stderr == "", "long edges resulted in a warning"


def test_2429():
    """
    the vt target should be usable
    https://gitlab.com/graphviz/graphviz/-/issues/2429
    """

    # a basic graph
    source = "digraph { a -> b; }"

    # run it through Graphviz
    dot("vt", source=source)


@pytest.mark.skipif(which("nop") is None, reason="nop not available")
@pytest.mark.xfail(
    strict=True, reason="https://gitlab.com/graphviz/graphviz/-/issues/2436"
)
def test_2436():
    """
    nop should preserve empty labels
    https://gitlab.com/graphviz/graphviz/-/issues/2436
    """

    # locate our associated test case in this directory
    input = Path(__file__).parent / "2436.dot"
    assert input.exists(), "unexpectedly missing test case"

    # run it through nop
    nop = which("nop")
    output = subprocess.check_output([nop, input], universal_newlines=True)

    # the empty label should be present
    assert re.search(r'\blabel\s*=\s*""', output), "empty label was not preserved"


@pytest.mark.xfail(
    strict=True, reason="https://gitlab.com/graphviz/graphviz/-/issues/2434"
)
def test_2434():
    """
    the order in which `agmemread` and `gvContext` calls are made should have
    no impact
    https://gitlab.com/graphviz/graphviz/-/issues/2434
    """

    # find co-located test source
    c_src = (Path(__file__).parent / "2434.c").resolve()
    assert c_src.exists(), "missing test case"

    # generate an SVG by calling `gvContext` first
    before, _ = run_c(c_src, ["before"], link=["cgraph", "gvc"])

    # generate an SVG by calling `gvContext` second
    after, _ = run_c(c_src, ["after"], link=["cgraph", "gvc"])

    # resulting images should be identical
    assert before == after, "agmemread/gvContext ordering affected image output"


def test_2437():
    """
    both an arrowhead and an arrowtail shall be created when using dir=both,
    compass ports, an edge default attribute and rank=same
    https://gitlab.com/graphviz/graphviz/-/issues/2437
    """

    # locate our associated test case in this directory
    input = Path(__file__).parent / "2437.dot"
    assert input.exists(), "unexpectedly missing test case"

    # translate this to SVG
    svg = dot("svg", input)

    # load this as XML
    root = ET.fromstring(svg)

    # The output is expected to contain tree polygons. The graph "background"
    # polygon, the arrowhead polygon and the arrowtail polygon.
    polygons = root.findall(".//{http://www.w3.org/2000/svg}polygon")

    assert len(polygons) == 3, "wrong number of polygons in output"


@pytest.mark.xfail(
    strict=True, reason="https://gitlab.com/graphviz/graphviz/-/issues/2416"
)
def test_2416():
    """
    `splines=curved` should not affect arrow directions
    https://gitlab.com/graphviz/graphviz/-/issues/2416
    """

    # an input graph that provokes the problem
    input = "digraph G { splines=curved; b -> a; a -> b; }"

    # run it through Graphviz
    output = dot("json", source=input)
    data = json.loads(output)

    edges = data["edges"]
    assert len(edges) == 2, "unexpected number of output edges"

    # extract the height each edge’s arrow starts at
    y_1 = edges[0]["_hdraw_"][3]["points"][0][1]
    y_2 = edges[1]["_hdraw_"][3]["points"][0][1]

    # assuming the graph is vertical, these should not be too close
    assert abs(y_1 - y_2) > 1, "edge arrows appear to be drawn next to the same node"


@pytest.mark.skipif(which("gvpr") is None, reason="GVPR not available")
def test_2454():
    """
    gvpr should support sscanf
    https://gitlab.com/graphviz/graphviz/-/issues/2454
    """

    # an input graph that provokes the problem
    input = "graph x{a -- {b c}}"

    # run it through Graphviz
    output = dot("dot", source=input)

    # run it through gvpr
    program = Path(__file__).parent / "2454.gvpr"
    with subprocess.Popen(
        ["gvpr", "-cf", program], stdin=subprocess.PIPE, universal_newlines=True
    ) as p:
        p.communicate(output)
        assert p.returncode == 0, "gvpr failed"


@pytest.mark.skipif(which("twopi") is None, reason="twopi not available")
@pytest.mark.xfail(
    strict=True, reason="https://gitlab.com/graphviz/graphviz/-/issues/2457"
)
def test_2457():
    """
    node definition order should not affect twopi’s layout
    https://gitlab.com/graphviz/graphviz/-/issues/2457
    """

    # locate our associated test cases in this directory
    case1 = Path(__file__).parent / "2457_1.dot"
    assert case1.exists(), "unexpectedly missing test case"
    case2 = Path(__file__).parent / "2457_2.dot"
    assert case2.exists(), "unexpectedly missing test case"

    # tweak the environment to force deterministic PDF generation
    env = os.environ.copy()
    env["SOURCE_DATE_EPOCH"] = "0"

    # generate PDFs
    pdf1 = subprocess.check_output(["twopi", "-Tpdf", case1], env=env)
    pdf2 = subprocess.check_output(["twopi", "-Tpdf", case2], env=env)

    assert pdf1 == pdf2, "node definition order affected PDF generation"


@pytest.mark.xfail(
    strict=True, reason="https://gitlab.com/graphviz/graphviz/-/issues/2458"
)
def test_2458():
    """
    `pack=true` should not result in edge labels disappearing
    https://gitlab.com/graphviz/graphviz/-/issues/2458
    """

    # locate our associated test case in this directory
    input = Path(__file__).parent / "2458.dot"
    assert input.exists(), "unexpectedly missing test case"

    # run it through Graphviz
    output = dot("svg", input)

    # the edge label should be present
    assert re.search(r"\bconnected\b", output), "missing edge label"


def test_2460():
    """
    labels involving back slashes should come out correctly in JSON
    https://gitlab.com/graphviz/graphviz/-/issues/2460
    """

    # locate our associated test case in this directory
    input = Path(__file__).parent / "2460.dot"
    assert input.exists(), "unexpectedly missing test case"

    # run it through Graphviz
    output = dot("json", input)
    data = json.loads(output)

    assert (
        data["objects"][0]["_ldraw_"][2]["text"]
        == r"double back slash in label \\. End should be the last word - End"
    ), "back slashes in labels handled incorrectly"


@pytest.mark.xfail(
    strict=platform.system() != "Windows",
    reason="https://gitlab.com/graphviz/graphviz/-/issues/2470",
)
def test_2470():
    """
    another “trouble in init_rank variant”
    https://gitlab.com/graphviz/graphviz/-/issues/2470
    """

    # locate our associated test case in this directory
    input = Path(__file__).parent / "2470.dot"
    assert input.exists(), "unexpectedly missing test case"

    # run it through Graphviz
    dot("ps", input)


@pytest.mark.xfail(
    reason="https://gitlab.com/graphviz/graphviz/-/issues/2471",
    strict=True,
)
def test_2471():
    """
    another “trouble in init_rank variant”
    https://gitlab.com/graphviz/graphviz/-/issues/2471
    """

    # locate our associated test case in this directory
    input = Path(__file__).parent / "2471.dot"
    assert input.exists(), "unexpectedly missing test case"

    # run it through Graphviz
    dot("png", input)


@pytest.mark.xfail(
    is_rocky_8(),
    reason="Cairo is <v1.16 or malfunctions",
    strict=True,
)
def test_2473_1():
    """
    `SOURCE_DATE_EPOCH` should be usable to suppress timestamps
    https://gitlab.com/graphviz/graphviz/-/issues/2473
    """

    # a trivial graph
    graph = "graph { a -- b }".encode("utf-8")

    # set an epoch
    env = os.environ.copy()
    env["SOURCE_DATE_EPOCH"] = "60"

    # generate a PDF
    first_run = subprocess.check_output(["dot", "-Tpdf"], input=graph, env=env)

    # wait long enough for the current time to change
    time.sleep(2)

    # generate another PDF
    second_run = subprocess.check_output(["dot", "-Tpdf"], input=graph, env=env)

    assert (
        first_run == second_run
    ), "PDF output is dependent on current time even when $SOURCE_DATE_EPOCH is set"


def test_2473_2():
    """
    When handling `SOURCE_DATE_EPOCH`, from
    https://reproducible-builds.org/specs/source-date-epoch/:

       If the value is malformed, the build process SHOULD exit with a non-zero
       error code.

    https://gitlab.com/graphviz/graphviz/-/issues/2473
    """

    # set up an invalid epoch
    env = os.environ.copy()
    env["SOURCE_DATE_EPOCH"] = "foo"

    # confirm Graphviz rejects this
    with pytest.raises(subprocess.CalledProcessError):
        subprocess.run(
            ["dot", "-Tpdf", "-o", os.devnull],
            input="graph { a -- b }",
            env=env,
            check=True,
            encoding="utf-8",
            universal_newlines=True,
        )


def test_2476():
    """
    tweaking `mclimit` should not lead to a “trouble in init_rank” failure
    https://gitlab.com/graphviz/graphviz/-/issues/2476
    """

    # locate our associated test case in this directory
    input = Path(__file__).parent / "2476.dot"
    assert input.exists(), "unexpectedly missing test case"

    # run it through Graphviz
    subprocess.check_call(["dot", "-Tsvg", "-Gmclimit=0.5", "-o", os.devnull, input])


def test_2490():
    """
    the `crow` arrow shall be correctly placed and orientated when ports are used
    https://gitlab.com/graphviz/graphviz/-/issues/2490
    """

    # locate our associated test case in this directory
    input = Path(__file__).parent / "2490.dot"
    assert input.exists(), "unexpectedly missing test case"

    # translate this to SVG
    svg = dot("svg", input)

    # load this as XML
    root = ET.fromstring(svg)

    # The output is expected to contain three polygons, of which the two last
    # are the `crow` arrow shapes of the edge head and tail. Except for the
    # crow's "toes", the corners of these are expected to have the same x
    # position as the nodes' centers. The "toes" are expected to have x
    # positions half the width more or less than the nodes' centers.
    ellipses = root.findall(".//{http://www.w3.org/2000/svg}ellipse")
    assert len(ellipses) == 2, "wrong number of ellipses in output"
    cx = float(ellipses[0].get("cx"))
    assert float(ellipses[1].get("cx")) == cx

    polygons = root.findall(".//{http://www.w3.org/2000/svg}polygon")
    assert len(polygons) == 3, "wrong number of polygons in output"
    for polygon_index, polygon in enumerate(polygons):
        points_attr = polygon.get("points")
        point_pair_strs = points_attr.split(" ")
        points = [point_pair_str.split(",") for point_pair_str in point_pair_strs]
        if polygon_index == 0:
            assert len(points) == 5
            # ignore the graph polygon
            continue
        assert len(points) == 9
        for point_index, point in enumerate(points):
            x = float(point[0])
            crow_width = 9
            expected_crow_tip_and_shaft_x = cx
            expected_crow_toe_left_x = cx - crow_width / 2
            expected_crow_toe_right_x = cx + crow_width / 2
            expected_first_crow_toe_x = (
                expected_crow_toe_left_x
                if polygon_index == 1
                else expected_crow_toe_right_x
            )
            expected_second_crow_toe_x = (
                expected_crow_toe_right_x
                if polygon_index == 1
                else expected_crow_toe_left_x
            )
            if point_index in [0, 2, 3, 4, 5, 6, 8]:
                assert x == expected_crow_tip_and_shaft_x
            elif point_index == 1:
                assert x == expected_first_crow_toe_x
            elif point_index == 7:
                assert x == expected_second_crow_toe_x


@pytest.mark.skipif(which("gv2gml") is None, reason="gv2gml not available")
def test_2493():
    """
    `gv2gml` should support the yWorks.com variant of GML
    https://gitlab.com/graphviz/graphviz/-/issues/2493
    """

    # a trivial graph with a colored label
    src = 'graph { a -- b[label="foo", fontcolor="red"]; }'

    # pass this through `gv2gml`
    gml = subprocess.check_output(["gv2gml", "-y"], input=src, universal_newlines=True)

    assert (
        re.search(r"\bfontcolor\b", gml) is None
    ), "gv2gml emitted 'fontcolor' when in yWorks.com mode"
    assert (
        re.search(r"\bcolor\b", gml) is not None
    ), "gv2gml did not emit LabelGraphics 'color' attribute"


def test_2497():
    """
    graph rendering should be deterministic
    https://gitlab.com/graphviz/graphviz/-/issues/2497
    """

    # get our baseline reference
    input = Path(__file__).parent / "2497.dot"
    assert input.exists(), "unexpectedly missing test case"
    ref = dot("svg", input)

    # now repeat this, expecting it not to change
    for _ in range(20):
        out = dot("svg", input)
        assert ref == out, "repeated rendering changed output"


def test_2502():
    """
    unicode labels should be usable
    https://gitlab.com/graphviz/graphviz/-/issues/2502
    """

    # locate our associated test case in this directory
    input = Path(__file__).parent / "2502.dot"
    assert input.exists(), "unexpectedly missing test case"

    # run it through Graphviz
    dot("dot", input)


@pytest.mark.xfail(
    strict=True, reason="https://gitlab.com/graphviz/graphviz/-/issues/2516"
)
def test_2516():
    """
    errors in HTML labels should result in a message with correct line number
    https://gitlab.com/graphviz/graphviz/-/issues/2516
    """

    # locate our associated test case in this directory
    input = Path(__file__).parent / "2516.dot"
    assert input.exists(), "unexpectedly missing test case"

    # run it through Graphviz
    proc = subprocess.run(
        ["dot", "-Tsvg", "-o", os.devnull, input],
        stderr=subprocess.PIPE,
        universal_newlines=True,
    )

    assert proc.returncode != 0, "malformed HTML label was accepted"

    assert (
        re.search(r"\bline 1\b", proc.stderr) is None
    ), "incorrect line number in error message"

    assert (
        re.search(r"\bline 2\b", proc.stderr) is not None
    ), "correct line number missing from error message"


@pytest.mark.parametrize(
    "testcase",
    (
        "705.dot",
        pytest.param(
            "2521.dot",
            marks=pytest.mark.xfail(
                strict=False,
                reason="https://gitlab.com/graphviz/graphviz/-/issues/2521",
            ),
        ),
        "2521_1.dot",
    ),
)
def test_2521(testcase: str):
    """
    `newrank=false` should reset to the default behavior
    https://gitlab.com/graphviz/graphviz/-/issues/2521
    """

    # locate our associated test case in this directory
    input = Path(__file__).parent / testcase
    assert input.exists(), "unexpectedly missing test case"

    # process this with and without `newrank=true`
    off = subprocess.check_output(["dot", "-Tpng", input])
    on = subprocess.check_output(["dot", "-Gnewrank=true", "-Tpng", input])

    assert off != on, "-Gnewrank=true had no effect"

    # we should be able to reset `newrank` with an explicit setting
    force_off = subprocess.check_output(["dot", "-Gnewrank=false", "-Tpng", input])
    assert force_off == off, "-Gnewrank=false did not reset the default"


@pytest.mark.xfail(
    is_macos(), strict=True, reason="https://gitlab.com/graphviz/graphviz/-/issues/2538"
)
def test_2538():
    """
    `chanSearch` assertion on `cp` should not fail
    https://gitlab.com/graphviz/graphviz/-/issues/2538
    """

    # locate our associated test case in this directory
    input = Path(__file__).parent / "2538.dot"
    assert input.exists(), "unexpectedly missing test case"

    # run it through Graphviz
    dot("dot", input)


@pytest.mark.skipif(which("sfdp") is None, reason="sfdp not available")
def test_2556():
    """
    sfdp should not fail a GTS assertion
    https://gitlab.com/graphviz/graphviz/-/issues/2556
    """

    # locate our associated test case in this directory
    input = Path(__file__).parent / "2556.dot"
    assert input.exists(), "unexpectedly missing test case"

    # run this through sfdp
    sfdp = which("sfdp")
    p = subprocess.run(
        [sfdp, "-Tpng", "-o", os.devnull, input],
        stderr=subprocess.PIPE,
        universal_newlines=True,
    )

    # if sfdp was built without libgts, it will not handle anything non-trivial
    no_gts_error = "remove_overlap: Graphviz not built with triangulation library"
    if no_gts_error in p.stderr:
        assert p.returncode != 0, "sfdp returned success after an error message"
        return

    p.check_returncode()


def test_2559():
    """
    `concentrate=true` should actually concentrate edges
    https://gitlab.com/graphviz/graphviz/-/issues/2559
    """

    # locate our associated test case in this directory
    input = Path(__file__).parent / "2559.dot"
    assert input.exists(), "unexpectedly missing test case"

    # convert this to JSON
    layout = dot("json", input)
    parsed = json.loads(layout)

    # the last edge, d→b, should be drawn as a curve rather than a straight edge
    assert not parsed["edges"][-1]["pos"].startswith(
        "e"
    ), "concentrated edge drawn as a regular straight edge"


@pytest.mark.skipif(which("fdp") is None, reason="fdp not available")
def test_2563():
    """
    `overlap` parameters should generate different results
    https://gitlab.com/graphviz/graphviz/-/issues/2563
    """

    # locate our associated test case in this directory
    input = Path(__file__).parent / "2563.dot"
    assert input.exists(), "unexpectedly missing test case"

    # try various `overlap=…` values
    results: Set[str] = set()
    for overlap in ("scale", "scalexy"):
        # run this through fdp
        fdp = which("fdp")
        p = subprocess.run(
            [fdp, f"-Goverlap={overlap}", input],
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            universal_newlines=True,
        )

        # if fdp was built without libgts, it will not handle anything non-trivial
        no_gts_error = "remove_overlap: Graphviz not built with triangulation library"
        if no_gts_error in p.stderr:
            assert p.returncode != 0, "fdp returned success after an error message"
            return
        p.check_returncode()

        # remove the overlap parameter itself, that would otherwise cause each
        # output to be unique
        output = re.sub(r"\boverlap\s*=\s*scale(xy)?\b", "", p.stdout)

        assert (
            output not in results
        ), "altering `overlap` attribute did not affect output"
        results.add(output)


def test_2564():
    """
    `overlap="scale"` should not result in all nodes overlapping
    https://gitlab.com/graphviz/graphviz/-/issues/2564
    """

    # locate our associated test case in this directory
    input = Path(__file__).parent / "2564.dot"
    assert input.exists(), "unexpectedly missing test case"

    # convert this to JSON
    layout = subprocess.check_output(
        ["dot", "-Kneato", "-Tjson", input], universal_newlines=True
    )
    parsed = json.loads(layout)

    # nodes should not be on top of one another
    starts = []
    for node in parsed["objects"]:
        start = re.match(r"(?P<start>\d+(\.\d+)?,\d+(\.\d+)?)\b", node["pos"]).group(
            "start"
        )
        assert start not in starts, "nodes overlap"
        starts += [start]


@pytest.mark.skipif(shutil.which("tclsh") is None, reason="tclsh not available")
@pytest.mark.skipif(
    platform.system() == "Windows",
    reason="pexpect.spawn is not available on Windows "
    "(https://pexpect.readthedocs.io/en/stable/overview.html#pexpect-on-windows)",
)
def test_2568():
    """
    tags used in TCL output should be usable for later lookup
    https://gitlab.com/graphviz/graphviz/-/issues/2568
    """

    # locate the TCL input for this test
    prelude = Path(__file__).parent / "2568.tcl"
    assert prelude.exists(), "unexpectedly missing test collateral"

    # if this appears to be an ASan-enabled CI job, teach `tclsh` to load ASan’s
    # supporting library because it is otherwise unaware that Tcldot depends on this
    # being loaded first
    env = os.environ.copy()
    if re.search(r"\basan\b", os.environ.get("CI_JOB_NAME", "").lower()):
        cc = os.environ.get("CC", "gcc")
        libasan = subprocess.check_output(
            [cc, "-print-file-name=libasan.so"], universal_newlines=True
        ).strip()
        print(f"setting LD_PRELOAD={libasan}")
        env["LD_PRELOAD"] = libasan

    # startup TCL and load our graph setup code
    proc = pexpect.spawn("tclsh", timeout=1, env=env)
    proc.expect("% ")
    proc.sendline(f'source "{shlex.quote(str(prelude))}"')

    # look for tags to query
    while True:
        index = proc.expect(
            [
                "invalid command name",
                re.compile(rb"-tags {\d(?P<tag>(edge|node)0x[\da-fA-F]+)}"),
                pexpect.TIMEOUT,
            ]
        )

        # stdout and stderr are multiplexed onto the same stream by `pexpect`, so if one
        # of the commands we previously entered was not recognized, we will see an error
        # at the end of the output stream
        assert index != 0, "at least one tag was not recognized"

        # if we got no output within 1s, assume we are done and try to neatly exit
        if index == 2:
            proc.sendeof()
            proc.wait()
            break

        tag = proc.match.group("tag").decode("utf-8")

        # …try to look up its corresponding entities
        if tag.startswith("edge"):
            cmd = "listnodes"
        else:
            cmd = "listedges"
        proc.sendline(f"{tag} {cmd}")


@pytest.mark.skipif(which("sfdp") is None, reason="sfdp not available")
def test_2572():
    """
    sfdp should be able to find non-overlapping layouts
    https://gitlab.com/graphviz/graphviz/-/issues/2572
    """

    # locate our associated test case in this directory
    input = Path(__file__).parent / "2572.dot"
    assert input.exists(), "unexpectedly missing test case"

    # run this through SFDP and convert this to JSON
    sfdp = which("sfdp")
    layout = subprocess.check_output(
        [sfdp, "-Kneato", "-Tjson", input], universal_newlines=True
    )
    parsed = json.loads(layout)

    @dataclasses.dataclass
    class Box:
        """
        a geometric rectangle, defined by two of its corners
        """

        llx: float  # lower left X coordinate
        lly: float  # lower left Y coordinate
        urx: float  # upper right X coordinate
        ury: float  # upper right Y coordinate

        def overlaps(self, other: "Box") -> bool:
            """
            do we intersect the given box?
            """
            if self.llx > other.urx:
                return False
            if self.lly > other.ury:
                return False
            if self.urx < other.llx:
                return False
            if self.ury < other.lly:
                return False
            return True

    nodes: List[Box] = []
    for obj in parsed["objects"]:
        # extract the ellipse drawn for this node
        ellipses = [e for e in obj["_draw_"] if e["op"] == "e"]
        assert len(ellipses) == 1, "could not find ellipse for node"
        center_x, center_y, width, height = ellipses[0]["rect"]

        assert center_x >= width, "ellipse extends into negative X space"
        assert center_y >= height, "ellipse extends into negative Y space"

        node = Box(
            center_x - width, center_y - height, center_x + width, center_y + height
        )

        assert not any(n.overlaps(node) for n in nodes), "nodes overlap"

        nodes.append(node)


@pytest.mark.skipif(which("gvpr") is None, reason="GVPR not available")
def test_2577():
    """
    accessing an uninitialized string should not corrupt GVPR’s state
    https://gitlab.com/graphviz/graphviz/-/issues/2577
    """

    # find our collocated test case
    program = Path(__file__).parent / "2577.gvpr"
    assert program.exists(), "unexpectedly missing test case"

    # run it through GVPR
    output = gvpr(program)

    # it should have printed an empty string for the uninitialized attribute
    assert (
        "Before...\n<>\nAfter." in output
    ), "incorrect handling of uninitialized attribute in GVPR"


@pytest.mark.skipif(which("gvpr") is None, reason="GVPR not available")
def test_2577_1():
    """
    a variant of `test_2577` that does not involve attribute access
    https://gitlab.com/graphviz/graphviz/-/issues/2577
    """

    # run GVPR on a simple program
    gvprbin = which("gvpr")
    output = subprocess.check_output(
        [gvprbin, 'BEGIN { printf("hello%s world\\n", ""); }'],
        stdin=subprocess.DEVNULL,
        universal_newlines=True,
    )

    # it should have printed the expected text
    assert output == "hello world\n", "gvpr cannot handle empty strings to printf"


@pytest.mark.skipif(which("gvgen") is None, reason="gvgen not available")
def test_2588():
    """
    `gvgen` should not crash when producing random graphs
    https://gitlab.com/graphviz/graphviz/-/issues/2588
    """

    gvgen = which("gvgen")

    # this execution depends on random numbers, so we need to run many times to
    # have a chance of provoking the bug
    for _ in range(200):
        subprocess.check_call([gvgen, "-R", "20"], stdout=subprocess.DEVNULL)


@pytest.mark.skipif(which("edgepaint") is None, reason="edgepaint not available")
@pytest.mark.skipif(which("gvgen") is None, reason="gvgen not available")
def test_2591():
    """
    edgepaint color schemes should do something
    https://gitlab.com/graphviz/graphviz/-/issues/2591
    """

    # make an input graph
    gvgen = which("gvgen")
    graph = subprocess.check_output([gvgen, "-k", "5"], universal_newlines=True)

    # run it through neato
    laidout = subprocess.check_output(
        ["dot", "-Kneato", "-Goverlap=false"], input=graph, universal_newlines=True
    )

    # try two different edgepaint invocations
    edgepaint = which("edgepaint")
    gray = subprocess.check_output(
        [edgepaint, "--angle=89.999", "--color_scheme=gray"],
        input=laidout,
        universal_newlines=True,
    )
    rgb = subprocess.check_output(
        [edgepaint, "--angle=89.999", "--color_scheme=#00ff00,#0000ff"],
        input=laidout,
        universal_newlines=True,
    )

    # process these into an image
    gray_svg = subprocess.check_output(
        ["dot", "-Kneato", "-n2", "-Tsvg"], input=gray, universal_newlines=True
    )
    rgb_svg = subprocess.check_output(
        ["dot", "-Kneato", "-n2", "-Tsvg"], input=rgb, universal_newlines=True
    )

    assert gray_svg != rgb_svg, "edgepaint --color_scheme had no effect"


@pytest.mark.skipif(shutil.which("tclsh") is None, reason="tclsh not available")
@pytest.mark.skipif(
    platform.system() == "Windows",
    reason="pexpect.spawn is not available on Windows "
    "(https://pexpect.readthedocs.io/en/stable/overview.html#pexpect-on-windows)",
)
@pytest.mark.xfail(
    is_cmake() and is_macos(),
    reason="FIXME: 'vgpane' command is unrecognized for unknown reasons",
    strict=True,
)
@pytest.mark.xfail(
    is_autotools() and is_macos(),
    reason="Autotools on macOS does not detect TCL",
    strict=True,
)
@pytest.mark.xfail(
    is_cmake() and is_ubuntu_2004(),
    reason="TCL packages are not built on Ubuntu 20.04 with CMake < 3.18",
    strict=True,
)
def test_2596():
    """
    running Tclpathplan `triangulate` with a malformed callback script should not read
    out-of-bounds
    https://gitlab.com/graphviz/graphviz/-/issues/2596
    """

    # if this appears to be an ASan-enabled CI job, teach `tclsh` to load ASan’s
    # supporting library because it is otherwise unaware that Tcldot depends on this
    # being loaded first
    env = os.environ.copy()
    if re.search(r"\basan\b", os.environ.get("CI_JOB_NAME", "").lower()):
        cc = os.environ.get("CC", "gcc")
        libasan = subprocess.check_output(
            [cc, "-print-file-name=libasan.so"], universal_newlines=True
        ).strip()
        print(f"setting LD_PRELOAD={libasan}")
        env["LD_PRELOAD"] = libasan

    # startup TCL and load the pathplan module
    proc = pexpect.spawn("tclsh", timeout=1, env=env)
    proc.expect("% ")
    proc.sendline("package require Tclpathplan")
    proc.expect("% ")

    # Create a pane. We assume the first created pane will be index 0, though
    # this is not technically required.
    proc.sendline("vgpane")
    proc.expect("vgpane0")
    proc.expect("% ")

    # bind the triangulation callback to something ending in a trailing '%'
    proc.sendline("vgpane0 bind triangle %")
    proc.expect("% ")

    # add a triangular polygon
    proc.sendline("vgpane0 insert 1 1 2 2 1 2")
    proc.expect("1")
    proc.expect("% ")

    # attempt triangulation on this polygon
    proc.sendline("vgpane0 triangulate 1")
    proc.expect("% ")

    # delete the pane to clean up
    proc.sendline("vgpane0 delete")
    proc.expect("% ")

    # tell TCL to exit
    proc.sendeof()
    proc.wait()


@pytest.mark.skipif(which("gvgen") is None, reason="gvgen not available")
@pytest.mark.skipif(which("mingle") is None, reason="mingle not available")
@pytest.mark.xfail(
    reason="https://gitlab.com/graphviz/graphviz/-/issues/2599", strict=True
)
def test_2599():
    """
    mingle should not segfault when processing simple graphs
    https://gitlab.com/graphviz/graphviz/-/issues/2599
    """

    # generate a graph
    gvgen = which("gvgen")
    graph = subprocess.check_output([gvgen, "-d", "-k", "5"], universal_newlines=True)

    # process it into canonical form
    processed = subprocess.check_output(["dot"], universal_newlines=True, input=graph)

    # pass it through mingle
    mingle = which("mingle")
    proc = subprocess.run(
        [mingle, "-v", "999"], universal_newlines=True, input=processed
    )

    # Address Sanitizer catches segfaults and turns them into non-zero exits, so ignore
    # testing in this scenario
    if re.search(r"\basan\b", os.environ.get("CI_JOB_NAME", "").lower()):
        pytest.skip("crashes of mingle are harder to detect under ASan")

    assert proc.returncode in (0, 1), "mingle crashed"


@pytest.mark.skipif(which("acyclic") is None, reason="acyclic not available")
def test_2600():
    """
    acyclic should produce output
    https://gitlab.com/graphviz/graphviz/-/issues/2600
    """

    # run acyclic on a simple cyclic graph
    acyclic = which("acyclic")
    ret = subprocess.run(
        [acyclic],
        input="digraph { A -> B -> C -> D -> E; E -> A }",
        stdout=subprocess.PIPE,
        check=False,
        universal_newlines=True,
    )

    assert ret.returncode == 1, "acyclic did not detect a cyclic graph"
    assert ret.stdout.strip() != "", "acyclic produced no output"


@pytest.mark.skipif(shutil.which("convert") is None, reason="ImageMagick not available")
@pytest.mark.skipif(which("neato") is None, reason="neato not available")
@pytest.mark.skipif(
    platform.system() == "Windows", reason="`convert` on Windows is not ImageMagick"
)
def test_2609(tmp_path: Path):
    """
    GIFs should not be blank
    https://gitlab.com/graphviz/graphviz/-/issues/2609
    """

    # locate our associated test case in this directory
    input = Path(__file__).parent / "2609.dot"
    assert input.exists(), "unexpectedly missing test case"

    # run this through Neato and convert to GIF
    neato = which("neato")
    gif = tmp_path / "2609.gif"
    subprocess.check_call([neato, "-Tgif", input, "-o", gif])

    # translate this into the simplest format we can parse to validate
    ppm = tmp_path / "2609.ppm"
    subprocess.check_call(["convert", gif, ppm])

    with open(ppm, "rb") as f:

        # we should have the PPM header
        assert f.read(3) == b"P6\n"

        # skip the dimensions line
        while True:
            if f.read(1) in (b"", b"\n"):
                break

        # skip maximum intensity line
        while True:
            if f.read(1) in (b"", b"\n"):
                break

        # read the first pixel as a reference color
        reference = f.read(3)

        # the remaining pixels should not all be identical if we have an actual image
        while True:
            pixel = f.read(3)
            if len(pixel) < 3:
                break
            if pixel != reference:
                # found a different pixel
                return

    pytest.fail("generated GIF was a solid color")


@pytest.mark.parametrize("package", ("Tcldot", "Tclpathplan"))
@pytest.mark.skipif(shutil.which("tclsh") is None, reason="tclsh not available")
@pytest.mark.xfail(
    is_autotools() and is_macos(),
    reason="Autotools on macOS does not detect TCL",
    strict=True,
)
@pytest.mark.xfail(
    is_cmake() and is_ubuntu_2004(),
    reason="TCL packages are not built on Ubuntu 20.04 with CMake < 3.18",
    strict=True,
)
def test_import_tcl_package(package: str):
    """
    The given TCL package should be loadable
    """

    # if this appears to be an ASan-enabled CI job, teach `tclsh` to load ASan’s
    # supporting library because it is otherwise unaware that Tcldot depends on this
    # being loaded first
    env = os.environ.copy()
    if re.search(r"\basan\b", os.environ.get("CI_JOB_NAME", "").lower()):
        cc = os.environ.get("CC", "gcc")
        libasan = subprocess.check_output(
            [cc, "-print-file-name=libasan.so"], universal_newlines=True
        ).strip()
        print(f"setting LD_PRELOAD={libasan}")
        env["LD_PRELOAD"] = libasan

    # ask TCL to import the given package
    response = subprocess.check_output(
        ["tclsh"],
        stderr=subprocess.STDOUT,
        input=f"package require {package};",
        universal_newlines=True,
        env=env,
    )

    assert "can't find package" not in response, f"{package} cannot be loaded by TCL"


@pytest.mark.skipif(shutil.which("tclsh") is None, reason="tclsh not available")
@pytest.mark.skipif(
    platform.system() == "Windows",
    reason="pexpect.spawn is not available on Windows "
    "(https://pexpect.readthedocs.io/en/stable/overview.html#pexpect-on-windows)",
)
@pytest.mark.xfail(
    is_cmake() and is_macos(),
    reason="FIXME: 'vgpane' command is unrecognized for unknown reasons",
    strict=True,
)
@pytest.mark.xfail(
    is_autotools() and is_macos(),
    reason="Autotools on macOS does not detect TCL",
    strict=True,
)
@pytest.mark.xfail(
    is_cmake() and is_ubuntu_2004(),
    reason="TCL packages are not built on Ubuntu 20.04 with CMake < 3.18",
    strict=True,
)
def test_triangulation_overflow():
    """
    running Tclpathplan `triangulate` with a malformed polygon should be rejected
    """

    # if this appears to be an ASan-enabled CI job, teach `tclsh` to load ASan’s
    # supporting library because it is otherwise unaware that Tcldot depends on this
    # being loaded first
    env = os.environ.copy()
    if re.search(r"\basan\b", os.environ.get("CI_JOB_NAME", "").lower()):
        cc = os.environ.get("CC", "gcc")
        libasan = subprocess.check_output(
            [cc, "-print-file-name=libasan.so"], universal_newlines=True
        ).strip()
        print(f"setting LD_PRELOAD={libasan}")
        env["LD_PRELOAD"] = libasan

    # startup TCL and load the pathplan module
    proc = pexpect.spawn("tclsh", timeout=1, env=env)
    proc.expect("% ")
    proc.sendline("package require Tclpathplan")
    proc.expect("% ")

    # Create a pane. We assume the first created pane will be index 0, though
    # this is not technically required.
    proc.sendline("vgpane")
    proc.expect("vgpane0")
    proc.expect("% ")

    # add a “polygon” with only a single point
    proc.sendline("vgpane0 insert 4 5")
    proc.expect("1")
    proc.expect("% ")

    # attempt triangulation on this polygon
    proc.sendline("vgpane0 triangulate 1")
    proc.expect("cannot be triangulated")
    proc.expect("% ")

    # delete the pane to clean up
    proc.sendline("vgpane0 delete")
    proc.expect("% ")

    # tell TCL to exit
    proc.sendeof()
    proc.wait()


@pytest.mark.skipif(shutil.which("tclsh") is None, reason="tclsh not available")
@pytest.mark.skipif(
    platform.system() == "Windows",
    reason="pexpect.spawn is not available on Windows "
    "(https://pexpect.readthedocs.io/en/stable/overview.html#pexpect-on-windows)",
)
@pytest.mark.xfail(
    is_cmake() and is_macos(),
    reason="FIXME: 'vgpane' command is unrecognized for unknown reasons",
    strict=True,
)
@pytest.mark.xfail(
    is_autotools() and is_macos(),
    reason="Autotools on macOS does not detect TCL",
    strict=True,
)
@pytest.mark.xfail(
    is_cmake() and is_ubuntu_2004(),
    reason="TCL packages are not built on Ubuntu 20.04 with CMake < 3.18",
    strict=True,
)
def test_vgpane_bad_triangulation():
    """
    running Tclpathplan `triangulate` with incorrect arguments should be rejected
    """

    # if this appears to be an ASan-enabled CI job, teach `tclsh` to load ASan’s
    # supporting library because it is otherwise unaware that Tcldot depends on this
    # being loaded first
    env = os.environ.copy()
    if re.search(r"\basan\b", os.environ.get("CI_JOB_NAME", "").lower()):
        cc = os.environ.get("CC", "gcc")
        libasan = subprocess.check_output(
            [cc, "-print-file-name=libasan.so"], universal_newlines=True
        ).strip()
        print(f"setting LD_PRELOAD={libasan}")
        env["LD_PRELOAD"] = libasan

    # startup TCL and load the pathplan module
    proc = pexpect.spawn("tclsh", timeout=1, env=env)
    proc.expect("% ")
    proc.sendline("package require Tclpathplan")
    proc.expect("% ")

    # Create a pane. We assume the first created pane will be index 0, though
    # this is not technically required.
    proc.sendline("vgpane")
    proc.expect("vgpane0")
    proc.expect("% ")

    # bind the triangulation callback to something ending in a trailing '%'
    proc.sendline("vgpane0 bind triangle %")
    proc.expect("% ")

    # run triangulation with no polygon ID, which should be rejected
    proc.sendline("vgpane0 triangulate")
    proc.expect("wrong # args")

    # delete the pane to clean up
    proc.sendline("vgpane0 delete")
    proc.expect("% ")

    # tell TCL to exit
    proc.sendeof()
    proc.wait()


@pytest.mark.skipif(shutil.which("tclsh") is None, reason="tclsh not available")
@pytest.mark.skipif(
    platform.system() == "Windows",
    reason="pexpect.spawn is not available on Windows "
    "(https://pexpect.readthedocs.io/en/stable/overview.html#pexpect-on-windows)",
)
@pytest.mark.xfail(
    is_cmake() and is_macos(),
    reason="FIXME: 'vgpane' command is unrecognized for unknown reasons",
    strict=True,
)
@pytest.mark.xfail(
    is_autotools() and is_macos(),
    reason="Autotools on macOS does not detect TCL",
    strict=True,
)
@pytest.mark.xfail(
    is_cmake() and is_ubuntu_2004(),
    reason="TCL packages are not built on Ubuntu 20.04 with CMake < 3.18",
    strict=True,
)
def test_vgpane_delete():
    """
    it should be possible to delete an existing `vgpane`
    """

    # if this appears to be an ASan-enabled CI job, teach `tclsh` to load ASan’s
    # supporting library because it is otherwise unaware that Tcldot depends on this
    # being loaded first
    env = os.environ.copy()
    if re.search(r"\basan\b", os.environ.get("CI_JOB_NAME", "").lower()):
        cc = os.environ.get("CC", "gcc")
        libasan = subprocess.check_output(
            [cc, "-print-file-name=libasan.so"], universal_newlines=True
        ).strip()
        print(f"setting LD_PRELOAD={libasan}")
        env["LD_PRELOAD"] = libasan

    # startup TCL and load the pathplan module
    proc = pexpect.spawn("tclsh", timeout=1, env=env)
    proc.expect("% ")
    proc.sendline("package require Tclpathplan")
    proc.expect("% ")

    # Create a pane. We assume the first created pane will be index 0, though
    # this is not technically required.
    proc.sendline("vgpane")
    proc.expect("vgpane0")
    proc.expect("% ")

    # delete the pane to clean up
    proc.sendline("vgpane0 delete")
    # `pexpect.expect` returns an index of which given expectation was matched. We
    # expect this to return no output (not the invalid handle message) and therefore
    # timeout.
    is_valid = proc.expect(['Invalid handle: "vgpane0"', pexpect.TIMEOUT]) == 1
    assert is_valid, "created vgpane was considered an invalid handle"

    # tell TCL to exit
    proc.sendeof()
    proc.wait()


def test_changelog_dates():
    """
    Check the dates of releases in the changelog are correctly formatted
    """
    changelog = Path(__file__).parent / "../CHANGELOG.md"
    with open(changelog, "rt", encoding="utf-8") as f:
        for lineno, line in enumerate(f, 1):
            m = re.match(r"## \[\d+\.\d+\.\d+\] [-–] (?P<date>.*)$", line)
            if m is None:
                continue
            d = re.match(r"\d{4}-\d{2}-\d{2}", m.group("date"))
            assert (
                d is not None
            ), f"CHANGELOG.md:{lineno}: date in incorrect format: {line}"


@pytest.mark.skipif(which("gvpack") is None, reason="gvpack not available")
def test_duplicate_hard_coded_metrics_warnings():
    """
    Check “no hard-coded metrics” warnings are not repeated
    """

    # use the #2239 test case that happens to provoke this
    input = Path(__file__).parent / "2239.dot"
    assert input.exists(), "unexpectedly missing test case"

    # run it through gvpack
    gvpack = which("gvpack")
    p = subprocess.run(
        [gvpack, "-u", "-o", os.devnull, input],
        stderr=subprocess.PIPE,
        universal_newlines=True,
    )

    assert (
        p.stderr.count("no hard-coded metrics for 'sans'") <= 1
    ), "multiple identical “no hard-coded metrics” warnings printed"


@pytest.mark.parametrize("branch", (0, 1, 2, 3))
@pytest.mark.skipif(which("gvpr") is None, reason="gvpr not available")
def test_gvpr_switches(branch: int):
    """
    confirm the behavior of GVPR switch statements
    """

    # an input GVPR program with multiple blocks and switches
    program = textwrap.dedent(
        f"""\
    BEGIN {{
      switch ({branch}) {{
        case 0:
          printf("begin 0\\n");
          break;
        case 1:
          printf("begin 1\\n");
          break;
        case 2:
          printf("begin 2\\n");
          break;
        default:
          printf("begin 3\\n");
          break;
      }}
    }}

    END {{
      switch ({branch}) {{
        case 0:
          printf("end 0\\n");
          break;
        case 1:
          printf("end 1\\n");
          break;
        case 2:
          printf("end 2\\n");
          break;
        default:
          printf("end 3\\n");
          break;
      }}
    }}
    """
    )

    # run this through GVPR with no input graph
    gvpr_bin = which("gvpr")
    result = subprocess.check_output(
        [gvpr_bin, program], stdin=subprocess.DEVNULL, universal_newlines=True
    )

    # confirm we got the expected output
    assert result == f"begin {branch}\nend {branch}\n", "incorrect GVPR switch behavior"


@pytest.mark.parametrize(
    "statement,expected",
    (
        ('printf("%d", 5)', "5"),
        ('printf("%d", 0)', "0"),
        ('printf("%.0d", 0)', ""),
        ('printf("%.0d", 1)', "1"),
        ('printf("%.d", 2)', "2"),
        ('printf("%d", -1)', "-1"),
        ('printf("%.3d", 5)', "005"),
        ('printf("%.3d", -5)', "-005"),
        ('printf("%5.3d", 5)', "  005"),
        ('printf("%-5.3d", -5)', "-005 "),
        ('printf("%-d", 5)', "5"),
        ('printf("%-+d", 5)', "+5"),
        ('printf("%+-d", 5)', "+5"),
        ('printf("%+d", -5)', "-5"),
        ('printf("% d", 5)', " 5"),
        ('printf("% .0d", 0)', " "),
        ('printf("%03d", 5)', "005"),
        ('printf("%03d", -5)', "-05"),
        ('printf("% +d", 5)', "+5"),
        ('printf("%-03d", -5)', "-5 "),
        ('printf("%o", 5)', "5"),
        ('printf("%o", 8)', "10"),
        ('printf("%o", 0)', "0"),
        ('printf("%.0o", 0)', ""),
        ('printf("%.0o", 1)', "1"),
        ('printf("%.3o", 5)', "005"),
        ('printf("%.3o", 8)', "010"),
        ('printf("%5.3o", 5)', "  005"),
        ('printf("%u", 5)', "5"),
        ('printf("%u", 0)', "0"),
        ('printf("%.0u", 0)', ""),
        ('printf("%.0u", 1)', "1"),
        ('printf("%.3u", 5)', "005"),
        ('printf("%5.3u", 5)', "  005"),
        ('printf("%u", 5)', "5"),
        ('printf("%u", 0)', "0"),
        ('printf("%.0u", 0)', ""),
        ('printf("%.0u", 1)', "1"),
        ('printf("%.3u", 5)', "005"),
        ('printf("%5.3u", 5)', "  005"),
        ('printf("%-x", 5)', "5"),
        ('printf("%03x", 5)', "005"),
        ('printf("%-x", 5)', "5"),
        ('printf("%03x", 5)', "005"),
        ('printf("%-X", 5)', "5"),
        ('printf("%03X", 5)', "005"),
        ('printf("%.2s", "abc")', "ab"),
        ('printf("%.6s", "abc")', "abc"),
        ('printf("%5s", "abc")', "  abc"),
        ('printf("%-5s", "abc")', "abc  "),
        ('printf("%5.2s", "abc")', "   ab"),
        ('printf("%%")', "%"),
    ),
)
@pytest.mark.skipif(which("gvpr") is None, reason="gvpr not available")
def test_gvpr_printf(statement: str, expected: str):
    """
    check various behaviors of `printf` in a GVPR program
    """

    # a program that performs the given `printf`
    program = f"BEGIN {{ {statement}; }}"

    # run this through GVPR with no input graph
    gvpr_bin = which("gvpr")
    result = subprocess.check_output(
        [gvpr_bin, program], stdin=subprocess.DEVNULL, universal_newlines=True
    )

    # confirm we got the expected output
    assert result == expected, "incorrect GVPR printf behavior"


usage_info = """\
Usage: dot [-Vv?] [-(GNE)name=val] [-(KTlso)<val>] <dot files>
(additional options for neato)    [-x] [-n<v>]
(additional options for fdp)      [-L(gO)] [-L(nUCT)<val>]
(additional options for config)  [-cv]

 -V          - Print version and exit
 -v          - Enable verbose mode 
 -Gname=val  - Set graph attribute 'name' to 'val'
 -Nname=val  - Set node attribute 'name' to 'val'
 -Ename=val  - Set edge attribute 'name' to 'val'
 -Tv         - Set output format to 'v'
 -Kv         - Set layout engine to 'v' (overrides default based on command name)
 -lv         - Use external library 'v'
 -ofile      - Write output to 'file'
 -O          - Automatically generate an output filename based on the input filename with a .'format' appended. (Causes all -ofile options to be ignored.) 
 -P          - Internally generate a graph of the current plugins. 
 -q[l]       - Set level of message suppression (=1)
 -s[v]       - Scale input by 'v' (=72)
 -y          - Invert y coordinate in output

 -n[v]       - No layout mode 'v' (=1)
 -x          - Reduce graph

 -Lg         - Don't use grid
 -LO         - Use old attractive force
 -Ln<i>      - Set number of iterations to i
 -LU<i>      - Set unscaled factor to i
 -LC<v>      - Set overlap expansion factor to v
 -LT[*]<v>   - Set temperature (temperature factor) to v

 -c          - Configure plugins (Writes $prefix/lib/graphviz/config 
               with available plugin information.  Needs write privilege.)
 -?          - Print usage and exit
"""


def test_dot_questionmarkV():
    """
    test the output from two short options combined
    """

    out = subprocess.check_output(
        ["dot", "-?V"],
        universal_newlines=True,
    )

    assert out == usage_info, "unexpected usage info"


def test_dot_randomV():
    """
    test the output from a malformed command
    """

    expected = f"Error: dot: option -r unrecognized\n\n{usage_info}"

    proc = subprocess.run(
        ["dot", "-randomV"],
        stderr=subprocess.PIPE,
        universal_newlines=True,
        check=False,
    )

    assert proc.returncode != 0, "malformed options were accepted"

    assert proc.stderr == expected, "unexpected usage info"


def test_dot_V():
    """
    test the output from `dot -V`
    """

    proc = subprocess.run(
        ["dot", "-V"], stderr=subprocess.PIPE, universal_newlines=True, check=True
    )

    c_src = (Path(__file__).parent / "get-package-version.c").resolve()
    assert c_src.exists(), "missing test case"
    package_version, _ = run_c(c_src)

    assert proc.stderr.startswith(
        f"dot - graphviz version {package_version.strip()} ("
    ), "unexpected -V info"


def test_dot_Vquestionmark():
    """
    test the output from two short options combined
    """

    proc = subprocess.run(
        ["dot", "-V?"], stderr=subprocess.PIPE, universal_newlines=True, check=True
    )

    c_src = (Path(__file__).parent / "get-package-version.c").resolve()
    assert c_src.exists(), "missing test case"
    package_version, _ = run_c(c_src)

    assert proc.stderr.startswith(
        f"dot - graphviz version {package_version.strip()} ("
    ), "unexpected -V info"


def test_dot_Vrandom():
    """
    test the output from a short option mixed with long
    """

    proc = subprocess.run(
        ["dot", "-Vrandom"], stderr=subprocess.PIPE, universal_newlines=True, check=True
    )

    c_src = (Path(__file__).parent / "get-package-version.c").resolve()
    assert c_src.exists(), "missing test case"
    package_version, _ = run_c(c_src)

    assert proc.stderr.startswith(
        f"dot - graphviz version {package_version.strip()} ("
    ), "unexpected -V info"


def test_pic_font_size():
    """
    font size in PIC output format should not be clamped down to 1
    related to https://gitlab.com/graphviz/graphviz/-/issues/2487
    """

    # run a basic graph through PIC generation
    src = "graph { a -- b; }"
    pic = dot("pic", source=src)

    # confirm we got a non-1 font size
    m = re.search(r"^\.ps (\d+)", pic, flags=re.MULTILINE)
    assert int(m.group(1)) > 1, "font size clamped down to 1"


@pytest.mark.skipif(which("mm2gv") is None, reason="mm2gv not available")
def test_mm_banner_overflow(tmp_path: Path):
    """mm2gv should be robust against files with a corrupted banner"""

    # construct a file with a corrupted banner > MM_MAX_TOKEN_LENGTH and < MM_MAX_LINE_LENGTH
    mm = tmp_path / "matrix.mm"
    mm.write_text(f"%{'a' * 10000}", encoding="utf-8")

    # run this through mm2gv
    ret = subprocess.call(["mm2gv", "-o", os.devnull, mm])

    assert ret in (0, 1), "mm2gv crashed when processing malformed input"
    assert ret == 1, "mm2gv did not reject malformed input"


def test_control_characters_in_error():
    """
    malformed input should not result in misleading control data making it to the
    output terminal unfiltered
    """

    # Run something through Graphviz that will trigger an error where the error message
    # will contain a color control sequence. This could be used to disrupt the user’s
    # terminal in confusing ways.
    src = 'graph { a[image="\033[31mfoo"]; }'
    ret = subprocess.run(
        ["dot", "-Tsvg", "-o", os.devnull],
        input=src,
        stderr=subprocess.PIPE,
        universal_newlines=True,
    )

    assert "\033" not in ret.stderr, "control character appears in error message"

    # Now try something more malicious. Use the backspace character to display a different
    # filename in the error message to what was referenced.
    src = 'graph { a[image="foo.svg\010\010\010png"]; }'
    ret = subprocess.run(
        ["dot", "-Tsvg", "-o", os.devnull],
        input=src,
        stderr=subprocess.PIPE,
        universal_newlines=True,
    )

    assert "\010" not in ret.stderr, "control character appears in error message"


def test_fig_max_colors():
    """
    using a large number of colors should not crash the FIG renderer
    """

    # contruct a graph that uses well over 256 colors
    buf = io.StringIO()
    buf.write("graph {\n")
    for red in range(256):
        for green in range(10):
            buf.write(f'  n_{red}_{green}[color="#{red:02x}{green:02x}00"];\n')
    buf.write("}\n")

    # render this using the FIG renderer
    dot("fig", source=buf.getvalue())


@pytest.mark.skipif(which("gvpr") is None, reason="gvpr not available")
def test_gvpr_s2f():
    """
    casting a string to floating point in GVPR should work
    """

    # a GVPR program that casts a string to floating point and prints the result
    program = 'BEGIN { float x = (float)"1.5"; printf("%0.1f\\n", x); }'

    # run this through GVPR with no input graph
    gvpr_bin = which("gvpr")
    result = subprocess.check_output(
        [gvpr_bin, program], stdin=subprocess.DEVNULL, universal_newlines=True
    )

    # confirm we got the expected output
    assert result == "1.5\n", "incorrect GVPR float cast behavior"


def test_changelog():
    """
    sanity checks on ../CHANGELOG.md
    """

    changelog = Path(__file__).parent / "../CHANGELOG.md"
    assert changelog.exists(), "CHANGELOG.md missing"

    with open(changelog, "rt", encoding="utf-8") as f:
        for lineno, line in enumerate(f, 1):

            ignore_h2 = False

            # an exception for an old heading
            if line == "## [2.42.3] and earlier\n":
                ignore_h2 = True

            # an exception for unreleased versions
            if line.startswith("## ") and "Unreleased" in line:
                ignore_h2 = True

            if (m := re.match("##(?P<remainder>[^#].*)$", line)) and not ignore_h2:

                expected_format = r" \[\d+\.\d+\.\d+\] [\-–] \d{4}-\d{2}-\d{2}$"
                assert re.match(expected_format, m.group("remainder")), (
                    f"CHANGELOG.md:{lineno}: second-level heading did not match "
                    f'regex r"{expected_format}": {line}'
                )

            if m := re.match("###(?P<remainder>.*)$", line):
                assert m.group("remainder") in (
                    " Added",
                    " Changed",
                    " Fixed",
                    " Removed",
                ), f"CHANGELOG.md:{lineno}: unexpected third-level heading: {line}"

            if m := re.match(
                r"\[(?P<version>\d+\.\d+\.\d+)\]:(?P<remainder>.*)$", line
            ):
                prefix = " https://gitlab.com/graphviz/graphviz/compare/"
                assert m.group("remainder").startswith(
                    prefix
                ), f"CHANGELOG.md:{lineno}: unexpected finalized history link: {line}"
                remainder = m.group("remainder")[len(prefix) :]

                assert m.group("remainder").endswith(
                    f'...{m.group("version")}'
                ), f"CHANGELOG.md:{lineno}: history link is for wrong version: {line}"

                version_range = re.match(
                    r"(?P<start_major>\d+)\.(?P<start_minor>\d+)\.(?P<start_patch>\d+)"
                    r"\.\.\."
                    r"(?P<end_major>\d+)\.(?P<end_minor>\d+)\.(?P<end_patch>\d+)$",
                    remainder,
                )
                assert (
                    version_range
                ), f"CHANGELOG.md:{lineno}: unexpected finalized history link: {line}"

                start = tuple(
                    int(version_range.group(v))
                    for v in ("start_major", "start_minor", "start_patch")
                )
                end = tuple(
                    int(version_range.group(v))
                    for v in ("end_major", "end_minor", "end_patch")
                )
                assert (
                    start < end
                ), f"CHANGELOG.md:{lineno}: invalid version range: {line}"


def test_agxbuf_print_nul():
    """
    `agxbprint` should not account for nor append a NUL byte
    """

    # find co-located test source
    c_src = (Path(__file__).parent / "agxbuf-print-nul.c").resolve()
    assert c_src.exists(), "missing test case"

    lib = Path(__file__).parents[1] / "lib"
    if platform.system() == "Windows" and not is_mingw():
        cflags = [f"/I{lib}"]
    else:
        # GNU99 needed for `strndup`
        cflags = ["-std=gnu99", f"-I{lib}"]

    run_c(c_src, cflags=cflags)


def test_agxbuf_use_implicit_nul():
    """
    `agxbuf` should be able to use its entire memory as an inline string
    """

    # find co-located test source
    c_src = (Path(__file__).parent / "agxbuf-use-implicit-nul.c").resolve()
    assert c_src.exists(), "missing test case"

    lib = Path(__file__).parents[1] / "lib"
    if platform.system() == "Windows" and not is_mingw():
        cflags = [f"/I{lib}"]
    else:
        # GNU99 needed for `strndup`
        cflags = ["-std=gnu99", f"-I{lib}"]

    run_c(c_src, cflags=cflags)


@pytest.mark.skipif(which("edgepaint") is None, reason="edgepaint not available")
def test_edgepaint_error_message():
    """
    when failing to open its output, edgepaint should not dereference a null
    pointer
    """

    # try to open a non-existent file
    edgepaint = which("edgepaint")
    proc = subprocess.run(
        [edgepaint, "-o", "/a/nonexistent/path"],
        stdout=subprocess.DEVNULL,
        stderr=subprocess.PIPE,
        universal_newlines=True,
    )

    # edgepaint should name itself in the error message, not “(null)”
    assert re.search(
        r"\bedgepaint\b", proc.stderr
    ), "edgepaint does not know its own name"
