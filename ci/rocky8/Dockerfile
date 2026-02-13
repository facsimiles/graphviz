# Digest from https://hub.docker.com/r/rockylinux/rockylinux/tags?name=8-ubi-init
FROM rockylinux/rockylinux:8-ubi-init@sha256:f301d0f66a1f9fb79eb2ee7fecc04f94e485149f7e3c232937792d39accc1715

COPY requirements.txt ./requirements.txt

# Development tools
RUN dnf -y --setopt install_weak_deps=false groupinstall 'Development Tools' \
# for dnf config-manager plugin
&& dnf install -y --setopt install_weak_deps=false python3-dnf-plugins-core \
&& dnf install -y \
# Install epel-release rpm (Needed for libglade2-devel. Must be a separate step)
https://dl.fedoraproject.org/pub/epel/epel-release-latest-8.noarch.rpm \
&& dnf config-manager --set-enabled powertools \
# Centos build utilities
&& dnf -y --setopt install_weak_deps=false install \
libtool-ltdl-devel \
libedit-devel \
cmake \
# security-hardened alternatives of packages
rocky-release-security \
# Option glut
freeglut-devel \
# Option gts
# Option swig
swig \
# Command smyrna
libglade2-devel \
# Command gvedit
qt5-devel \
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
poppler-qt5 \
# Plugin library rsvg
librsvg2-devel \
# Plugin library webp
# Language extension gv_go
# Language extension gv_guile
guile-devel \
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
# create a Python virtual environment
python3.11 \
&& python3.11 -m venv /opt/virtualenv \
# install Python dependencies within the virtual environment
&& env PATH="/opt/virtualenv/bin:$PATH" python3 -m pip install --requirement requirements.txt \
# Remove possible old version of Graphviz so that we can detect if
# individual Graphviz tools are missing in the current release
&& dnf remove -y graphviz \
# Clean up
&& dnf clean all && rm -rf /var/cache/dnf
