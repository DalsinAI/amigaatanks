#!/bin/sh
# Atomic Tanks for GFOS: host tests.
#   tests/host/run.sh [GFOPENALLEGRO/allegro4]
# Through gfopenallegro's allegro4 core on its fake acgame.library:
#   - test_dirent: amiga/dirent_amiga.cpp, the <dirent.h> calls libnix lacks;
#   - test_data: every bitmap, sound and unicode.dat the game loads, at 16-bit;
#   - the whole game (src/ built as on GFOS, -D__amigaos__ with amiga/compat)
#     links on the host, so no Allegro call is missing, and a smoke run in
#     the game's own data shows its first screen (smoke_probe.c).
# Copyright (c) 2026 AmiCradle Team. GPL-2 or later, as Atomic Tanks.
set -eu
ROOT=$(CDPATH= cd -- "$(dirname -- "$0")/../.." && pwd)
A4=${1:-"$ROOT/../gfopenallegro/allegro4"}
CC=${HOSTCC:-cc}
CXX=${HOSTCXX:-c++}
T=$(mktemp -d)
trap 'rm -rf "$T"' EXIT HUP INT TERM
mkdir -p "$T/a4" "$T/game" "$T/scratch"
for f in "$A4"/src/a4_*.c "$A4"/tests/fake_acgame.c; do
    "$CC" -std=gnu99 -D_DEFAULT_SOURCE -w -I"$A4/include" -I"$A4/src" -I"$A4/tests" -c "$f" -o "$T/a4/$(basename "$f").o"
done
"$CXX" -std=gnu++17 -w -I"$ROOT/amiga/compat" -I"$A4/include" -c "$ROOT/amiga/dirent_amiga.cpp" -o "$T/dirent.o"
"$CXX" -std=gnu++17 -w -I"$ROOT/amiga/compat" -I"$A4/include" "$ROOT/tests/host/test_dirent.cpp" "$T/dirent.o" "$T"/a4/*.o -lm -o "$T/test_dirent"
"$T/test_dirent" "$T/scratch"
"$CC" -std=gnu99 -w -I"$A4/include" "$ROOT/tests/host/test_data.c" "$T"/a4/*.o -lm -o "$T/test_data"
"$T/test_data" "$ROOT"
# the whole-game link and smoke run take a while: package/build-amiga.sh sets
# HOST_LINK=0, since the stove build links the real thing right after
[ "${HOST_LINK:-1}" = 0 ] && { echo "game: host link skipped (HOST_LINK=0)"; exit 0; }
n=0
for f in "$ROOT"/src/*.cpp; do
    # the game's defines on GFOS, less the Linux ones the host compiler sets
    "$CXX" -std=gnu++17 -w -U__linux__ -U__linux -Ulinux -U__gnu_linux__ -D__amigaos__ \
        -DVERSION='"6.7"' -DDATA_DIR='"."' -I"$ROOT/amiga/compat" -I"$A4/include" -I"$ROOT/src" \
        -c "$f" -o "$T/game/$(basename "$f").o"
    n=$((n + 1))
done
"$CC" -std=gnu99 -w -I"$A4/include" -I"$A4/tests" -c "$ROOT/tests/host/smoke_probe.c" -o "$T/probe.o"
"$CXX" -o "$T/atanks" "$T"/game/*.o "$T/dirent.o" "$T/probe.o" "$T"/a4/*.o -lm -lpthread
echo "game: $n sources link on the host"
mkdir -p "$T/run"
for d in button misc missile sound stock tank tankgun title text unicode.dat credits.txt; do cp -R "$ROOT/$d" "$T/run/"; done
(cd "$T/run" && AT_SMOKE_SECS=${AT_SMOKE_SECS:-8} AT_SMOKE_PPM=${AT_SMOKE_PPM:-} "$T/atanks" | grep '^smoke:')
[ -d "$T/run/User" ] || { echo "FAIL: the game made no User drawer"; exit 1; }
echo "game: smoke run made User/ and showed its first screen"
