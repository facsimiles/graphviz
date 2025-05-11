#!/usr/bin/env python3

"""
make references to dynamic libraries relative

Autotools in combination with libtool builds binaries and libraries with links to their
dependents hard coded to absolute paths. This is a problem for relocatability (moving
files to another path and expecting binaries to still run). Persuading libtool to use a
relative path in rpath is hard. This script takes an alternative approach of rewriting
these absolute paths post-link to make them relative.

This script only supports macOS. Porting to other operating systems should not be hard.
"""

import argparse
import re
import shlex
import subprocess
import sys
from pathlib import Path
from typing import Iterator, Optional, Union


def run(*args: Union[str, Path]) -> str:
    """
    run a command, echoing it like `set -x`

    Args:
        args: command and options.

    Returns:
        the stdout of the process.
    """
    print(f"+ {shlex.join(str(a) for a in args)}")
    output = subprocess.check_output(args, text=True).strip()
    if output != "":
        print(output)
    return output


def otool(binary: Path) -> Iterator[Path]:
    """
    inspect the linked libraries of a binary/library

    Args:
        binary: target to inspect

    Yields:
        paths to dependent libraries
    """
    links = run("otool", "-L", binary)
    for line in links.split("\n"):
        if m := re.match(r"\s+(?P<path>[^ ]+)", line):
            yield Path(m.group("path"))


def relative_to(path: Path, prefix: Path) -> Optional[Path]:
    """wrapper around pathlib.Path.relative_to to simplify caller"""
    try:
        return path.relative_to(prefix)
    except ValueError:
        return None


def main(args: list[str]) -> int:
    """entry point"""

    # parse command line options
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("install_root", help="installation base directory")
    options = parser.parse_args(args[1:])

    root = Path(options.install_root)
    if not root.exists():
        sys.stderr.write(f"{root} does not exist\n")
        return -1
    exe = root / "bin"
    if not exe.exists():
        sys.stderr.write(f"{exe} does not exist\n")
        return -1
    lib = root / "lib"
    if not lib.exists():
        sys.stderr.write(f"{lib} does not exist\n")
        return -1
    plugins = lib / "graphviz"
    if not plugins.exists():
        sys.stderr.write(f"{plugins} does not exist\n")
        return -1

    # rewrite binary links to be relative
    for binary in exe.iterdir():
        if not binary.is_file():
            continue
        for linkee in otool(binary):
            if relative := relative_to(linkee, lib):
                run(
                    "install_name_tool",
                    "-change",
                    linkee,
                    f"@executable_path/../lib/{relative}",
                    binary,
                )

        # add an rpath that lets this binary (transitively) find libraries and
        # plugins
        try:
            run("install_name_tool", "-add_rpath", "@executable_path/../lib", binary)
        except subprocess.CalledProcessError:
            # ignore failure that is typically the result of this rpath already being
            # present
            pass

        # echo the resulting state of the binary for debugging
        run("otool", "-L", binary)

    # rewrite library links to be relative
    for library in lib.iterdir():
        if not library.is_file():
            continue
        for index, linkee in enumerate(otool(library)):
            if relative := relative_to(linkee, lib):
                if index == 0:
                    run(
                        "install_name_tool",
                        "-id",
                        f"@rpath/{relative}",
                        library,
                    )
                else:
                    run(
                        "install_name_tool",
                        "-change",
                        linkee,
                        f"@loader_path/{relative}",
                        library,
                    )

        run("otool", "-L", library)

    # rewrite plugin links to be relative
    for plugin in plugins.iterdir():
        if not plugin.is_file():
            continue
        for index, linkee in enumerate(otool(plugin)):
            if relative := relative_to(linkee, plugins):
                if index == 0:
                    run(
                        "install_name_tool",
                        "-id",
                        f"@rpath/graphviz/{relative}",
                        plugin,
                    )
                else:
                    run(
                        "install_name_tool",
                        "-change",
                        linkee,
                        f"@loader_path/{relative}",
                        plugin,
                    )
                continue
            if relative := relative_to(linkee, lib):
                run(
                    "install_name_tool",
                    "-change",
                    linkee,
                    f"@loader_path/../{relative}",
                    plugin,
                )
                continue

        run("otool", "-L", plugin)

    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv))
