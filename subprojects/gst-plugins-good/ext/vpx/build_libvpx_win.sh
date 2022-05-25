#!/bin/bash -e

export PATH=/usr/local/bin:/mingw/bin:/bin:$PATH

mkdir -p /tmp

cd /c
mkdir -p vpx
cd vpx

if [ ! -e "libvpx" ]; then
    git clone --depth 1 --branch v1.11.0 https://chromium.googlesource.com/webm/libvpx

    rm -rf build
    mkdir build
    cd build

    ../libvpx/configure --target=x86_64-win64-vs16 --enable-pic --as=nasm --disable-unit-tests --size-limit=16384x16384 --enable-postproc --enable-multi-res-encoding --enable-temporal-denoising --enable-vp9-temporal-denoising --enable-vp9-postproc --disable-tools --disable-examples --disable-docs
    make

    powershell.exe -Command '&{Import-Module "C:\BuildTools\Common7\Tools\Microsoft.VisualStudio.DevShell.dll"; Enter-VsDevShell 62da6021 -SkipAutomaticLocation -DevCmdArguments "-arch=x64 -host_arch=x64"} dir'
    C:/BuildTools/MSBuild/Current/Bin/MSBuild.exe vpx.sln -m -t:Build -p:Configuration="Release" -p:Platform="x64"
fi