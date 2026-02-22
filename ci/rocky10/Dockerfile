# Digest from https://hub.docker.com/r/rockylinux/rockylinux/tags?name=10-ubi-init
FROM rockylinux/rockylinux:10-ubi-init@sha256:9d6caecb3c82a043f94ec464c8228314cfc7ee4ddf53a5226a23939f5605ab1c

COPY requirements.txt ./requirements.txt

# Development tools
RUN dnf -y --setopt install_weak_deps=false groupinstall 'Development Tools' \
# for dnf config-manager plugin
&& dnf install -y --setopt install_weak_deps=false python3-dnf-plugins-core \
&& dnf config-manager --enable crb \
# Centos build utilities
&& dnf -y --setopt install_weak_deps=false install \
libtool-ltdl-devel \
libedit-devel \
cmake \
# Option glut
freeglut-devel \
# Option gts
# Option swig
swig \
# Command gvedit
qt6-qtbase-devel \
# Plugin library devil
# Plugin library gd
gd-devel \
# Documentation
ghostscript \
# Plugin library
ghostscript-devel \
# Plugin library gdk
gtk3-devel \
# Plugin library lasi
# Plugin library pangocairo
# Plugin library poppler
poppler-glib-devel \
poppler-qt6 \
# Plugin library rsvg
librsvg2-devel \
# Plugin library webp
# Language extension gv_java
java-devel \
# Language extension gv_lua
lua-devel \
# Language extension gv_perl
perl \
# Language extension gv_php
php-devel \
openssl-devel \
# Language extension gv_python3
python3-devel \
# Language extension gv_ruby
ruby-devel \
# Language extension gv_tcl
tcl-devel \
libX11-devel \
libXrender-devel \
tk-devel \
expat-devel \
gmp-devel \
dejavu-sans-fonts \
dejavu-sans-mono-fonts \
dejavu-serif-fonts \
# create a Python virtual environment
&& python3 -m venv /opt/virtualenv \
# install Python dependencies within the virtual environment
&& env PATH="/opt/virtualenv/bin:$PATH" python3 -m pip install --requirement requirements.txt \
# Remove possible old version of Graphviz so that we can detect if
# individual Graphviz tools are missing in the current release
&& dnf remove -y graphviz \
# Clean up
&& dnf clean all && rm -rf /var/cache/dnf
