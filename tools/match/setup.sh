#!/usr/bin/env bash
# setup.sh - bring up the byte-match toolchain under tools/match/vendor/ (gitignored).
#
#   tools/match/setup.sh [--force] [--cc1 PATH] [--wsl] [--keep-build]
#
# What it installs (exact pins in tools/match/README.md and vendor/VERSIONS.txt):
#   vendor/maspsx/          mkst/maspsx at $MASPSX_COMMIT (ASPSX 2.56 emulation), plus our
#                           fixes from tools/match/patches/maspsx-*.patch (applied after
#                           the checkout, never committed into the clone)
#   vendor/cc1[.exe|.sh]    GCC 2.7.2 "psx" cc1, the retail Armored Core compiler:
#       Linux/macOS : decompals/old-gcc release $OLDGCC_RELEASE prebuilt tarball
#       Windows     : built here from the stock GNU gcc-2.7.2 source plus the
#                     decompals/old-gcc patch set (commit $OLDGCC_COMMIT) with the
#                     MinGW gcc on PATH - old-gcc ships no Windows binary
#       --wsl       : (Windows) use the Linux tarball through wsl.exe instead
#       --cc1 PATH  : use a cc1 you already have (e.g. PsyQ 4.0 CC1PSX.EXE), nothing fetched
#   vendor/toolchain.json   what match_fn.py reads to find the pieces
#
# Needs on PATH: git, curl, tar, python (3.10+), a mipsel GNU as/objdump
# (mipsel-none-elf-* or mipsel-linux-gnu-*), and on Windows: gcc (MinGW-w64), make, patch.
set -euo pipefail

HERE="$(cd "$(dirname "$0")" && pwd)"
VENDOR="$HERE/vendor"

OLDGCC_RELEASE=0.17
OLDGCC_TARBALL_LINUX=gcc-2.7.2-psx.tar.gz
OLDGCC_TARBALL_LINUX_SHA256=500a459b3485e885a8d302cac23c2a4632f3900e03a09153f6190699fd723571
OLDGCC_TARBALL_MACOS=gcc-2.7.2-psx-macos.tar.gz
OLDGCC_COMMIT=cf70ec58bce58128732062bc9930045c82904cbb      # patches/ used for the Windows build
GCC_SRC_URL=https://ftp.gnu.org/old-gnu/gcc/gcc-2.7.2.tar.gz
GCC_SRC_MIRROR=https://ftp.gwdg.de/pub/gnu/ftp/old-gnu/gcc/gcc-2.7.2.tar.gz
GCC_SRC_SHA256=7cd8bce5c3aeec59a72ecc2d3d5123864a817b14cdbd0680b1a969c3bccc5da5
MASPSX_REPO=https://github.com/mkst/maspsx.git
MASPSX_COMMIT=670417cf5bd5d9dae4a962b18f400363670da5c5

FORCE=false; CC1_GIVEN=""; USE_WSL=false; KEEP_BUILD=false
while [ $# -gt 0 ]; do
  case "$1" in
    --force) FORCE=true ;;
    --cc1) CC1_GIVEN="$2"; shift ;;
    --wsl) USE_WSL=true ;;
    --keep-build) KEEP_BUILD=true ;;
    -h|--help) sed -n 2,20p "$0"; exit 0 ;;
    *) echo "unknown option $1" >&2; exit 3 ;;
  esac
  shift
done

say() { printf '==> %s\n' "$*"; }
die() { printf 'setup.sh: %s\n' "$*" >&2; exit 2; }
need() { command -v "$1" >/dev/null 2>&1 || die "need $1 on PATH"; }

case "$(uname -s)" in
  MINGW*|MSYS*|CYGWIN*) OS=windows ;;
  Darwin) OS=macos ;;
  *) OS=linux ;;
esac
PY=python3; command -v python3 >/dev/null 2>&1 || PY=python
need git; need curl; need tar; need "$PY"

sha256_of() {
  if command -v sha256sum >/dev/null 2>&1; then sha256sum "$1" | cut -d' ' -f1
  else shasum -a 256 "$1" | cut -d' ' -f1; fi
}
fetch() {  # fetch URL DEST [MIRROR]
  local url="$1" dest="$2" mirror="${3:-}"
  if [ -s "$dest" ]; then return 0; fi
  say "download $url"
  curl -fL --retry 2 --max-time 600 -o "$dest.part" "$url" \
    || { [ -n "$mirror" ] && say "retry via $mirror" && curl -fL --retry 3 --max-time 600 -o "$dest.part" "$mirror"; } \
    || die "download failed: $url"
  mv "$dest.part" "$dest"
}
verify() {  # verify FILE SHA256
  local got; got="$(sha256_of "$1")"
  [ "$got" = "$2" ] || die "sha256 mismatch for $1: got $got want $2"
}

mkdir -p "$VENDOR/dl"
VERSIONS="$VENDOR/VERSIONS.txt"
: > "$VERSIONS"

# ---------------------------------------------------------------- 1. maspsx
if [ ! -f "$VENDOR/maspsx/maspsx.py" ] || $FORCE; then
  rm -rf "$VENDOR/maspsx"
  say "clone maspsx @ $MASPSX_COMMIT"
  # LF checkout whatever the user's core.autocrlf says, so our patches apply byte-exactly.
  git clone -q -c core.autocrlf=false "$MASPSX_REPO" "$VENDOR/maspsx"
  git -C "$VENDOR/maspsx" -c advice.detachedHead=false checkout -q "$MASPSX_COMMIT"
fi
MASPSX_HEAD="$(git -C "$VENDOR/maspsx" rev-parse HEAD)"
[ "$MASPSX_HEAD" = "$MASPSX_COMMIT" ] || die "maspsx is at $MASPSX_HEAD, expected $MASPSX_COMMIT (use --force)"
echo "maspsx $MASPSX_COMMIT ($MASPSX_REPO)" >> "$VERSIONS"
# Our maspsx fixes (tools/match/patches/maspsx-*.patch), idempotent: a patch that is already
# in the tree is left alone, one that neither applies nor reverts means a stale clone.
for p in "$HERE"/patches/maspsx-*.patch; do
  [ -f "$p" ] || continue
  if git -C "$VENDOR/maspsx" apply --reverse --check "$p" 2>/dev/null; then
    :
  elif git -C "$VENDOR/maspsx" apply --check "$p" 2>/dev/null; then
    say "patch maspsx: $(basename "$p")"
    git -C "$VENDOR/maspsx" apply "$p"
  else
    die "maspsx patch $(basename "$p") does not apply to vendor/maspsx (CRLF or edited clone? rerun with --force)"
  fi
  echo "maspsx patch $(basename "$p") sha256=$(sha256_of "$p")" >> "$VERSIONS"
done

# ---------------------------------------------------------------- 2. cc1
CC1=""
if [ -n "$CC1_GIVEN" ]; then
  [ -f "$CC1_GIVEN" ] || die "--cc1 $CC1_GIVEN does not exist"
  CC1="$(cd "$(dirname "$CC1_GIVEN")" && pwd)/$(basename "$CC1_GIVEN")"
  echo "cc1 user-supplied $CC1" >> "$VERSIONS"

elif [ "$OS" = linux ] || [ "$OS" = macos ]; then
  tb="$OLDGCC_TARBALL_LINUX"; [ "$OS" = macos ] && tb="$OLDGCC_TARBALL_MACOS"
  dest="$VENDOR/gcc-2.7.2-psx"
  if [ ! -x "$dest/cc1" ] || $FORCE; then
    fetch "https://github.com/decompals/old-gcc/releases/download/$OLDGCC_RELEASE/$tb" "$VENDOR/dl/$tb"
    [ "$OS" = linux ] && verify "$VENDOR/dl/$tb" "$OLDGCC_TARBALL_LINUX_SHA256"
    rm -rf "$dest"; mkdir -p "$dest"
    tar xzf "$VENDOR/dl/$tb" -C "$dest"
    chmod +x "$dest"/*
  fi
  CC1="$dest/cc1"
  echo "cc1 decompals/old-gcc release $OLDGCC_RELEASE $tb sha256=$(sha256_of "$VENDOR/dl/$tb")" >> "$VERSIONS"

elif $USE_WSL; then
  # Windows with WSL: run the official Linux binary through wsl.exe (cwd is mapped to /mnt/...,
  # match_fn.py passes relative file names, so no path translation is needed).
  command -v wsl.exe >/dev/null 2>&1 || die "--wsl but wsl.exe not found"
  tb="$OLDGCC_TARBALL_LINUX"; dest="$VENDOR/gcc-2.7.2-psx"
  if [ ! -f "$dest/cc1" ] || $FORCE; then
    fetch "https://github.com/decompals/old-gcc/releases/download/$OLDGCC_RELEASE/$tb" "$VENDOR/dl/$tb"
    verify "$VENDOR/dl/$tb" "$OLDGCC_TARBALL_LINUX_SHA256"
    rm -rf "$dest"; mkdir -p "$dest"; tar xzf "$VENDOR/dl/$tb" -C "$dest"
  fi
  wsl_cc1="$(wsl.exe wslpath -a "$(cygpath -w "$dest/cc1")" | tr -d '\r')"
  cat > "$VENDOR/cc1.sh" <<EOF
#!/usr/bin/env bash
# generated by setup.sh --wsl: GCC 2.7.2 psx cc1 (Linux binary) via WSL
exec wsl.exe -e "$wsl_cc1" "\$@"
EOF
  CC1="$VENDOR/cc1.sh"
  echo "cc1 decompals/old-gcc release $OLDGCC_RELEASE $tb via WSL sha256=$(sha256_of "$VENDOR/dl/$tb")" >> "$VERSIONS"

else
  # Windows, native: build cc1.exe from source with MinGW-w64 gcc.
  if [ ! -f "$VENDOR/cc1.exe" ] || $FORCE; then
    need gcc; need patch
    MAKE=make; command -v make >/dev/null 2>&1 || MAKE=mingw32-make; need "$MAKE"
    fetch "$GCC_SRC_URL" "$VENDOR/dl/gcc-2.7.2.tar.gz" "$GCC_SRC_MIRROR"
    verify "$VENDOR/dl/gcc-2.7.2.tar.gz" "$GCC_SRC_SHA256"

    BUILD="$(mktemp -d "${TMPDIR:-/tmp}/ac1_cc1_build.XXXXXX")"
    say "build dir $BUILD"
    say "fetch decompals/old-gcc patches @ $OLDGCC_COMMIT"
    git init -q "$BUILD/old-gcc"
    git -C "$BUILD/old-gcc" fetch -q --depth 1 https://github.com/decompals/old-gcc.git "$OLDGCC_COMMIT"
    git -C "$BUILD/old-gcc" -c advice.detachedHead=false checkout -q FETCH_HEAD
    tar xzf "$VENDOR/dl/gcc-2.7.2.tar.gz" -C "$BUILD"
    SRC="$BUILD/gcc-2.7.2"

    # Same recipe as old-gcc's gcc-2.7.2-psx.Dockerfile ...
    say "patch (old-gcc gcc-2.7.2-psx recipe)"
    ( cd "$SRC"
      sed -i -- 's/include <varargs.h>/include <stdarg.h>/g' *.c
      P="$BUILD/old-gcc/patches"
      patch -s -u -p1 obstack.h -i "$P/obstack-2.7.2.h.patch"
      patch -s -u -p1 configure -i "$P/configure.patch"
      patch -s -u -p1 config.sub -i "$P/config.sub.patch"
      patch -s -u -p1 config/mips/mips.h -i "$P/mipsel-2.7.patch"
      patch -s -u -p1 < "$P/psx-2.5.7.patch" )

    # ... plus the bits a MinGW host lacks: <sys/times.h>, <sys/resource.h>,
    # <sys/param.h>, <sys/file.h>, <sys/wait.h> and the BSD string helpers.
    STUBS="$BUILD/stubs"; mkdir -p "$STUBS/sys"
    cat > "$STUBS/sys/times.h" <<'EOF'
#ifndef _STUB_SYS_TIMES_H
#define _STUB_SYS_TIMES_H
#include <time.h>
struct tms { long tms_utime, tms_stime, tms_cutime, tms_cstime; };
static long times(struct tms *t) { long c = (long)clock(); if (t) { t->tms_utime = c; t->tms_stime = t->tms_cutime = t->tms_cstime = 0; } return c; }
#endif
EOF
    cat > "$STUBS/sys/resource.h" <<'EOF'
#ifndef _STUB_SYS_RESOURCE_H
#define _STUB_SYS_RESOURCE_H
#include <sys/time.h>
struct rusage { struct timeval ru_utime, ru_stime; };
#define RUSAGE_SELF 0
static int getrusage(int who, struct rusage *r) { (void)who; if (r) { r->ru_utime.tv_sec = r->ru_utime.tv_usec = 0; r->ru_stime.tv_sec = r->ru_stime.tv_usec = 0; } return 0; }
#endif
EOF
    : > "$STUBS/sys/param.h"; : > "$STUBS/sys/file.h"; : > "$STUBS/sys/wait.h"
    cat > "$STUBS/compat.c" <<'EOF'
/* BSD string helpers and sbrk that gcc 2.7.2 expects from a Unix libc. */
#include <string.h>
#include <stdlib.h>
char *index(const char *s, int c) { return strchr(s, c); }
char *rindex(const char *s, int c) { return strrchr(s, c); }
int bcmp(const void *a, const void *b, size_t n) { return memcmp(a, b, n); }
void bcopy(const void *s, void *d, size_t n) { memmove(d, s, n); }
void bzero(void *s, size_t n) { memset(s, 0, n); }
void *sbrk(long inc) { (void)inc; return 0; }
EOF
    gcc -c -O1 -w -o "$STUBS/compat.o" "$STUBS/compat.c"
    STUBS_W="$(cygpath -m "$STUBS")"

    say "configure (mips-sony-psx, little endian, gnu as)"
    ( cd "$SRC" && ./configure --target=mips-sony-psx --prefix=/opt/cross --with-endian-little \
        --with-gnu-as --disable-gprof --disable-gdb --disable-werror \
        --host=i386-pc-linux --build=i386-pc-linux > "$BUILD/configure.log" 2>&1 ) \
      || die "configure failed, see $BUILD/configure.log"
    say "make cc1 (a few minutes)"
    ( cd "$SRC" && "$MAKE" cc1 CC=gcc CFLAGS="-std=gnu89 -w -O1 -I$STUBS_W" \
        LDFLAGS="$STUBS_W/compat.o" > "$BUILD/make.log" 2>&1 ) \
      || die "make cc1 failed, see $BUILD/make.log"
    [ -f "$SRC/cc1.exe" ] || die "no cc1.exe after make, see $BUILD/make.log"

    # old-gcc's own post-build checks
    ( cd "$SRC" && ./cc1.exe -quiet -O2 -G0 "$BUILD/old-gcc/tests/little_endian.c" -o le.s \
        && grep -qE 'lbu\s\$2,0\(\$4\)' le.s ) || die "cc1 self-test: not little-endian"
    ( cd "$SRC" && ./cc1.exe -quiet -O2 -G0 "$BUILD/old-gcc/tests/host_wide_int.c" -o hwi.s \
        && grep -qE '# 0x80000000$' hwi.s ) || die "cc1 self-test: HOST_WIDE_INT is not 32-bit"
    ( cd "$SRC" && { ./cc1.exe -quiet -help </dev/null 2>&1 || true; } | grep -q -- -msoft-float ) || die "cc1 self-test: no -msoft-float"

    cp "$SRC/cc1.exe" "$VENDOR/cc1.exe"
    if $KEEP_BUILD; then say "build tree kept in $BUILD"; else rm -rf "$BUILD"; fi
  fi
  CC1="$VENDOR/cc1.exe"
  echo "cc1 built here: GNU gcc-2.7.2.tar.gz sha256=$GCC_SRC_SHA256 + decompals/old-gcc patches @ $OLDGCC_COMMIT (gcc-2.7.2-psx recipe), host $(gcc -dumpmachine 2>/dev/null) gcc $(gcc -dumpversion 2>/dev/null)" >> "$VERSIONS"
fi

# ---------------------------------------------------------------- 3. binutils
AS=""
for p in "${MIPS_PREFIX:-}" mipsel-none-elf- mipsel-linux-gnu- mips-linux-gnu-; do
  [ -n "$p" ] || continue
  if command -v "${p}as" >/dev/null 2>&1; then AS="$(command -v "${p}as")"; break; fi
done
[ -n "$AS" ] || die "no mipsel GNU as on PATH (PCSX-Redux mipsel-none-elf toolchain, or apt binutils-mipsel-linux-gnu)"
echo "as $("$AS" --version | head -1) ($AS)" >> "$VERSIONS"
CPP="$(command -v cpp || command -v gcc || true)"
[ -n "$CPP" ] || die "no host cpp/gcc for preprocessing"
echo "cpp $("$CPP" --version | head -1) ($CPP)" >> "$VERSIONS"

# ---------------------------------------------------------------- 4. record + smoke test
"$PY" - "$VENDOR" "$CC1" "$AS" "$CPP" <<'EOF'
import json, os, sys
vendor, cc1, as_, cpp = sys.argv[1:5]
def rel(p):
    try:
        r = os.path.relpath(p, vendor)
        return r if not r.startswith("..") else p
    except ValueError:
        return p
tc = {"cc1": rel(cc1), "maspsx": "maspsx/maspsx.py", "as": as_, "cpp": cpp,
      "aspsx_version": "2.56", "note": "written by tools/match/setup.sh; see VERSIONS.txt"}
with open(os.path.join(vendor, "toolchain.json"), "w") as fh:
    json.dump(tc, fh, indent=2); fh.write("\n")
EOF

say "smoke test: compile one function through cpp -> cc1 -> maspsx -> as"
T="$(mktemp -d)"
cat > "$T/t.c" <<'EOF'
int add3(int a, int b) { return a + b + 3; }
EOF
( cd "$T"
  "$CPP" -E -P -undef -nostdinc t.c -o t.i
  case "$CC1" in *.sh) bash "$CC1" t.i -O2 -mips1 -mcpu=3000 -w -msoft-float -mgas -G0 -quiet -o t.s </dev/null ;;
                   *)  "$CC1" t.i -O2 -mips1 -mcpu=3000 -w -msoft-float -mgas -G0 -quiet -o t.s </dev/null ;; esac
  "$PY" "$VENDOR/maspsx/maspsx.py" --aspsx-version=2.56 t.s > t.m.s </dev/null   # maspsx blocks on an open stdin
  "$AS" -EL -march=r3000 -mtune=r3000 -no-pad-sections -G0 -o t.o t.m.s
  "$PY" - <<'PYEOF'
import struct
d = open("t.o", "rb").read()
assert d[:4] == b"\x7fELF", "not an ELF"
print("    ok:", len(d), "byte ELF object")
PYEOF
) || die "smoke test failed"
rm -rf "$T"

say "done. $(wc -l < "$VERSIONS") component(s) recorded in $VERSIONS:"
sed 's/^/    /' "$VERSIONS"
