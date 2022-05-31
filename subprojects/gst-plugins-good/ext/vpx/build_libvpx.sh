#!/usr/bin/env bash
set -euo pipefail

platform=$1
builddir=${2:-}    # unset in windows invocation
installdir=${3:-}  # unset in windows invocation

arch_run() { "$@"; }
unix=1

case $platform in
     win)
         unix=0
         export PATH=/usr/local/bin:/mingw/bin:/bin:$PATH
         mkdir -p /tmp
         builddir=/c/vpx
         ;;
     mac)
         arch_run() { arch -x86_64 "$@"; }
         ;;
     linux)
         ;;
esac

rm -rf "$builddir"
mkdir -p "$builddir"
cd "$builddir"

v=1.11.0
curl -sfL "https://github.com/webmproject/libvpx/archive/refs/tags/v${v}.tar.gz" -o "libvpx-v$v.tar.gz"

tar xf "libvpx-v$v.tar.gz"

mkdir build && cd build

_conf=( )

# on windows we need to set the target explicitly:
[[ $platform = 'win' ]] && _conf+=( "--target=x86_64-win64-vs16" )

# on mac/linux we set the prefix, on windows we use the default:
(( unix )) && _conf+=( --prefix="$installdir" )

_conf+=(
    --enable-pic --as=nasm --disable-unit-tests
    --size-limit=16384x16384 --enable-postproc --enable-multi-res-encoding
    --enable-temporal-denoising --enable-vp9-temporal-denoising
    --enable-vp9-postproc --disable-tools --disable-examples --disable-docs
)

arch_run ../"libvpx-$v"/configure "${_conf[@]}"
arch_run make

if (( unix )); then
    arch_run make install
else
    powershell.exe -Command '&{Import-Module "C:\BuildTools\Common7\Tools\Microsoft.VisualStudio.DevShell.dll"; Enter-VsDevShell 62da6021 -SkipAutomaticLocation -DevCmdArguments "-arch=x64 -host_arch=x64"} dir'
    C:/BuildTools/MSBuild/Current/Bin/MSBuild.exe vpx.sln -m -t:Build -p:Configuration="Release" -p:Platform="x64"
fi

if [[ $platform = 'linux' ]]; then
    sed -i 's/^\(Libs:.*\)$/\1 -lpthread/' "$installdir"/lib/pkgconfig/vpx.pc
fi
