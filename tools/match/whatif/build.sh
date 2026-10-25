#!/usr/bin/env bash
# Build tools/match/vendor/cc1-whatif.exe: the pinned cc1 plus the what-if knobs.
#
#   tools/match/setup.sh --force --keep-build      # once: keeps the configured build dir
#   tools/match/whatif/build.sh BUILD_DIR          # BUILD_DIR = the ac1_cc1_build.* it printed
#
# BUILD_DIR holds gcc-2.7.2/ (patched, configured, built) and stubs/ from setup.sh. It is
# copied, gcc-whatif.patch is applied to the copy, and only the patched files rebuild.
# The pinned vendor/cc1.exe is never touched. `python tools/loop/whatif.py selftest`
# then checks that with no knobs set, the two compilers write identical assembly.
set -euo pipefail
HERE="$(cd "$(dirname "$0")" && pwd)"
VENDOR="$HERE/../vendor"
BUILD="${1:?usage: build.sh BUILD_DIR (from setup.sh --keep-build)}"
[ -d "$BUILD/gcc-2.7.2" ] && [ -d "$BUILD/stubs" ] || { echo "build.sh: $BUILD has no gcc-2.7.2/ and stubs/" >&2; exit 1; }
WORK="$(mktemp -d "${TMPDIR:-/tmp}/cc1_whatif.XXXXXX")"
cp -rp "$BUILD/gcc-2.7.2" "$WORK/src"
cp -rp "$BUILD/stubs" "$WORK/stubs"
chmod -R u+w "$WORK/src"
( cd "$WORK/src" && patch -s -u -p1 < "$HERE/gcc-whatif.patch" )
MAKE=make; command -v make >/dev/null 2>&1 || MAKE=mingw32-make
STUBS_W="$(cygpath -m "$WORK/stubs" 2>/dev/null || echo "$WORK/stubs")"
( cd "$WORK/src" && "$MAKE" cc1 CC=gcc CFLAGS="-std=gnu89 -w -O1 -I$STUBS_W" \
    LDFLAGS="$STUBS_W/compat.o -Wl,--stack,67108864 -Wl,--disable-high-entropy-va -Wl,--disable-dynamicbase" \
    > "$WORK/make.log" 2>&1 ) || { echo "build.sh: make failed, see $WORK/make.log" >&2; exit 1; }
cp "$WORK/src/cc1.exe" "$VENDOR/cc1-whatif.exe"
echo "built $VENDOR/cc1-whatif.exe (work dir $WORK)"
