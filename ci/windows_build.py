#!/usr/bin/env python3

"""Graphviz CI script for compilation on Windows"""

import argparse
import io
import itertools
import os
import shlex
import shutil
import subprocess
import sys
import textwrap
from pathlib import Path
from typing import Optional, TextIO, Union


def run(
    args: list[Union[str, Path]],
    cwd: Path,
    env: dict[str, str],
    out: Optional[TextIO] = None,
) -> None:
    """run a command, echoing it beforehand"""

    print(f"+ {shlex.join(str(x) for x in args)}")
    p = subprocess.run(
        args,
        stdout=subprocess.PIPE,
        stderr=subprocess.STDOUT,
        cwd=cwd,
        check=False,
        text=True,
        env=env,
    )
    sys.stderr.write(p.stdout)
    if out is not None:
        out.write(p.stdout)
    p.check_returncode()


def require(program: str, fallback: Path, env: dict[str, str], log: TextIO) -> None:
    """
    detect if a given program is missing from the default $Path and needs a fallback

    Args:
        program: The name of a program to look for, without its “.exe” suffix.
        fallback: A path at which to find a version of that program. This alternate
            version will be used if the sought program is not found in $Path.
        env: An environment to use when searching for the program. If the program needs
            a fallback, the $Path variable in this environment will be adjusted.
        log: Sink to write informational messages to.
    """
    assert (
        fallback / f"{program}.exe"
    ).exists(), "{fallback} is not a valid fallback path for {program}"
    which = shutil.which(f"{program}.exe", path=env["PATH"])
    if which is None:
        log.write(
            textwrap.dedent(
                f"""\
        {program} not found
        Fallback needed for: {program}
        Setting up fallback path for: {program} to {fallback}
        """
            )
        )
        env["PATH"] = f"{fallback}{os.pathsep}{env['PATH']}"
    else:
        log.write(f"Found {program} at {which}\n")


def main(args: list[str]) -> int:
    """entry point"""

    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "--build-shared-libs",
        choices=("ON", "OFF"),
        default="ON",
        help="control shared libraries selection",
    )
    parser.add_argument(
        "--configuration",
        choices=("Debug", "Release"),
        required=True,
        help="build configuration to select",
    )
    parser.add_argument(
        "--platform", choices=("Win32", "x64"), required=True, help="target platform"
    )
    options = parser.parse_args(args[1:])

    # find the repository root directory
    root = Path(__file__).resolve().parent.parent

    # retrieve submodules, dependencies are stored there
    run(["git", "submodule", "update", "--init", "--depth=1"], root, None)

    # an environment we will use during configuration/compilation
    build_env = os.environ.copy()

    # buffer for output so we can report warning count later
    log = io.StringIO()

    # find some external build dependencies
    utilities = root / "windows/dependencies/graphviz-build-utilities"
    require("win_bison", utilities / "winflexbison", build_env, log)
    require("win_flex", utilities / "winflexbison", build_env, log)
    require("makensis", utilities / "NSIS/Bin", build_env, log)

    build = root / "build"
    if build.exists():
        shutil.rmtree(build)
    build.mkdir(parents=True)
    run(["cmake", "--version"], build, None, log)
    run(
        [
            "cmake",
            "--log-level=VERBOSE",
            "-G",
            "Visual Studio 17 2022",
            "-A",
            options.platform,
            f"-DBUILD_SHARED_LIBS={options.build_shared_libs}",
            "-Dwith_cxx_api=ON",
            "-DENABLE_LTDL=ON",
            "-DWITH_EXPAT=ON",
            "-DWITH_GVEDIT=OFF",
            "-DWITH_ZLIB=ON",
            "--warn-uninitialized",
            "-Werror=dev",
            "..",
        ],
        build,
        build_env,
        log,
    )
    run(
        ["cmake", "--build", ".", "--config", options.configuration],
        build,
        build_env,
        log,
    )
    run(["cpack", "-C", options.configuration], build, build_env, log)

    # report warning count
    warning_count = log.getvalue().count(" warning ")
    metrics = Path("metrics.txt")
    summary = f"{os.environ['CI_JOB_NAME']}-warnings {warning_count}"
    print(summary)
    metrics.write_text(f"{summary}\n", encoding="utf-8")

    # derive Graphviz version
    version_buffer = io.StringIO()
    run([sys.executable, "gen_version.py"], root, None, version_buffer)
    gv_version = version_buffer.getvalue().strip()

    # find the installer
    installers = list(build.glob("Graphviz*.exe"))
    assert len(installers) == 1, "failed to find Graphviz installer"
    installer = build / installers[0]

    # install Graphviz
    install_dir = "C:\\Graphviz"
    run([installer, "/S", f"/D={install_dir}"], root, None)

    # which Windows interface are we targeting?
    if options.platform == "x64":
        api = "win64"
    else:
        api = "win32"

    # move the installer to the location expected by CI archiving steps
    dst = build / f"graphviz-install-{gv_version}-{api}.exe"
    shutil.move(installer, dst)

    # an environment we will use during testing
    test_env = os.environ.copy()

    test_env["PATH"] = f"{install_dir}\\bin;{test_env.get('PATH', '')}"
    test_env["CFLAGS"] = f"{test_env.get('CFLAGS', '')} -I{install_dir}\\include"
    test_env["LIB"] = f"{test_env.get('LIB', '')};{install_dir}\\lib"
    test_env["graphviz_ROOT"] = install_dir

    # run the test suite
    run(
        [
            sys.executable,
            "-m",
            "pytest",
            "-m",
            "not slow",
            "--junit-xml=report.xml",
            "ci/tests.py",
            "tests",
        ],
        root,
        test_env,
    )

    # create artifacts to archive
    ident = "windows"
    version_id = "10"
    packages = (
        root / "Packages" / ident / version_id / "cmake" / os.environ["configuration"]
    )
    packages.mkdir(parents=True)
    for src in itertools.chain(build.glob("*.exe"), build.glob("*.zip")):
        print(f"+ mv {shlex.join(str(a) for a in [src, packages])}")
        shutil.move(src, packages)

    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv))
