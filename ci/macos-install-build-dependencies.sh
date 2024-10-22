#!/usr/bin/env bash

set -e
set -o pipefail
set -u
set -x

# Homebrew/Macports install steps may pull in newer versions of Flex or Bison,
# but we still end up with the XCode-provided headers in the compiler’s include
# path, so log what versions we have for debugging purposes
echo -n 'XCode-provided version of Flex is: '
flex --version
echo -n 'XCode-provided version of Bison is: '
bison --version | head -1

brew tap --repair
brew update
brew install autogen || brew upgrade autogen
brew install automake || brew upgrade automake
brew install cmake || brew upgrade cmake
brew install bison || brew upgrade bison
brew install devil || brew upgrade devil
brew install gd || brew upgrade gd
brew install gtk+ || brew upgrade gtk+
brew install gts || brew upgrade gts
brew install swig || brew upgrade swig

# quoting Homebrew:
#
#   bison is keg-only, which means it was not symlinked into /opt/homebrew,
#   because macOS already provides this software and installing another version in
#   parallel can cause all kinds of trouble.
#
#   If you need to have bison first in your PATH, run:
#     echo 'export PATH="/opt/homebrew/opt/bison/bin:$PATH"' >> ~/.zshrc
#
#   For compilers to find bison you may need to set:
#     export LDFLAGS="-L/opt/homebrew/opt/bison/lib"
export PATH="/opt/homebrew/opt/bison/bin:$PATH"

brew install pango || brew upgrade pango
brew install qt5 || brew upgrade qt5

# quoting Homebrew:
#
#   qt@5 is keg-only, which means it was not symlinked into /opt/homebrew,
#   because this is an alternate version of another formula.
#
#   If you need to have qt@5 first in your PATH, run:
#     echo 'export PATH="/opt/homebrew/opt/qt@5/bin:$PATH"' >> ~/.zshrc
#
#   For compilers to find qt@5 you may need to set:
#     export LDFLAGS="-L/opt/homebrew/opt/qt@5/lib"
#     export CPPFLAGS="-I/opt/homebrew/opt/qt@5/include"
#
#   For pkg-config to find qt@5 you may need to set:
#     export PKG_CONFIG_PATH="/opt/homebrew/opt/qt@5/lib/pkgconfig"
export PATH="/opt/homebrew/opt/qt@5/bin:$PATH"
export LDFLAGS="-L/opt/homebrew/opt/qt@5/lib"
export CPPFLAGS="-I/opt/homebrew/opt/qt@5/include"
export PKG_CONFIG_PATH="/opt/homebrew/opt/qt@5/lib/pkgconfig"

brew install librsvg || brew upgrade librsvg
brew install libxaw || brew upgrade libxaw

# install MacPorts for libANN
curl --retry 3 --location --no-progress-meter -O \
  https://github.com/macports/macports-base/releases/download/v2.9.3/MacPorts-2.9.3-13-Ventura.pkg
sudo installer -package MacPorts-2.9.3-13-Ventura.pkg -target /
export PATH=/opt/local/bin:${PATH}

# lib/mingle dependency
sudo port install libANN
