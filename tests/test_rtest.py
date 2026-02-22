#!/usr/bin/env python3

"""
Older Graphviz regression test suite that has been encapsulated

TODO:
 Report differences with shared version and with new output.
"""

import io
import os
import platform
import re
import sys
import warnings
from dataclasses import dataclass
from pathlib import Path

import pytest

sys.path.append(os.path.dirname(__file__))
from gvtest import (  # pylint: disable=wrong-import-position
    is_rocky_10,
    is_ubuntu_2404,
    run,
)

# Test specifications
GRAPHDIR = Path(__file__).parent / "graphs"
# Directory of input graphs and data
OUTHTML = Path("nhtml")  # Directory for html test report


@dataclass
class Case:
    """test case struct"""

    name: str
    input: Path
    algorithm: str
    format: str
    flags: list[str]
    index: int = 0
    xfail: bool = True  # is this test case expected to fail?


TESTS: list[Case] = [
    Case("trivial", Path("trivial.gv"), "dot", "gv", [], xfail=is_rocky_10()),
    Case(
        "shapes",
        Path("shapes.gv"),
        "dot",
        "gv",
        [],
        xfail=not is_ubuntu_2404(),
    ),
    Case("shapes", Path("shapes.gv"), "dot", "ps", []),
    Case("crazy", Path("crazy.gv"), "dot", "png", []),
    Case("crazy", Path("crazy.gv"), "dot", "ps", []),
    Case("arrows", Path("arrows.gv"), "dot", "gv", []),
    Case("arrows", Path("arrows.gv"), "dot", "ps", []),
    Case("arrowsize", Path("arrowsize.gv"), "dot", "png", []),
    Case("center", Path("center.gv"), "dot", "ps", []),
    Case("center", Path("center.gv"), "dot", "png", ["-Gmargin=1"]),
    # color encodings
    # multiple edge colors
    Case("color", Path("color.gv"), "dot", "png", []),
    Case("color", Path("color.gv"), "dot", "png", ["-Gbgcolor=lightblue"]),
    Case("decorate", Path("decorate.gv"), "dot", "png", []),
    Case("record", Path("record.gv"), "dot", "gv", []),
    Case("record", Path("record.gv"), "dot", "ps", []),
    Case("html", Path("html.gv"), "dot", "gv", []),
    Case("html", Path("html.gv"), "dot", "ps", []),
    Case("html2", Path("html2.gv"), "dot", "gv", []),
    Case("html2", Path("html2.gv"), "dot", "ps", []),
    Case("html2", Path("html2.gv"), "dot", "svg", []),
    Case("pslib", Path("pslib.gv"), "dot", "ps", ["-lgraphs/sdl.ps"]),
    Case("user_shapes", Path("user_shapes.gv"), "dot", "ps", []),
    # dot png - doesn't work: Warning: No loadimage plugin for "gif:cairo"
    Case("user_shapes", Path("user_shapes.gv"), "dot", "png:gd", []),
    # bug - the epsf version has problems
    Case(
        "ps_user_shapes",
        Path("ps_user_shapes.gv"),
        "dot",
        "ps",
        ["-Nshapefile=graphs/dice.ps"],
    ),
    Case("colorscheme", Path("colorscheme.gv"), "dot", "ps", []),
    Case("colorscheme", Path("colorscheme.gv"), "dot", "png", []),
    Case("compound", Path("compound.gv"), "dot", "gv", []),
    Case("dir", Path("dir.gv"), "dot", "ps", []),
    Case("clusters", Path("clusters.gv"), "dot", "ps", []),
    Case("clusters", Path("clusters.gv"), "dot", "png", []),
    Case(
        "clustlabel",
        Path("clustlabel.gv"),
        "dot",
        "ps",
        ["-Glabelloc=t", "-Glabeljust=r"],
    ),
    Case(
        "clustlabel",
        Path("clustlabel.gv"),
        "dot",
        "ps",
        ["-Glabelloc=b", "-Glabeljust=r"],
        1,
    ),
    Case(
        "clustlabel",
        Path("clustlabel.gv"),
        "dot",
        "ps",
        ["-Glabelloc=t", "-Glabeljust=l"],
        2,
    ),
    Case(
        "clustlabel",
        Path("clustlabel.gv"),
        "dot",
        "ps",
        ["-Glabelloc=b", "-Glabeljust=l"],
        3,
    ),
    Case(
        "clustlabel",
        Path("clustlabel.gv"),
        "dot",
        "ps",
        ["-Glabelloc=t", "-Glabeljust=c"],
        4,
    ),
    Case(
        "clustlabel",
        Path("clustlabel.gv"),
        "dot",
        "ps",
        ["-Glabelloc=b", "-Glabeljust=c"],
        5,
    ),
    Case("clustlabel", Path("clustlabel.gv"), "dot", "ps", ["-Glabelloc=t"], 6),
    Case("clustlabel", Path("clustlabel.gv"), "dot", "ps", ["-Glabelloc=b"], 7),
    Case(
        "rootlabel",
        Path("rootlabel.gv"),
        "dot",
        "ps",
        ["-Glabelloc=t", "-Glabeljust=r"],
    ),
    Case(
        "rootlabel",
        Path("rootlabel.gv"),
        "dot",
        "ps",
        ["-Glabelloc=b", "-Glabeljust=r"],
        1,
    ),
    Case(
        "rootlabel",
        Path("rootlabel.gv"),
        "dot",
        "ps",
        ["-Glabelloc=t", "-Glabeljust=l"],
        2,
    ),
    Case(
        "rootlabel",
        Path("rootlabel.gv"),
        "dot",
        "ps",
        ["-Glabelloc=b", "-Glabeljust=l"],
        3,
    ),
    Case(
        "rootlabel",
        Path("rootlabel.gv"),
        "dot",
        "ps",
        ["-Glabelloc=t", "-Glabeljust=c"],
        4,
    ),
    Case(
        "rootlabel",
        Path("rootlabel.gv"),
        "dot",
        "ps",
        ["-Glabelloc=b", "-Glabeljust=c"],
        5,
    ),
    Case("rootlabel", Path("rootlabel.gv"), "dot", "ps", ["-Glabelloc=t"], 6),
    Case("rootlabel", Path("rootlabel.gv"), "dot", "ps", ["-Glabelloc=b"], 7),
    Case("layers", Path("layers.gv"), "dot", "ps", []),
    # check mode=hier
    Case("mode", Path("mode.gv"), "neato", "ps", ["-Gmode=KK"]),
    Case("mode", Path("mode.gv"), "neato", "ps", ["-Gmode=hier"], 1),
    Case("mode", Path("mode.gv"), "neato", "ps", ["-Gmode=hier", "-Glevelsgap=1"], 2),
    Case("model", Path("mode.gv"), "neato", "ps", ["-Gmodel=circuit"]),
    Case(
        "model",
        Path("mode.gv"),
        "neato",
        "ps",
        ["-Goverlap=false", "-Gmodel=subset"],
        1,
    ),
    # cairo versions have problems
    Case("nojustify", Path("nojustify.gv"), "dot", "png", []),
    Case("nojustify", Path("nojustify.gv"), "dot", "png:gd", []),
    Case("nojustify", Path("nojustify.gv"), "dot", "ps", []),
    Case("nojustify", Path("nojustify.gv"), "dot", "ps:cairo", []),
    # bug
    Case("ordering", Path("ordering.gv"), "dot", "gv", ["-Gordering=in"]),
    Case("ordering", Path("ordering.gv"), "dot", "gv", ["-Gordering=out"], 1),
    Case("overlap", Path("overlap.gv"), "neato", "gv", ["-Goverlap=false"]),
    Case("overlap", Path("overlap.gv"), "neato", "gv", ["-Goverlap=scale"], 1),
    Case("pack", Path("pack.gv"), "neato", "gv", []),
    Case("pack", Path("pack.gv"), "neato", "gv", ["-Gpack=20"], 1),
    Case("pack", Path("pack.gv"), "neato", "gv", ["-Gpackmode=graph"], 2),
    Case("page", Path("mode.gv"), "neato", "ps", ["-Gpage=8.5,11"]),
    Case("page", Path("mode.gv"), "neato", "ps", ["-Gpage=8.5,11", "-Gpagedir=TL"], 1),
    Case("page", Path("mode.gv"), "neato", "ps", ["-Gpage=8.5,11", "-Gpagedir=TR"], 2),
    # pencolor, fontcolor, fillcolor
    Case("colors", Path("colors.gv"), "dot", "ps", []),
    Case("polypoly", Path("polypoly.gv"), "dot", "ps", []),
    Case("polypoly", Path("polypoly.gv"), "dot", "png", []),
    Case("ports", Path("ports.gv"), "dot", "gv", []),
    Case("radius", Path("radius.gv"), "dot", "gv", [], xfail=is_rocky_10()),
    Case("rotate", Path("crazy.gv"), "dot", "png", ["-Glandscape"]),
    Case("rotate", Path("crazy.gv"), "dot", "ps", ["-Glandscape"]),
    Case("rotate", Path("crazy.gv"), "dot", "png", ["-Grotate=90"], 1),
    Case("rotate", Path("crazy.gv"), "dot", "ps", ["-Grotate=90"], 1),
    Case("rankdir", Path("crazy.gv"), "dot", "gv", ["-Grankdir=LR"]),
    Case("rankdir", Path("crazy.gv"), "dot", "gv", ["-Grankdir=BT"], 1),
    Case("rankdir", Path("crazy.gv"), "dot", "gv", ["-Grankdir=RL"], 2),
    Case("url", Path("url.gv"), "dot", "ps2", []),
    Case("url", Path("url.gv"), "dot", "svg", ["-Gstylesheet=stylesheet"]),
    Case("url", Path("url.gv"), "dot", "imap", []),
    Case("url", Path("url.gv"), "dot", "cmapx", []),
    Case("url", Path("url.gv"), "dot", "imap_np", []),
    Case("url", Path("url.gv"), "dot", "cmapx_np", []),
    Case(
        "viewport", Path("viewport.gv"), "neato", "png", ["-Gviewport=300,300", "-n2"]
    ),
    Case("viewport", Path("viewport.gv"), "neato", "ps", ["-Gviewport=300,300", "-n2"]),
    Case(
        "viewport",
        Path("viewport.gv"),
        "neato",
        "png",
        ["-Gviewport=300,300,1,200,620", "-n2"],
        1,
    ),
    Case(
        "viewport",
        Path("viewport.gv"),
        "neato",
        "ps",
        ["-Gviewport=300,300,1,200,620", "-n2"],
        1,
    ),
    Case(
        "viewport",
        Path("viewport.gv"),
        "neato",
        "png",
        ["-Gviewport=300,300,2,200,620", "-n2"],
        2,
    ),
    Case(
        "viewport",
        Path("viewport.gv"),
        "neato",
        "ps",
        ["-Gviewport=300,300,2,200,620", "-n2"],
        2,
    ),
    Case("rowcolsep", Path("rowcolsep.gv"), "dot", "gv", ["-Gnodesep=0.5"]),
    Case("rowcolsep", Path("rowcolsep.gv"), "dot", "gv", ["-Granksep=1.5"], 1),
    Case("size", Path("mode.gv"), "neato", "ps", ["-Gsize=5,5"]),
    Case("size", Path("mode.gv"), "neato", "png", ["-Gsize=5,5"]),
    # size with !
    Case("size_ex", Path("root.gv"), "dot", "ps", ["-Gsize=6,6!"]),
    Case("size_ex", Path("root.gv"), "dot", "png", ["-Gsize=6,6!"]),
    Case("dotsplines", Path("size.gv"), "dot", "gv", ["-Gsplines=line"]),
    Case("dotsplines", Path("size.gv"), "dot", "gv", ["-Gsplines=polyline"], 1),
    Case(
        "neatosplines",
        Path("overlap.gv"),
        "neato",
        "gv",
        ["-Goverlap=false", "-Gsplines"],
    ),
    Case(
        "neatosplines",
        Path("overlap.gv"),
        "neato",
        "gv",
        ["-Goverlap=false", "-Gsplines=polyline"],
        1,
    ),
    Case("style", Path("style.gv"), "dot", "ps", []),
    Case("style", Path("style.gv"), "dot", "png", []),
    # edge clipping
    Case("edgeclip", Path("edgeclip.gv"), "dot", "gv", []),
    # edge weight
    Case("weight", Path("weight.gv"), "dot", "gv", []),
    Case("root", Path("root.gv"), "twopi", "gv", []),
    Case("cairo", Path("cairo.gv"), "dot", "ps:cairo", []),
    Case("cairo", Path("cairo.gv"), "dot", "png:cairo", []),
    Case("cairo", Path("cairo.gv"), "dot", "svg:cairo", []),
    Case("flatedge", Path("flatedge.gv"), "dot", "gv", []),
    Case("nestedclust", Path("nestedclust.gv"), "dot", "gv", []),
    Case("rd_rules", Path("rd_rules.gv"), "dot", "png", []),
    Case("sq_rules", Path("sq_rules.gv"), "dot", "png", []),
    Case("fdp_clus", Path("fdp.gv"), "fdp", "png", []),
    Case("japanese", Path("japanese.gv"), "dot", "png", []),
    Case("russian", Path("russian.gv"), "dot", "png", []),
    Case("AvantGarde", Path("AvantGarde.gv"), "dot", "png", []),
    Case("AvantGarde", Path("AvantGarde.gv"), "dot", "ps", []),
    Case("Bookman", Path("Bookman.gv"), "dot", "png", []),
    Case("Bookman", Path("Bookman.gv"), "dot", "ps", []),
    Case("Helvetica", Path("Helvetica.gv"), "dot", "png", []),
    Case("Helvetica", Path("Helvetica.gv"), "dot", "ps", []),
    Case("NewCenturySchlbk", Path("NewCenturySchlbk.gv"), "dot", "png", []),
    Case("NewCenturySchlbk", Path("NewCenturySchlbk.gv"), "dot", "ps", []),
    Case("Palatino", Path("Palatino.gv"), "dot", "png", []),
    Case("Palatino", Path("Palatino.gv"), "dot", "ps", []),
    Case("Times", Path("Times.gv"), "dot", "png", []),
    Case("Times", Path("Times.gv"), "dot", "ps", []),
    Case("ZapfChancery", Path("ZapfChancery.gv"), "dot", "png", []),
    Case("ZapfChancery", Path("ZapfChancery.gv"), "dot", "ps", []),
    Case("ZapfDingbats", Path("ZapfDingbats.gv"), "dot", "png", []),
    Case("ZapfDingbats", Path("ZapfDingbats.gv"), "dot", "ps", []),
    Case("xlabels", Path("xlabels.gv"), "dot", "png", []),
    Case("xlabels", Path("xlabels.gv"), "neato", "png", []),
    Case("sides", Path("sides.gv"), "dot", "ps", []),
]


def doDiff(output: Path, reference: Path, fmt):
    """
    Compare old and new output and report if different.

    Args:
        output: File generated during this test run
        reference: Golden copy to compare against
    """
    F = fmt.split(":")[0]
    if F in ["ps", "ps2"]:
        with open(output, "rt", encoding="latin-1") as src:
            dst1 = io.StringIO()
            done_setup = False
            for line in src:
                if done_setup:
                    dst1.write(line)
                else:
                    done_setup = re.match(r"%%End.*Setup", line) is not None

        with open(reference, "rt", encoding="latin-1") as src:
            dst2 = io.StringIO()
            done_setup = False
            for line in src:
                if done_setup:
                    dst2.write(line)
                else:
                    done_setup = re.match(r"%%End.*Setup", line) is not None

        assert dst1.getvalue() == dst2.getvalue()
    elif F == "svg":
        with open(output, "rt", encoding="utf-8") as f:
            a = re.sub(r"^<!--.*-->$", "", f.read(), flags=re.MULTILINE)
        with open(reference, "rt", encoding="utf-8") as f:
            b = re.sub(r"^<!--.*-->$", "", f.read(), flags=re.MULTILINE)
        assert a.strip() == b.strip()
    elif F == "png":
        OUTHTML.mkdir(exist_ok=True)
        run(["diffimg", output, reference, OUTHTML / f"dif_{reference.name}"])
    else:
        with open(reference, "rt", encoding="utf-8") as a:
            with open(output, "rt", encoding="utf-8") as b:
                assert a.read().strip() == b.read().strip()


def genOutname(name, alg, fmt, index: int):
    """
    Generate output file name

    Args:
        name: Name of the current test case.
        alg: Algorithm (-K).
        fmt: Format (-T). If format ends in :*, remove this, change the colons to
            underscores, and append to basename.
        index: A number to discriminate test cases that are otherwise indistinguishable.
    """
    fmt_split = fmt.split(":")
    if len(fmt_split) >= 2:
        F = fmt_split[0]
        XFMT = f'_{"_".join(fmt_split[1:])}'
    else:
        F = fmt
        XFMT = ""

    suffix = "" if index == 0 else str(index)

    OUTFILE = f"{name}_{alg}{XFMT}{suffix}.{F}"
    return OUTFILE


@pytest.mark.parametrize(
    "name,input,algorithm,format,flags,index",
    (
        pytest.param(
            c.name,
            c.input,
            c.algorithm,
            c.format,
            c.flags,
            c.index,
            marks=pytest.mark.xfail(strict=True) if c.xfail else (),
        )
        for c in TESTS
    ),
)
def test_graph(
    name: str,
    input: Path,
    algorithm: str,
    format: str,
    flags: list[str],
    index: int,
    tmp_path: Path,
):  # pylint: disable=too-many-arguments,too-many-positional-arguments
    """
    Run a single test.

    Args:
        tmp_path: Transient working directory, created by Pytest
    """
    MY_DIR = Path(__file__).resolve().parent
    if platform.system() == "Linux":
        REFDIR = MY_DIR / "linux.x86"
    elif platform.system() == "Darwin":
        REFDIR = MY_DIR / "macosx"
    elif platform.system() == "Windows":
        REFDIR = MY_DIR / "nshare"
    else:
        warnings.warn(f'Unrecognized system "{platform.system()}"')
        REFDIR = MY_DIR / "nshare"

    if input.suffix != ".gv":
        pytest.skip(f"Unknown graph spec, test {name} - ignoring")
    INFILE = GRAPHDIR / input

    OUTFILE = genOutname(name, algorithm, format, index)
    OUTPATH = tmp_path / OUTFILE
    testcmd = ["dot", f"-K{algorithm}", f"-T{format}"] + flags + ["-o", OUTPATH, INFILE]
    # FIXME: Remove when https://gitlab.com/graphviz/graphviz/-/issues/1790 is
    # fixed
    if platform.system() == "Windows" and name == "ps_user_shapes":
        pytest.skip(
            f"Skipping test {name}: using PostScript shapefile "
            "because it fails with Windows builds (#1790)"
        )

    env = os.environ.copy()
    if platform.system() == "Linux":
        env["FONTCONFIG_FILE"] = str(Path(__file__).parent / "fonts.conf")

    run(testcmd, env=env)
    doDiff(OUTPATH, REFDIR / OUTFILE, format)
