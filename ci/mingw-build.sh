#!/usr/bin/env bash

set -eux

pacman -S --noconfirm --needed bison
pacman -S --noconfirm --needed ${MINGW_PACKAGE_PREFIX}-cairo
pacman -S --noconfirm --needed ${MINGW_PACKAGE_PREFIX}-cmake
pacman -S --noconfirm --needed ${MINGW_PACKAGE_PREFIX}-diffutils
pacman -S --noconfirm --needed ${MINGW_PACKAGE_PREFIX}-expat
pacman -S --noconfirm --needed flex
pacman -S --noconfirm --needed ${MINGW_PACKAGE_PREFIX}-gcc
pacman -S --noconfirm --needed ${MINGW_PACKAGE_PREFIX}-libgd
pacman -S --noconfirm --needed ${MINGW_PACKAGE_PREFIX}-libtool
pacman -S --noconfirm --needed ${MINGW_PACKAGE_PREFIX}-pango
pacman -S --noconfirm --needed ${MINGW_PACKAGE_PREFIX}-pkgconf
pacman -S --noconfirm --needed ${MINGW_PACKAGE_PREFIX}-python
pacman -S --noconfirm --needed ${MINGW_PACKAGE_PREFIX}-ninja
pacman -S --noconfirm --needed ${MINGW_PACKAGE_PREFIX}-nsis
pacman -S --noconfirm --needed ${MINGW_PACKAGE_PREFIX}-tcl
pacman -S --noconfirm --needed ${MINGW_PACKAGE_PREFIX}-zlib
pacman -S --noconfirm --needed ${MINGW_PACKAGE_PREFIX}-gts

export PATH=$PATH:/c/Git/cmd

# use the libs installed with pacman instead of those in
# https://gitlab.com/graphviz/graphviz-windows-dependencies
export CMAKE_OPTIONS=-Duse_win_pre_inst_libs=OFF

ci/build.sh
