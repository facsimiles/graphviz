FROM ubuntu:24.10

ARG DEBIAN_FRONTEND=noninteractive

COPY requirements.txt ./requirements.txt
COPY doc/infosrc/requirements.txt ./doc-infosrc-requirements.txt

RUN apt-get update -y \
&& apt-get install --no-install-recommends -y \
# Development tools
build-essential \
clang-format \
cmake \
git \
pkg-config \
autoconf \
bison \
libtool \
dh-python \
flex \
doxygen \
# Debian build utilities
d-shlibs \
debhelper \
fakeroot \
# Option glut
freeglut3-dev \
# Option gts
libgts-dev \
# Option swig
swig \
# Command smyra
libgtkglext1-dev \
libglade2-dev \
# Command gvedit
libqt5gui5 \
qt5-qmake \
qtbase5-dev \
# for libmingle
libann-dev \
# Plugin library devil
libdevil-dev \
# Plugin library gd
libgd-dev \
# Plugin library gdk
libgtk-3-dev \
# Documentation
ghostscript \
# Plugin library ghostscipt
libgs-dev \
# Plugin library lasi
liblasi-dev \
# Plugin library poppler
libpoppler-dev \
libpoppler-glib-dev \
# Plugin library rsvg
librsvg2-dev \
# Plugin library webp
libwebp-dev \
# Language extension gv_ruby
ruby \
# Language extension gv_go
golang-go \
# Language extension gv_guile
guile-3.0 \
guile-3.0-dev \
# Language extension gv_lua
lua5.3 \
liblua5.3-dev \
# Language extension gv_perl
libperl-dev \
# Language extension gv_php
php-dev \
libsodium-dev \
libargon2-dev \
# Language extension gv_python3
libpython3-dev \
# Language extension gv_ruby
ruby-dev \
# Language extension gv_tcl
tcl-dev \
# Test utilities
python3-venv \
gcovr \
lcov \
shellcheck \
# Libraries used in test
catch2 \
libboost-dev \
libfmt-dev \
libsvgpp-dev \
# create a Python virtual environment
&& python3 -m venv /opt/virtualenv \
# install Python dependencies within the virtual environment
&& env PATH="/opt/virtualenv/bin:$PATH" python3 -m pip install --requirement requirements.txt \
&& env PATH="/opt/virtualenv/bin:$PATH" python3 -m pip install --requirement doc-infosrc-requirements.txt \
# Clean up
&& rm -rf /var/lib/apt/lists/*
