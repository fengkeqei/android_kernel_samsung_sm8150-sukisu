#!/usr/bin/env bash
# Build SM-G9730/beyond1qlte with the tracked enforcing SukiSU baseline.
set -euo pipefail

ROOT=$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)
TOOLS=/home/dev/Documents/Workspace/GalaxyS10/KernelSource
MAKE=$TOOLS/tools/make-4.3/bin/make
CLANG_BIN=$TOOLS/llvm-arm-toolchain-ship-10.0/llvm-arm-toolchain-ship/10.0/bin
NDK_BIN=$TOOLS/android-ndk-r16b/toolchains/aarch64-linux-android-4.9/prebuilt/linux-x86_64/bin
CLANG_LIBS=$TOOLS/clang-libs
OUT=/home/dev/Documents/Workspace/GalaxyS10/out-beyond1qlte-sukisu-gh
LOG=$ROOT/build-gh.log
DEFCONFIG=afaneh_beyond1qlte_defconfig

if [[ ${1:-} == --clean ]]; then
  rm -rf "$OUT" "$LOG"
elif [[ $# -ne 0 ]]; then
  printf 'Usage: %s [--clean]\n' "$0" >&2
  exit 2
fi

for tool in "$MAKE" "$CLANG_BIN/clang" "$NDK_BIN/aarch64-linux-android-gcc"; do
  [[ -x $tool ]] || { printf 'Required tool not executable: %s\n' "$tool" >&2; exit 1; }
done
command -v python2 >/dev/null || { printf 'python2 is required\n' >&2; exit 1; }

export ARCH=arm64
export CROSS_COMPILE="$NDK_BIN/aarch64-linux-android-"
export REAL_CC="$CLANG_BIN/clang"
export CFP_CC="$CLANG_BIN/clang"
export CLANG_TRIPLE=aarch64-linux-gnu-
export PYTHON=python2
export LD_LIBRARY_PATH="$CLANG_LIBS${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}"
export PATH="$CLANG_BIN:$NDK_BIN:$PATH"
export KBUILD_BUILD_USER=dpi
export KBUILD_BUILD_HOST=21DN2A11
export KBUILD_BUILD_VERSION=2

make_args=(
  -C "$ROOT" -j"$(nproc)" "O=$OUT" "ARCH=$ARCH"
  "CROSS_COMPILE=$CROSS_COMPILE" "REAL_CC=$REAL_CC" "CFP_CC=$CFP_CC"
  "CLANG_TRIPLE=$CLANG_TRIPLE" "PYTHON=$PYTHON"
  "KBUILD_BUILD_USER=$KBUILD_BUILD_USER" "KBUILD_BUILD_HOST=$KBUILD_BUILD_HOST"
  "KBUILD_BUILD_VERSION=$KBUILD_BUILD_VERSION" "GIT_BIN=false" "CURL_BIN=false"
  "KSU_VERSION_OVERRIDE=40900" "KSU_VERSION_TAG_OVERRIDE=4.2.0"
  "KSU_VERSION_FULL_OVERRIDE=v4.2.0-85eb4a95"
  "KCFLAGS=-I$ROOT/techpack/audio/include/uapi -I$ROOT/techpack/audio/include -include $ROOT/techpack/audio/config/sm8150_beyondq.h"
)

{
  printf '%s\n' "[+] Configuring $DEFCONFIG"
  "$MAKE" "${make_args[@]}" "$DEFCONFIG"
  grep -qx 'CONFIG_KSU=y' "$OUT/.config"
  grep -qx '# CONFIG_KEXEC is not set' "$OUT/.config"
  grep -qx '# CONFIG_CRASH_DUMP is not set' "$OUT/.config"
  grep -qx 'CONFIG_SECURITY_SELINUX_ALWAYS_ENFORCE=y' "$OUT/.config"
  grep -qx '# CONFIG_SECURITY_SELINUX_ALWAYS_PERMISSIVE is not set' "$OUT/.config"
  printf '%s\n' '[+] Building Image-dtb'
  "$MAKE" "${make_args[@]}" Image-dtb
  image=$OUT/arch/arm64/boot/Image-dtb
  [[ -s $image ]]
  head -c 16 "$image" | grep -qx 'UNCOMPRESSED_IMG'
  rg -q ' (ksu_|kernelsu_)' "$OUT/System.map"
  printf '%s\n' "[+] Image-dtb: $image ($(stat -c %s "$image") bytes)"
} 2>&1 | tee "$LOG"
