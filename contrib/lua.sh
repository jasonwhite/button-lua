#!/bin/bash -e
# Downloads and builds Lua.

VERSION=5.3.2
SHA256SUM=c740c7bb23a936944e1cc63b7c3c5351a8976d7867c5252c8854f7b2af9da68f

if [[ -n "$1" ]]; then
    OS=$1
fi

# cd to this script directory
cd "$( dirname "${BASH_SOURCE[0]}" )"

if [[ ! -f lua-$VERSION.tar.gz ]]; then
    echo "Downloading Lua $VERSION ..."
    curl -R -O http://www.lua.org/ftp/lua-$VERSION.tar.gz
    echo "$SHA256SUM  lua-$VERSION.tar.gz" | sha256sum --check
fi

if [[ ! -d lua-$VERSION ]]; then
    tar zxf lua-$VERSION.tar.gz
fi

INSTALL_TOP=$(pwd)/lua

cd lua-$VERSION

# Build in POSIX mode. Note that this does not provide the ability to load
# dynamic libraries (which is what we want). Loading dynamic libraries is not
# safe and can lead to non-determinism when generating a build description.
make posix test

make install INSTALL_TOP=$INSTALL_TOP