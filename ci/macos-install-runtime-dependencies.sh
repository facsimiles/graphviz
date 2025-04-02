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
brew install cmake || brew upgrade cmake
brew install pango || brew upgrade pango
brew install qt5 || brew upgrade qt5
brew install devil || brew upgrade devil
brew install gd || brew upgrade gd
brew install gtk+ || brew upgrade gtk+
brew install gts || brew upgrade gts

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
  https://github.com/macports/macports-base/releases/download/v2.10.5/MacPorts-2.10.5-14-Sonoma.pkg
sudo installer -package MacPorts-2.10.5-14-Sonoma.pkg -target /
export PATH=/opt/local/bin:${PATH}

# lib/mingle dependency
sudo port install libANN

# create a Python virtual environment
sudo mkdir /opt/virtualenv
sudo chown $(whoami) /opt/virtualenv
python3 -m venv /opt/virtualenv

# install Python dependencies within the virtual environment
env PATH="/opt/virtualenv/bin:$PATH" python3 -m pip install --requirement requirements.txt
