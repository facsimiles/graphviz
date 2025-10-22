#!/usr/bin/env python3

"""
Generate version

Release version entry format : <major>.<minor>.<patch>

Stable release version output format     : <major>.<minor>.<patch>
Development release version output format: <major>.<minor>.<patch>~dev.<YYYYmmdd.HHMM>

The patch version of a development release should be the same as the
next stable release patch number. The string "~dev." and the
committer date will be added.

Example sequence:

Entry version   Entry collection          Output
2.44.1          stable                 => 2.44.1
2.44.2          development            => 2.44.2~dev.20200704.1652
2.44.2          stable                 => 2.44.2
2.44.3          development            => 2.44.3~dev.20200824.1337
"""

import argparse
import enum
import os
import re
import subprocess
import sys
from pathlib import Path
from typing import Tuple

CHANGELOG = Path(__file__).parent / "CHANGELOG.md"
assert CHANGELOG.exists(), "CHANGELOG.md file missing"


class Collection(enum.Enum):
    """release collection, as described above"""

    STABLE = enum.auto()
    DEVELOPMENT = enum.auto()


def get_version() -> Tuple[int, int, int, Collection]:
    """
    Derive a Graphviz version from the changelog information.

    Returns a tuple of major version, minor version, patch version,
    "stable"/"development".
    """

    # is this a development revision (as opposed to a stable release)?
    is_development = False

    with open(CHANGELOG, encoding="utf-8") as f:
        for line in f:
            # is this a version heading?
            m = re.match(r"## \[(?P<heading>[^\]]*)\]", line)
            if m is None:
                continue
            heading = m.group("heading")

            # is this the leading unreleased heading of a development version?
            UNRELEASED_PREFIX = "Unreleased ("
            if heading.startswith(UNRELEASED_PREFIX):
                is_development = True
                heading = heading[len(UNRELEASED_PREFIX) :]

            # extract the version components
            m = re.match(r"(?P<major>\d+)\.(?P<minor>\d+)\.(?P<patch>\d+)", heading)
            if m is None:
                raise RuntimeError(
                    "non-version ## heading encountered before seeing a "
                    "version heading"
                )

            major = int(m.group("major"))
            minor = int(m.group("minor"))
            patch = int(m.group("patch"))
            break

        else:
            # we read the whole changelog without finding a version
            raise RuntimeError("no version found")

    if is_development:
        coll = Collection.DEVELOPMENT
    else:
        coll = Collection.STABLE

    return major, minor, patch, coll


graphviz_date_format = "%Y%m%d.%H%M"
changelog_date_format = "%a %b %e %Y"

parser = argparse.ArgumentParser(description="Generate Graphviz version.")
parser.add_argument(
    "--committer-date-changelog",
    dest="date_format",
    action="store_const",
    const=changelog_date_format,
    help="Print changelog formatted committer date in UTC instead of version",
)
parser.add_argument(
    "--committer-date-graphviz",
    dest="date_format",
    action="store_const",
    const=graphviz_date_format,
    help="Print graphviz special formatted committer date in UTC " "instead of version",
)
parser.add_argument(
    "--major",
    dest="component",
    action="store_const",
    const="major",
    help="Print major version",
)
parser.add_argument(
    "--minor",
    dest="component",
    action="store_const",
    const="minor",
    help="Print minor version",
)
parser.add_argument(
    "--patch",
    dest="component",
    action="store_const",
    const="patch",
    help="Print patch version",
)
parser.add_argument(
    "--pre-release",
    dest="component",
    action="store_const",
    const="pre_release",
    help="Print separator (~) and pre-release identifiers",
)

args = parser.parse_args()

date_format = args.date_format or graphviz_date_format

major_version, minor_version, patch_version, collection = get_version()

if collection == Collection.DEVELOPMENT:
    pre_release = "~dev"
else:
    pre_release = ""

committer_date = "0"
if pre_release != "" or args.date_format:
    os.environ["TZ"] = "UTC"
    try:
        p = subprocess.run(
            [
                "git",
                "log",
                "-n",
                "1",
                "--format=%cd",
                f"--date=format-local:{date_format}",
            ],
            stdout=subprocess.PIPE,
            cwd=Path(__file__).resolve().parent,
            check=True,
            universal_newlines=True,
        )
        committer_date = p.stdout.strip()
    except FileNotFoundError:
        sys.stderr.write("Warning: Git is not installed: setting version date to 0.\n")
    except subprocess.CalledProcessError:
        sys.stderr.write(
            "Warning: build not started in a Git clone: setting version date to 0.\n"
        )

if pre_release != "":
    # add committer date
    pre_release += f".{committer_date}"

if args.date_format:
    print(committer_date)
elif args.component == "major":
    print(major_version)
elif args.component == "minor":
    print(minor_version)
elif args.component == "patch":
    print(patch_version)
elif args.component == "pre_release":
    print(pre_release)
else:
    print(f"{major_version}.{minor_version}.{patch_version}{pre_release}")
