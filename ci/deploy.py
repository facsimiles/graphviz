#!/usr/bin/env python3

"""
steps for deploying a new release (see ../.gitlab-ci.yml)

This is based on Gitlab’s generic package example,
https://gitlab.com/gitlab-org/release-cli/-/tree/master/docs/examples/release-assets-as-generic-package
"""

import hashlib
import json
import logging
import os
import re
import shutil
import stat
import subprocess
import sys
from pathlib import Path

# logging output stream, setup in main()
log = None


def upload(dry_run: bool, version: str, path: Path) -> str:
    """
    upload a file to the Graphviz generic package with the given version
    """

    name = path.name

    # Gitlab upload file_name field only allows letters, numbers, dot, dash, and
    # underscore
    safe = re.sub(r"[^a-zA-Z0-9.\-]", "_", name)
    log.info(f"escaped name {name} to {safe}")

    target = (
        f'{os.environ["CI_API_V4_URL"]}/projects/'
        f'{os.environ["CI_PROJECT_ID"]}/packages/generic/graphviz-releases/'
        f"{version}/{safe}"
    )

    if dry_run:
        log.info("skipping upload due to 'dry_run' flag")
        return target

    log.info(f"uploading {path} to {target}")
    # calling Curl is not the cleanest way to achieve this, but Curl takes care of
    # encodings, headers and part splitting for us
    proc = subprocess.run(
        [
            "curl",
            "--silent",  # no progress bar
            "--include",  # include HTTP response headers in output
            "--verbose",  # more connection details
            "--retry",
            "3",  # retry on transient errors
            "--header",
            f'JOB-TOKEN: {os.environ["CI_JOB_TOKEN"]}',
            "--upload-file",
            path,
            target,
        ],
        stdout=subprocess.PIPE,
        stderr=subprocess.STDOUT,
        check=False,
        text=True,
    )
    log.info("Curl response:")
    for i, line in enumerate(proc.stdout.split("\n"), 1):
        log.info(f" {i:3}: {line}")
    proc.check_returncode()

    resp = proc.stdout.split("\n")[-1]
    if json.loads(resp)["message"] != "201 Created":
        raise RuntimeError(f"upload failed: {resp}")

    return target


def checksum(path: Path) -> Path:
    """generate checksum for the given file"""

    assert path.exists()

    log.info(f"SHA256 summing {path}")
    check = Path(f"{path}.sha256")
    data = path.read_bytes()
    check.write_text(f"{hashlib.sha256(data).hexdigest()}  {path}\n", encoding="utf-8")
    return check


def is_macos_artifact(name: str) -> bool:
    """is this a deployment artifact for macOS?"""
    return name.startswith("Darwin_")


def is_windows_artifact(name: str) -> bool:
    """is this a deployment artifact for Windows?"""
    return name.startswith("windows_")


def get_format(path: Path) -> str:
    """a human readable description of the format of this file"""
    if path.suffix[1:].lower() == "exe":
        return "EXE installer"
    if path.suffix[1:].lower() == "zip":
        return "ZIP archive"
    if ".tar." in str(path):
        return "".join(path.suffixes[-2:])[1:].lower()
    return path.suffix[1:].lower()


def main() -> int:
    """entry point"""

    # setup logging to print to stderr
    global log
    ch = logging.StreamHandler()
    log = logging.getLogger("deploy.py")
    log.addHandler(ch)
    log.setLevel(logging.INFO)

    if os.environ.get("CI") is None:
        log.error("CI environment variable unset; refusing to run")
        return -1

    # echo some useful things for debugging
    log.info(f"os.uname(): {os.uname()}")
    if Path("/etc/os-release").exists():
        with open("/etc/os-release", "rt", encoding="utf-8") as f:
            log.info("/etc/os-release:")
            for i, line in enumerate(f, 1):
                log.info(f" {i}: {line[:-1]}")

    # bail out early if we do not have release-cli to avoid uploading assets that
    # become orphaned when we fail to create the release
    if not shutil.which("release-cli"):
        log.error("release-cli not found")
        return -1

    # retrieve version name left by prior CI tasks
    log.info("deriving Graphviz version")
    root = Path(__file__).resolve().parents[1]
    version = subprocess.check_output(
        [sys.executable, "gen_version.py"], cwd=root, text=True
    ).strip()
    log.info(f"Graphviz version == {version}")

    # the generic package version has to be \d+.\d+.\d+ but it does not need to
    # correspond to the release version (which may not conform to this pattern if
    # this is a dev release)
    if re.match(r"\d+\.\d+\.\d+$", version) is None:
        # generate a compliant version
        package_version = f'0.0.{int(os.environ["CI_COMMIT_SHA"], 16)}'
    else:
        # we can use a version corresponding to the release version
        package_version = version
    log.info(f"using generic package version {package_version}")

    # we only create Gitlab releases for stable version numbers
    skip_release = re.match(r"\d+\.\d+\.\d+$", version) is None

    # list of assets we have uploaded
    assets: list[str] = []

    # 0-pad major version to 2 digits, to sort versions properly
    vparts = version.split(".")
    assert len(vparts) > 0, "Malformed version string"
    vparts[0] = f"{int(vparts[0]):02d}"

    # data for the website’s download page
    webdata = {
        "name": f"graphviz-{version}",
        "version": ".".join(vparts),
        "sources": [],
        "windows": [],
    }

    for tarball in (
        Path(f"graphviz-{version}.tar.gz"),
        Path(f"graphviz-{version}.tar.xz"),
    ):
        if not tarball.exists():
            log.error(f"source {tarball} not found")
            return -1

        # accrue the source tarball and accompanying checksum
        url = upload(skip_release, package_version, tarball)
        assets.append(url)
        webentry = {"format": get_format(tarball), "url": url}
        check = checksum(tarball)
        url = upload(skip_release, package_version, check)
        assets.append(url)
        webentry[check.suffix[1:]] = url

        webdata["sources"].append(webentry)

    # exported artifacts to be included in the release
    ARTIFACTS = {
        # CI job → [artifacts…]
        "fedora41-build": [f"fedora_41_graphviz-{version}-rpms.tar.xz"],
        "fedora42-build": [f"fedora_42_graphviz-{version}-rpms.tar.xz"],
        "fedora43-build": [f"fedora_43_graphviz-{version}-rpms.tar.xz"],
        "rocky8-build": [f"rocky_8.9_graphviz-{version}-rpms.tar.xz"],
        "rocky9-build": [f"rocky_9.7_graphviz-{version}-rpms.tar.xz"],
        "rocky10-build": [f"rocky_10.1_graphviz-{version}-rpms.tar.xz"],
        "fedora41-cmake-build": [f"fedora_41_graphviz-{version}-cmake.rpm"],
        "fedora42-cmake-build": [f"fedora_42_graphviz-{version}-cmake.rpm"],
        "fedora43-cmake-build": [f"fedora_43_graphviz-{version}-cmake.rpm"],
        "rocky8-cmake-build": [f"rocky_8.9_graphviz-{version}-cmake.rpm"],
        "rocky9-cmake-build": [f"rocky_9.7_graphviz-{version}-cmake.rpm"],
        "rocky10-cmake-build": [f"rocky_10.1_graphviz-{version}-cmake.rpm"],
        "ubuntu-22.04-build": [f"ubuntu_22.04_graphviz-{version}-debs.tar.xz"],
        "ubuntu-24.04-build": [f"ubuntu_24.04_graphviz-{version}-debs.tar.xz"],
        "ubuntu-25.10-build": [f"ubuntu_25.10_graphviz-{version}-debs.tar.xz"],
        "ubuntu-22.04-cmake-build": [f"ubuntu_22.04_graphviz-{version}-cmake.deb"],
        "ubuntu-24.04-cmake-build": [f"ubuntu_24.04_graphviz-{version}-cmake.deb"],
        "ubuntu-25.10-cmake-build": [f"ubuntu_25.10_graphviz-{version}-cmake.deb"],
        "macos-autotools-build": [f"Darwin_23.6.0_graphviz-{version}-arm64.pkg"],
        "macos-cmake-build": [f"Darwin_23.6.0_Graphviz-{version}-Darwin.zip"],
        "windows-cmake-Win32-release-build": [
            f"windows_10_cmake_Release_graphviz-install-{version}-win32.exe",
            f"windows_10_cmake_Release_Graphviz-{version}-win32.zip",
        ],
        "windows-cmake-Win32-debug-build": [
            f"windows_10_cmake_Debug_graphviz-install-{version}-win32.exe",
            f"windows_10_cmake_Debug_Graphviz-{version}-win32.zip",
        ],
        "windows-cmake-x64-release-build": [
            f"windows_10_cmake_Release_graphviz-install-{version}-win64.exe",
            f"windows_10_cmake_Release_Graphviz-{version}-win64.zip",
        ],
        "windows-cmake-x64-debug-build": [
            f"windows_10_cmake_Debug_graphviz-install-{version}-win64.exe",
            f"windows_10_cmake_Debug_Graphviz-{version}-win64.zip",
        ],
        "windows-mingw64-cmake-build": [
            f"msys2_*_Graphviz-{version}-win64.exe",
            f"msys2_*_Graphviz-{version}-win64.zip",
        ],
        "windows-cygwin-build": [f"CYGWIN_*_graphviz-{version}-x86_64.tar.xz"],
        "windows-cygwin-cmake-build": [f"CYGWIN_*_Graphviz-{version}-CYGWIN-1.tar.bz2"],
    }

    for job, artifacts in ARTIFACTS.items():
        for artifact in artifacts:
            paths = list((Path("Packages") / job).glob(artifact))
            assert len(paths) != 0, f"missing artifact: {artifact}"
            assert len(paths) == 1, f"multiple matches for {artifact}: {paths}"
            path = paths[0]

            # get existing permissions
            mode = path.stat().st_mode

            # fixup permissions, o-rwx g-wx
            path.chmod(mode & ~stat.S_IRWXO & ~stat.S_IWGRP & ~stat.S_IXGRP)

            url = upload(skip_release, package_version, path)
            assets.append(url)

            webentry = {
                "format": get_format(path),
                "url": url,
            }
            if "win32" in artifact:
                webentry["bits"] = 32
            elif "win64" in artifact:
                webentry["bits"] = 64

            # if this is a standalone Windows or macOS package, also provide checksum(s)
            if is_macos_artifact(artifact) or is_windows_artifact(artifact):
                c = checksum(path)
                url = upload(skip_release, package_version, c)
                assets.append(url)
                webentry[c.suffix[1:]] = url

            # only expose a subset of the Windows artifacts
            if "windows_10_cmake_Release_" in artifact:
                webdata["windows"].append(webentry)

    # various release pages truncate the viewable artifacts to 100 or even 50
    if len(assets) > 50:
        log.error(
            f"upload has {len(assets)} assets, which will result in some of "
            f"them being unviewable in web page lists: {assets}"
        )
        return -1

    assert len(webdata["windows"]) > 0, "no Windows artifacts found"

    if skip_release:
        log.warning(
            f"skipping release creation because {version} is not "
            "of the form \\d+.\\d+.\\d+"
        )
        return 0

    # construct a command to create the release itself
    cmd = [
        "release-cli",
        "create",
        "--name",
        version,
        "--tag-name",
        version,
        "--description",
        "See the [CHANGELOG](https://gitlab.com/"
        "graphviz/graphviz/-/blob/main/CHANGELOG.md).",
    ]
    for a in assets:
        name = os.path.basename(a)
        url = a
        cmd += ["--assets-link", json.dumps({"name": name, "url": url})]

    # create the release
    subprocess.check_call(cmd)

    # output JSON data for the website
    log.info(f"dumping {webdata} to graphviz-{version}.json")
    with open(f"graphviz-{version}.json", "wt", encoding="utf-8") as f:
        json.dump(webdata, f, indent=2)

    return 0


if __name__ == "__main__":
    sys.exit(main())
