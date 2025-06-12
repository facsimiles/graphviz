#!/usr/bin/env python3

"""
Make TCL package index
"""

import argparse
import sys
from pathlib import Path
from typing import List


def main(args: List[str]) -> int:
    """entry point"""

    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "--file",
        type=Path,
        required=True,
        help="final shared object file to be loaded (doesn't need to be installed)",
    )
    parser.add_argument(
        "--name",
        type=str,
        required=True,
        help="name of extension",
    )
    parser.add_argument(
        "--version",
        type=str,
        required=True,
        help="version of extension",
    )
    options = parser.parse_args(args[1:])

    # for non-release versions, convert the '~dev.' portion into something compliant
    # with TCL’s version number rules
    version = options.version.replace("~dev.", "b")
    with open("pkgIndex.tcl", "wt", encoding="utf-8") as f:
        f.write(f'package ifneeded {options.name} {version} "\n')
        if "tk" in str(options.file):
            f.write("\tpackage require Tk 8.3\n")
        f.write(f'\tload [file join $dir {options.file}] {options.name}"\n')


if __name__ == "__main__":
    sys.exit(main(sys.argv))
