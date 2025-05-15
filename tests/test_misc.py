"""
Graphviz miscellaneous test cases
"""

import itertools
import json
import os
import sys
from pathlib import Path
from typing import Union

sys.path.append(os.path.dirname(__file__))
from gvtest import ROOT, compile_c, dot, run  # pylint: disable=wrong-import-position


def test_json_node_order():
    """
    test that nodes appear in JSON output in the same order as they were input
    """

    # a simple graph with some nodes
    input = (
        "digraph G {\n"
        "  ordering=out;\n"
        '  {rank=same;"1";"2"};\n'
        '  "1"->"2";\n'
        '  {rank=same;"4";"5";};\n'
        '  "4"->"5";\n'
        '  "7"->"5";\n'
        '  "7"->"4";\n'
        '  "6"->"1";\n'
        '  "3"->"6";\n'
        '  "6"->"4";\n'
        '  "3"->"8";\n'
        "}"
    )

    # turn it into JSON
    output = dot("json", source=input)

    # parse this into a data structure we can inspect
    data = json.loads(output)

    # extract the nodes, filtering out subgraphs
    nodes = [n["name"] for n in data["objects"] if "nodes" not in n]

    # the nodes should appear in the order in which they were seen in the input
    assert nodes == ["1", "2", "4", "5", "7", "6", "3", "8"]


def test_json_edge_order():
    """
    test that edges appear in JSON output in the same order as they were input
    """

    # a simple graph with some edges
    input = (
        "digraph G {\n"
        "  ordering=out;\n"
        '  {rank=same;"1";"2"};\n'
        '  "1"->"2";\n'
        '  {rank=same;"4";"5";};\n'
        '  "4"->"5";\n'
        '  "7"->"5";\n'
        '  "7"->"4";\n'
        '  "6"->"1";\n'
        '  "3"->"6";\n'
        '  "6"->"4";\n'
        '  "3"->"8";\n'
        "}"
    )

    # turn it into JSON
    output = dot("json", source=input)

    # parse this into a data structure we can inspect
    data = json.loads(output)

    # the edges should appear in the order in which they were seen in the input
    expected = [
        ("1", "2"),
        ("4", "5"),
        ("7", "5"),
        ("7", "4"),
        ("6", "1"),
        ("3", "6"),
        ("6", "4"),
        ("3", "8"),
    ]
    edges = [
        (data["objects"][e["tail"]]["name"], data["objects"][e["head"]]["name"])
        for e in data["edges"]
    ]
    assert edges == expected


def test_xml_escape(tmp_path: Path):
    """
    Check the functionality of ../lib/util/xml.c:gv_xml_escape.
    """

    # locate our test program
    xml_c = Path(__file__).parent / "../lib/util/xml.c"
    assert xml_c.exists(), "missing xml.c"

    # compile the stub to something we can run
    xml_exe = tmp_path / "xml.exe"
    lib = ROOT / "lib"
    cflags = ["-DTEST_XML", f"-I{lib}"]
    compile_c(xml_c, cflags, dst=xml_exe)

    def escape(
        dash: bool, nbsp: bool, raw: bool, utf8: bool, s: Union[bytes, str]
    ) -> str:

        source = tmp_path / "input"
        if isinstance(s, bytes):
            source.write_bytes(s)
        else:
            source.write_text(s, encoding="utf-8")

        destination = tmp_path / "output"

        args = [xml_exe]
        if dash:
            args += ["--dash"]
        if nbsp:
            args += ["--nbsp"]
        if raw:
            args += ["--raw"]
        if utf8:
            args += ["--utf8"]
        args += [source, destination]

        run(args)

        return destination.read_text(encoding="utf-8")

    for dash, nbsp, raw, utf8 in itertools.product((False, True), repeat=4):
        # something basic with nothing escapable
        plain = "the quick brown fox"
        plain_escaped = escape(dash, nbsp, raw, utf8, plain)
        assert plain == plain_escaped, "text incorrectly modified"

        # basic tag escaping
        tag = "template <typename T> void foo(T t);"
        tag_escaped = escape(dash, nbsp, raw, utf8, tag)
        assert (
            tag_escaped == "template &lt;typename T&gt; void foo(T t);"
        ), "incorrect < or > escaping"

        # something with an embedded escape
        embedded = "salt &amp; pepper"
        embedded_escaped = escape(dash, nbsp, raw, utf8, embedded)
        if raw:
            assert embedded_escaped == "salt &amp;amp; pepper", "missing & escape"
        else:
            assert embedded_escaped == embedded, "text incorrectly modified"

        # hyphen escaping
        hyphen = "UTF-8"
        hyphen_escaped = escape(dash, nbsp, raw, utf8, hyphen)
        if dash:
            assert hyphen_escaped == "UTF&#45;8", "incorrect dash escape"
        else:
            assert hyphen_escaped == hyphen, "text incorrectly modified"

        # line endings
        nl = b"the quick\nbrown\rfox"
        nl_escaped = escape(dash, nbsp, raw, utf8, nl)
        if raw:
            assert (
                nl_escaped == "the quick&#10;brown&#13;fox"
            ), "incorrect new line escape"
        else:
            # allow benign modification of the \r
            assert nl_escaped in (
                "the quick\nbrown\rfox",
                "the quick\nbrown\nfox",
            ), "text incorrectly modified"

        # non-breaking space escaping
        two = "the quick  brown fox"
        two_escaped = escape(dash, nbsp, raw, utf8, two)
        if nbsp:
            assert two_escaped == "the quick &#160;brown fox", "incorrect nbsp escape"
        else:
            assert two_escaped == two, "text incorrectly modified"

        # cases from table in https://en.wikipedia.org/wiki/UTF-8
        for c, expected in (
            ("$", "$"),
            ("¢", "&#xa2;"),
            ("ह", "&#x939;"),
            ("€", "&#x20ac;"),
            ("한", "&#xd55c;"),
            ("𐍈", "&#x10348;"),
        ):
            unescaped = f"character |{c}|"
            escaped = escape(dash, nbsp, raw, utf8, unescaped)
            if utf8:
                assert escaped == f"character |{expected}|", "bad UTF-8 escaping"
            else:
                assert escaped == unescaped, "bad UTF-8 passthrough"
