#!/usr/bin/env python3

"""
MSVC compiler wrapper

Configuring MSVC’s command line binary, `cl`, to run standalone is complicated and
involves interaction with fragile programming environments like PowerShell and/or Batch
files. To avoid relying on this, this script outsources compiler discovery and
configuration to CMake. This allows standalone compilation using any version of MSVC
CMake is able to successfully discover and use.

Though this script is intended as an MSVC wrapper, it is agnostic to the C compiler
being called. It can be used in combination with any C compiler that understands MSVC
options.
"""

import io
import os
import re
import shlex
import subprocess
import sys
import tempfile
import textwrap
from pathlib import Path
from typing import Optional, Union


def run(args: list[Union[Path, str]]):
    """run an external command, echoing it first"""
    print(f"+ {shlex.join(str(x) for x in args)}", flush=True)
    subprocess.run(args, check=True)


def safe_path(path: Union[Path, str]) -> str:
    """
    escape a path for use in a CMake file

    Windows paths typically contain backslashes. E.g. C:\foo\bar. When inserted as-is
    into CMakeLists.txt files, these are interpreted by CMake as escape sequences. To
    properly propagate such paths through to CMake, they need to be either (1)
    double-backslash escaped or (2) replaced with forward slashes.

    Args:
        path: An unescaped path.

    Returns:
        An equivalent, safe to write into a CMakeLists.txt.
    """
    return str(path).replace("\\", "/")


def make_cmakelists(args: list[str]) -> str:
    """
    reverse `cl` command line options into equivalent CMakeLists.txt content

    Only a subset of `cl` options are recognized by this function. It can be extended to
    recognize further options if/when they are necessary.

    Args:
        args: Command line options as would be given to the `cl` binary. `args[0]` is
            expected to be `"cl"`.

    Returns:
        Content suitable for writing into a CMakeLists.txt. Any paths referenced will be
        absolute, so this content can be written to an arbitrary directory.
    """

    # MSVC runtime library to link
    crt: Optional[str] = None

    # C sources to compile
    sources: list[Path] = []

    # binary output name
    destination: Path = Path("main").absolute()

    # ISO C standard to use, C{std}
    std: Optional[int] = None

    # macro definitions
    definitions: list[str] = []

    # extra C compiler flags
    cflags: list[str] = []

    # directories to put in the include path
    includes: list[Path] = []

    # libraries to link
    libs: list[str] = []

    i = 1
    while i < len(args):

        if m := re.match(r"[\-/]D(?P<definition>.*)$", args[i]):
            definitions += [m.group("definition")]
            i += 1
            continue

        if re.match(r"[\-/]Fe:$", args[i]):
            if i + 1 == len(args):
                raise RuntimeError("/Fe: option missing parameter")
            destination = Path(args[i + 1]).absolute()
            i += 2
            continue

        if re.match(r"[\-/]fsanitize\b", args[i]):
            # pass these through as-is
            cflags += [args[i]]
            i += 1
            continue

        if re.match(r"[\-/]I$", args[i]):
            if i + 1 == len(args):
                raise RuntimeError("/I option missing parameter")
            includes += [Path(args[i + 1]).resolve()]
            i += 2
            continue

        if m := re.match(r"[\-/]I(?P<include>.+)$", args[i]):
            includes += [Path(m.group("include")).resolve()]
            i += 1
            continue

        if re.match(r"[\-/]link$", args[i]):
            # swallow everything else as libraries to link
            libs += args[i + 1 :]
            break

        if re.match(r"[\-/]nologo$", args[i]):
            # CMake will set this automatically
            i += 1
            continue

        if re.match(r"[\-/]MD$", args[i]):
            crt = "MultiThreadedDLL"
            i += 1
            continue

        if re.match(r"[\-/]MDd$", args[i]):
            crt = "MultiThreadedDebugDLL"
            i += 1
            continue

        if m := re.match(r"[\-/]std:c(?P<std>\d+)$", args[i]):
            std = int(m.group("std"))
            i += 1
            continue

        if re.match(r"[\-/]Wall$", args[i]):
            cflags += [args[i]]
            i += 1
            continue

        # pass through targeted warning enable/disable
        if re.match(r"[\-/]w[de]\d+$", args[i]):
            cflags += [args[i]]
            i += 1
            continue

        if re.match(r"\w", args[i]):
            sources += [Path(args[i]).absolute()]
            i += 1
            continue

        raise RuntimeError(f"unrecognized cl option {args[i]}")

    res = io.StringIO()

    res.write(
        textwrap.dedent(
            """\
    cmake_minimum_required(VERSION 3.21 FATAL_ERROR)
    project(temp C)
    """
        )
    )

    if std is not None:
        res.write(
            textwrap.dedent(
                f"""\
        set(CMAKE_C_STANDARD {std})
        set(CMAKE_C_STANDARD_REQUIRED ON)
        """
            )
        )
        if std >= 11:
            res.write(
                textwrap.dedent(
                    """\
            if(${CMAKE_C_COMPILER_ID} STREQUAL MSVC)
              set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /experimental:c11atomics")
            endif()
            """
                )
            )

    res.write("add_executable(main\n")
    for s in sources:
        res.write(f"  {safe_path(s)}\n")
    res.write(")\n")

    for d in definitions:
        res.write(f"target_compile_definitions(main PRIVATE {d})\n")

    if len(cflags) > 0:
        res.write("include(CheckCCompilerFlag)\n")
    for index, f in enumerate(cflags):
        res.write(
            textwrap.dedent(
                f"""\
        check_c_compiler_flag({f} WAS_SUPPORTED{index})
        if(NOT WAS_SUPPORTED{index})
          message(FATAL_ERROR "compiler does not understand {f}")
        endif()
        target_compile_options(main PRIVATE {f})
        """
            )
        )

    for i in includes:
        res.write(f"target_include_directories(main PRIVATE {safe_path(i)})\n")

    if crt is not None:
        res.write(
            f"set_target_properties(main PROPERTIES MSVC_RUNTIME_LIBRARY {crt})\n"
        )

    for index, l in enumerate(libs):
        if m := re.match(r"(?P<libname>\w+)\.lib$", l):
            res.write(
                textwrap.dedent(
                    f"""\
            find_library(LIB{index} {m.group('libname')} REQUIRED)
            target_link_libraries(main PRIVATE ${{LIB{index}}})
            """
                )
            )
        else:
            lib = Path(l).resolve()
            res.write(
                textwrap.dedent(
                    f"""\
            find_library(
              LIB{index} {lib.name}
              PATHS {safe_path(lib.parent)}
              REQUIRED
              NO_DEFAULT_PATH
            )
            target_link_libraries(main PRIVATE ${{LIB{index}}})
            """
                )
            )

    res.write(
        textwrap.dedent(
            f"""\
    install(PROGRAMS
      $<TARGET_FILE:main>
      DESTINATION {safe_path(destination.parent)}
      RENAME {destination.name}
    )
    """
        )
    )

    return res.getvalue()


def main(args: list[str]) -> int:
    """entry point"""

    # construct CMakeLists.txt content equivalent to our options
    cmakelists = make_cmakelists(args)

    # dump this for easier debugging
    print(f'{"-" * 37} CMakeLists.txt {"-" * 37}', flush=True)
    print(f'{cmakelists}{"-" * 80}', flush=True)

    with tempfile.TemporaryDirectory() as t:
        tmp = Path(t)

        # setup a hierarchy for a CMake build
        source = tmp / "src"
        source.mkdir(parents=True)
        build = tmp / "build"
        (source / "CMakeLists.txt").write_text(cmakelists, encoding="utf-8")

        # determine x86 vs x64
        if platform := os.environ.get("project_platform"):
            arch = ["-A", platform]
        else:
            arch = []

        # configure and run the build
        paranoia = ["--log-level=VERBOSE", "--warn-uninitialized", "-Werror-dev"]
        run(["cmake"] + arch + ["-S", source, "-B", build] + paranoia)
        run(["cmake", "--build", build, "--verbose"])
        run(["cmake", "--build", build, "--target", "install", "--verbose"])

    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv))
