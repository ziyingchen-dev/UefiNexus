#!/bin/bash

ARCH=$1
EDK_DIR=/workspaces/edk2

cd $EDK_DIR || exit 1

export WORKSPACE=$EDK_DIR
export EDK_TOOLS_PATH=$EDK_DIR/BaseTools
export PATH=$EDK_DIR/BaseTools/BinWrappers/PosixLike:$PATH
export PACKAGES_PATH=/workspaces/UefiNexus

PKG=UefiNexusPkg
TARGET=DEBUG
TOOLCHAIN=GCC

if [ "$ARCH" = "x64" ]; then
    EDK_ARCH=X64
elif [ "$ARCH" = "arm" ]; then
    EDK_ARCH=AARCH64
    export GCC_AARCH64_PREFIX=aarch64-linux-gnu-
else
    echo "Usage: ./build.sh <x64|arm>"
    exit 1
fi

build -p $PKG/$PKG.dsc -a $EDK_ARCH -t $TOOLCHAIN -b $TARGET