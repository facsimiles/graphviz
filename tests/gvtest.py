"""common Graphviz test functionality"""

import os
import platform
import re
import shlex
import shutil
import subprocess
import sys
import sysconfig
import tempfile
from pathlib import Path
from typing import Optional, Union

ROOT = Path(__file__).resolve().parent.parent
"""absolute path to the root of the repository"""


def run_raw(args: list[Union[Path, str]], **kwargs) -> Optional[Union[bytes, str]]:
    """
    execute an external command

    This function wraps up the common pattern of running a program then (1) if it fails,
    let its output propagate to stdout/stderr and throw a Python exception, or (2) if it
    succeeds, return its output.

    Args:
        args: Command and options to execute.
        kwargs: Optional extra arguments to `subprocess.run`.

    Return:
        The command’s stdout output.
    """

    # dump the command being run for the user to observe if the test fails
    print(f"+ {shlex.join(str(x) for x in args)}")

    is_stdout_decoded = kwargs.get("universal_newlines") or kwargs.get("text")

    if kwargs.get("stdout", subprocess.PIPE) == subprocess.PIPE:
        is_stdout_captured = True
        kwargs["stdout"] = subprocess.PIPE
    else:
        is_stdout_captured = False

    proc = subprocess.run(args, check=False, **kwargs)

    if proc.returncode != 0:
        if is_stdout_captured:
            if is_stdout_decoded:
                sys.stdout.write(proc.stdout)
            else:
                sys.stdout.buffer.write(proc.stdout)
    proc.check_returncode()

    return proc.stdout


def run(args: list[Union[Path, str]], **kwargs) -> Optional[str]:
    """
    execute an external command that takes/returns textual input/output

    This function is the same as `run_raw` but encodes/decodes between bytes in the
    external world and UTF-8 strings within Python.

    Args:
        args: Command and options to execute.
        kwargs: Optional extra arguments to `subprocess.run`.

    Return:
        The command’s stdout output.
    """
    return run_raw(args, text=True, **kwargs)


def compile_c(
    src: Path,
    cflags: list[str] = None,
    link: list[Union[Path, str]] = None,
    dst: Optional[Union[Path, str]] = None,
) -> Path:
    """compile a C program"""

    if cflags is None:
        cflags = []
    if link is None:
        link = []

    # include compiler flags from both the caller and the environment
    cflags = os.environ.get("CFLAGS", "").split() + cflags
    ldflags = os.environ.get("LDFLAGS", "").split()

    # enable most warnings
    if platform.system() == "Windows" and not is_mingw():
        # disable:
        #   • C4820: warns about padding introduced at the end of a struct
        #   • C5045: warns about opportunities for Spectre mitigations
        # We do not care about these in test code.
        cflags = ["/Wall", "/wd4820", "/wd5045"] + cflags
    else:
        cflags = ["-Wall", "-Wextra"] + cflags

    # on macOS, ensure the compiled binary can find Graphviz libraries at runtime
    if is_macos():
        dot_exe = shutil.which("dot")
        assert dot_exe is not None, "`dot` not found"
        lib = Path(dot_exe).parents[1] / "lib"
        ldflags += [f"-Wl,-rpath,{lib}"]

    # expand link libraries into command line options
    pkgconf = shutil.which("pkg-config") or shutil.which("pkgconf")
    if pkgconf is not None and not is_cmake():
        # assume pkg-config can pick up Graphviz’ .pc files
        libraries = []
        for l in link:
            # for absolute paths, assume we need no pkg-config lookup
            if Path(l).is_absolute() and Path(l).exists():
                # flush any pending pkg-config lookup to roughly keep the library
                # ordering the caller requested
                if len(libraries) > 0:
                    cflags += run([pkgconf, "--cflags", "--"] + libraries).split()
                    ldflags += run([pkgconf, "--libs", "--"] + libraries).split()
                    libraries = []
                ldflags += [l]
            else:
                libraries += [f"lib{l}"]
        if len(libraries) > 0:
            cflags += run([pkgconf, "--cflags", "--"] + libraries).split()
            ldflags += run([pkgconf, "--libs", "--"] + libraries).split()
    elif platform.system() == "Windows" and not is_mingw():
        if len(link) > 0:
            if not is_static_build():
                cflags += ["-DGVDLL=1"]
            ldflags += ["-link"]
            for l in link:
                if Path(l).is_absolute() and Path(l).exists():
                    ldflags += [l]
                else:
                    ldflags += [f"{l}.lib"]
    else:
        for l in link:
            if Path(l).is_absolute() and Path(l).exists():
                ldflags += [l]
            else:
                ldflags += [f"-l{l}"]

    # if the user did not give us destination, use a temporary path
    if dst is None:
        _, dst = tempfile.mkstemp(".exe")

    if platform.system() == "Windows" and not is_mingw():
        # determine which runtime library option we need
        rtflag = "-MDd" if os.environ.get("configuration") == "Debug" else "-MD"

        # construct an invocation of MSVC
        cl = Path(__file__).parent / "cl.py"
        args = (
            [sys.executable, cl, "/std:c17", src, "-Fe:", dst, "-nologo", rtflag]
            + cflags
            + ldflags
        )

    else:
        # construct an invocation of the default C compiler
        cc = os.environ.get("CC", "cc")
        args = [cc, "-std=c17", src, "-o", dst] + cflags + ldflags

    # compile the program
    try:
        run_raw(args)
    except subprocess.CalledProcessError:
        try:
            os.remove(dst)
        except (FileNotFoundError, PermissionError):
            pass
        raise

    return dst


def dot(
    T: str, source_file: Optional[Path] = None, source: Optional[str] = None
) -> Union[bytes, str]:
    """
    run Graphviz on the given source file or text

    Args:
      T: Output format, as would be supplied to `-T` on the command line.
      source_file: Input file to parse. Can be `None` if `source` is provided
        instead.
      source: Input text to parse. Can be `None` if `source_file` is provided
        instead.

    Returns:
      Dot output as text if a textual format was selected or as binary if not.
    """

    assert (
        source_file is not None or source is not None
    ), "one of `source_file` or `source` needs to be provided"

    # is the output format a textual format?
    output_is_text = T in (
        "canon",
        "cmapx",
        "dot",
        "fig",
        "gv",
        "json",
        "pic",
        "svg",
        "svg_inline",
        "xdot",
        "xdot1.2",
        "xdot1.4",
    )

    kwargs = {}
    if output_is_text:
        kwargs["encoding"] = "utf-8"
        kwargs["text"] = True

    args = ["dot", f"-T{T}"]

    if source_file is not None:
        args += [source_file]
    elif not output_is_text:
        kwargs["input"] = source.encode("utf-8")
    else:
        kwargs["input"] = source

    # dump the command being run for the user to observe if the test fails
    print(f"+ {shlex.join(str(x) for x in args)}")

    proc = subprocess.run(args, stdout=subprocess.PIPE, check=True, **kwargs)
    return proc.stdout


def freedesktop_os_release() -> dict[str, str]:
    """
    polyfill for `platform.freedesktop_os_release`
    """
    release = {}
    os_release = Path("/etc/os-release")
    if os_release.exists():
        with open(os_release, "rt", encoding="utf-8") as f:
            for line in f.readlines():
                if line.startswith("#") or "=" not in line:
                    continue
                key, _, value = (x.strip() for x in line.partition("="))
                # remove quotes
                if len(value) >= 2 and value[0] == '"' and value[-1] == '"':
                    value = value[1:-1]
                release[key] = value
    return release


def gvpr(program: Path) -> str:
    """run a GVPR program on empty input"""

    assert which("gvpr") is not None, "attempt to run GVPR without it available"

    return run(["gvpr", "-f", program], stdin=subprocess.DEVNULL)


def build_system() -> str:
    """get the build system name"""
    if platform.system() == "Windows":
        return "cmake"
    return os.getenv("build_system")


def is_asan_instrumented(binary: Path) -> bool:
    """
    is the given binary using Address Sanitizer?

    This function returns false negatives (incorrectly says “False” for
    ASan-instrumented binaries) if there is no tool for inspecting symbol tables or if
    the symbol table has been stripped.
    """
    assert binary.exists()

    # Get the symbol table of the binary. We deliberately avoid `text=True` to tolerate
    # non-ASCII bytes in the symbol table.
    if objdump := shutil.which("objdump"):
        symbols = run_raw([objdump, "--syms", binary])
    elif llvm_objdump := shutil.which("llvm-objdump"):
        symbols = run_raw([llvm_objdump, "--syms", binary])
    elif dumpbin := shutil.which("dumpbin"):
        dependencies = run_raw([dumpbin, "/DEPENDENTS", binary])
        # Look for the ASan DLL dependency
        return (
            re.search(rb"\bclang_rt\.asan_dynamic-.*\.dll\b", dependencies) is not None
        )
    else:
        # if we cannot examine the symbol table, assume ASan is not in use
        return False

    # Look for a reference to a known ASan symbol. This exists even if ASan has been
    # statically linked (`-static-libasan`).
    return re.search(rb"\b__asan_init\b", symbols) is not None


def is_autotools() -> bool:
    """was the Grapviz under test built with Autotools?"""
    if platform.system() == "Windows":
        return False
    return not is_cmake()


def is_cmake() -> bool:
    """was the Graphviz under test built with CMake?"""
    return build_system() == "cmake"


def is_fedora() -> bool:
    """is the current environment Fedora?"""
    return freedesktop_os_release().get("ID") == "fedora"


def is_fedora_43() -> bool:
    """is the current environment Fedora 43?"""
    if not is_fedora():
        return False
    return freedesktop_os_release().get("VERSION_ID") == "43"


def is_macos() -> bool:
    """is the current platform macOS?"""
    return platform.system() == "Darwin"


def is_mingw() -> bool:
    """is the current platform MinGW?"""
    return "mingw" in sysconfig.get_platform()


def is_static_build() -> bool:
    """was the build we are testing done with static linking?"""

    if os.environ.get("BUILD_SHARED_LIBS") == "OFF":
        return True

    # answer “no” to other scenarios, even when some of them are statically
    # linked, because the test suite is unaffected
    return False


def is_ubuntu() -> bool:
    """is the current environment Ubuntu?"""
    return freedesktop_os_release().get("ID") == "ubuntu"


def is_ubuntu_2404() -> bool:
    """is the current environment Ubuntu 24.04?"""
    if not is_ubuntu():
        return False
    return freedesktop_os_release().get("VERSION_ID") == "24.04"


def is_ubuntu_2510() -> bool:
    """is the current environment Ubuntu 25.10?"""
    if not is_ubuntu():
        return False
    return freedesktop_os_release().get("VERSION_ID") == "25.10"


def remove_asan_summary(s: str) -> str:
    """
    Remove the “Suppressions used…” informational text Address Sanitizer prints.
    This is useful but can interfere with our ability to scan output for expected
    content.
    """
    return re.sub(r"-+\nSuppressions used:[^\-]+-+\n?\n?", "", s)


def remove_xtype_warnings(s: str) -> str:
    """
    Remove macOS XType warnings from a string. These appear to be harmless, but
    occur in CI.
    """

    # avoid doing this anywhere except on macOS
    if not is_macos():
        return s

    return re.sub(r"^.* XType: .*\.$\n", "", s, flags=re.MULTILINE)


def is_rocky(wanted_version_id: Optional[str] = None) -> bool:
    """
    is the current environment Rocky Linux? And if specified, is it version `wanted_version_id`?
    """
    if freedesktop_os_release().get("ID") != "rocky":
        return False
    if wanted_version_id is None:
        return True
    version_id = freedesktop_os_release().get("VERSION_ID")
    if version_id is None:
        return False
    return re.match(rf"{wanted_version_id}\b", version_id) is not None


def is_rocky_8() -> bool:
    """
    is the current environment Rocky Linux 8?
    """
    return is_rocky("8")


def is_rocky_10() -> bool:
    """
    is the current environment Rocky Linux 10?
    """
    return is_rocky("10")


def has_sandbox() -> bool:
    """
    do we have an available sandbox mechanism?

    The logic here should correspond to available `Sandbox` back ends in
    ../cmd/dot/dot_sandbox.
    """
    if shutil.which("bwrap"):
        return True
    if platform.system() == "Darwin":
        return True
    return False


def plugin_version() -> tuple[int, int, int]:
    """get the Graphviz plugin libtool revision, current, age"""

    # treat configure.ac as the canonical source of this information
    configure_ac = Path(__file__).resolve().parents[1] / "configure.ac"

    # parse out the version components
    current: Optional[int] = None
    revision: Optional[int] = None
    age: Optional[int] = None
    with open(configure_ac, "rt", encoding="utf-8") as f:
        for line in f:
            if m := re.match(r"\s*GVPLUGIN_CURRENT\s*=\s*(?P<current>\d+)\s*$", line):
                current = int(m.group("current"))
                if revision is not None and age is not None:
                    break
                continue
            if m := re.match(r"\s*GVPLUGIN_REVISION\s*=\s*(?P<revision>\d+)\s*$", line):
                revision = int(m.group("revision"))
                if current is not None and age is not None:
                    break
                continue
            if m := re.match(r"\s*GVPLUGIN_AGE\s*=\s*(?P<age>\d+)\s*$", line):
                age = int(m.group("age"))
                if current is not None and revision is not None:
                    break
                continue
    assert current is not None, "failed to parse plugin version"
    assert revision is not None, "failed to parse plugin version"
    assert age is not None, "failed to parse plugin version"

    return current, revision, age


def run_c(
    src: Path,
    args: list[Union[Path, str]] = None,
    input: str = "",
    cflags: list[str] = None,
    link: list[Union[Path, str]] = None,
) -> tuple[str, str]:
    """compile and run a C program"""

    if args is None:
        args = []
    if cflags is None:
        cflags = []
    if link is None:
        link = []

    # create some temporary space to work in
    with tempfile.TemporaryDirectory() as tmp:
        # output filename to write our compiled code to
        exe = Path(tmp) / "a.exe"

        # compile the program
        compile_c(src, cflags, link, exe)

        # dump the command being run for the user to observe if the test fails
        argv = [exe] + args
        print(f"+ {shlex.join(str(x) for x in argv)}")

        input_bytes = None
        if input is not None:
            input_bytes = input.encode("utf-8")

        # run it
        p = subprocess.run(argv, input=input_bytes, capture_output=True, check=False)

        # decode output manually rather than using `text=True` above to avoid exceptions
        # from non-UTF-8 bytes in the output
        stdout = p.stdout.decode("utf-8", "replace")
        stderr = p.stderr.decode("utf-8", "replace")

        # check it succeeded
        if p.returncode != 0:
            sys.stdout.write(stdout)
            sys.stderr.write(stderr)
        p.check_returncode()

        return stdout, stderr


def which(cmd: str) -> Optional[Path]:
    """
    `shutil.which` but only return results that are adjacent to the main Graphviz
    executable.

    Graphviz has numerous optional components. So if you choose to e.g. disable
    `mingle` during compilation and installation, you may find that a naive
    `shutil.which("mingle")` after this returns the `mingle` from a prior
    system-wide installation of the full Graphviz. This is undesirable during
    testing, as this older `mingle` will then load shared libraries from the
    installation you just created, most likely crashing.
    """

    # try a straightforward lookup of the command
    abs_cmd = shutil.which(cmd)
    if abs_cmd is None:
        return None
    abs_cmd = Path(abs_cmd)

    # where does the main Graphviz program live?
    abs_dot = shutil.which("dot")
    assert abs_dot is not None, "dot not in $PATH"
    abs_dot = Path(abs_dot)

    # discard the result we found if it does not live in the same directory
    if abs_cmd.parent != abs_dot.parent:
        return None

    return abs_cmd
