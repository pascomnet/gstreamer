#!/usr/bin/env bash
set -euo pipefail

builddir="$1"
installdir="$2"

rm -rf "$builddir"
mkdir -p "$builddir"
cd "$builddir"

v=1.11.0
curl -sfL "https://github.com/webmproject/libvpx/archive/refs/tags/v${v}.tar.gz" -o "libvpx-v$v.tar.gz"

tar xf "libvpx-v$v.tar.gz"

mkdir build && cd build

_conf=(
    --prefix="$installdir" --enable-pic --as=nasm --disable-unit-tests
    --size-limit=16384x16384 --enable-postproc --enable-multi-res-encoding
    --enable-temporal-denoising --enable-vp9-temporal-denoising
    --enable-vp9-postproc --disable-tools --disable-examples --disable-docs
)

../"libvpx-$v"/configure "${_conf[@]}"

make && make install

sed -i 's/^\(Libs:.*\)$/\1 -lpthread/' "$installdir"/lib/pkgconfig/vpx.pc
