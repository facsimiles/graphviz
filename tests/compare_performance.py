#!/usr/bin/env python3

"""
compare the runtime performance of two different versions of Graphviz

When making changes to Graphviz’ code that may affect performance, you can use this
script to evaluate the effect of your changes:

  python3 compare_performance.py \
    /path/to/install/before/bin/dot /path/to/install/after/bin/dot

This will run various known expensive workloads and report the relative performance of
the two or more versions you gave it as tabulated results.
"""

import argparse
import io
import os
import platform
import shlex
import shutil
import subprocess
import sys
import time
from pathlib import Path
from typing import Union

import tabulate

MY_DIR = Path(__file__).resolve().parent
"""
path to our containing directory
"""

TESTS: dict[str, list[Union[str, Path]]] = {
    "#1172 1": ["dot", "-Ln3", "-Txdot", MY_DIR / "1172_1.xdot"],
    "#1172 2": ["dot", "-Ln3", "-Txdot", MY_DIR / "1172_2.xdot"],
    "#1652": ["neato", "-Tsvg", MY_DIR / "1652.dot"],
    "#1718": ["circo", "-Tsvg", MY_DIR / "1718.dot"],
    "#1864": ["neato", "-Tsvg", MY_DIR / "1864.dot"],
    "#2064": [
        "dot",
        "-Gnslimit=2",
        "-Gnslimit1=2",
        "-Gmaxiter=5000",
        "-Tsvg",
        MY_DIR / "2064.dot",
    ],
    "#2095": ["dot", "-Tsvg", MY_DIR / "2095_1.dot"],
    "#2108": ["dot", "-Tsvg", MY_DIR / "2108.dot"],
    "#2222": ["dot", "-Tsvg", MY_DIR / "2222.dot"],
    "#2343": ["dot", "-Tpng", MY_DIR / "2343.dot"],
    "#2475 1": [
        "dot",
        "-Gnslimit=2",
        "-Gnslimit1=2",
        "-Gmclimit=.33",
        "-Gremincross=false",
        "-Gsearchsize=20",
        "-Gsplines=false",
        "-Tsvg",
        MY_DIR / "2475_1.dot",
    ],
    "#2475 2": [
        "dot",
        "-Gnslimit=2",
        "-Gnslimit1=2",
        "-Gmclimit=.33",
        "-Gremincross=false",
        "-Gsearchsize=20",
        "-Gsplines=false",
        "-Tsvg",
        MY_DIR / "2475_2.dot",
    ],
    "#2593": ["ccomps", MY_DIR / "2593.dot"],
    "#2621": ["dot", "-Tpng", MY_DIR / "2621.dot"],
    "#2646": ["dot", "-Tpdf", MY_DIR / "2646.dot"],
    "!2854": ["dot", "-Tsvg", MY_DIR / "2854.dot"],
}
"""
relevant workloads to evaluate

Entries are a test name mapped to a command to run. Commands will all have an
implicit `-o /dev/null` added to them when executed.
"""


def run(args: list[Union[str, Path]], root: Union[str, Path]):
    """
    run a command, pre-echoing it like Bash’s `set -x`

    Args:
        args: Command line to run
        root: Root directory of the Graphviz installation under test
    """
    assert len(args) > 0

    exe = root / "bin"
    lib = root / "lib"

    env = os.environ.copy()
    if platform.system() == "Darwin":
        if "DYLD_LIBRARY_PATH" in env:
            env["DYLD_LIBRARY_PATH"] = f"{lib}{os.pathsep}{env['DYLD_LIBRARY_PATH']}"
            prefix = f'DYLD_LIBRARY_PATH="{lib}{os.pathsep}${{DYLD_LIBRARY_PATH}}"'
        else:
            env["DYLD_LIBRARY_PATH"] = str(lib)
            prefix = f'DYLD_LIBRARY_PATH="{lib}"'
    else:
        if "LD_LIBRARY_PATH" in env:
            env["LD_LIBRARY_PATH"] = f"{lib}{os.pathsep}{env['LD_LIBRARY_PATH']}"
            prefix = f'LD_LIBRARY_PATH="{lib}{os.pathsep}${{LD_LIBRARY_PATH}}"'
        else:
            env["LD_LIBRARY_PATH"] = str(lib)
            prefix = f'LD_LIBRARY_PATH="{lib}"'

    arg0 = shutil.which(args[0], path=exe)
    assert arg0 is not None, "{args[0]} not found in installation {root}"
    argv = [arg0] + args[1:] + ["-o", os.devnull]

    print(f"+ env {prefix} {shlex.join(str(x) for x in argv)}", flush=True)
    proc = subprocess.run(argv, check=False)
    if proc.returncode != 0:
        print(f"warning: command returned {proc.returncode}", file=sys.stderr)


def pretty_time(duration: float):
    """turn a count of seconds into XXhXXmXXs"""
    hours = int(duration / 60 / 60)
    minutes = int(duration / 60 % 60)
    seconds = int(duration % 60)
    return f"{hours}h{minutes:02}m{seconds:02}s"


def main(args: list[str]) -> int:
    """entry point"""

    # parse command line options
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "candidate",
        type=argparse.FileType("rb"),
        nargs="+",
        help="Graphviz binaries to compare",
    )
    options = parser.parse_args(args[1:])

    # derive installation roots
    roots = []
    for dot in options.candidate:
        exe = Path(dot.name).resolve()
        roots += [exe.parents[1]]
    headers = ["test case"] + roots

    print(f"Comparing {[str(r) for r in roots]}…", flush=True)

    results: list[list[str]] = []
    for name, cmd in TESTS.items():
        result = []
        for root in roots:
            start = time.monotonic()
            run(cmd, root)
            stop = time.monotonic()
            result += [stop - start]

            # dump results progress, so that if the user interrupts they still have
            # something partial to analyze
            row = [name]
            for index, r in enumerate(result):
                cell = io.StringIO()
                if index != 0:
                    if r < result[0]:
                        cell.write("\033[32m")  # green
                    elif r > result[0]:
                        cell.write("\033[31m")  # red
                cell.write(pretty_time(r))
                if index != 0:
                    cell.write(f" ({int((r - result[0]) / result[0]  * 100)}%)\033[0m")
                row += [cell.getvalue()]
            print(
                tabulate.tabulate(
                    results + [row], headers=headers, tablefmt="simple_outline"
                ),
                flush=True,
            )
        results += [row]

    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv))
