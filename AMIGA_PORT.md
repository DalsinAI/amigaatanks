# Atomic Tanks for GFOS

Atomic Tanks 6.7 (GPL-2 or later) by Tom Hudson, Jesse Smith, Sven Eden
and others (see `Data/credits.txt`), from https://github.com/talregev/atanks
(Jesse Smith's tree) at commit b9e3be3988a772bf7d5b6fa1d8811e98ca7f52ac,
built for AmigaOS 3.2 / GFOS on a 68040, 68060 or 68080. No FPU is needed.

Turn-based artillery for one to ten players, people or computer: aim,
pick the power, fire, and buy weapons, shields and other items between
rounds with what you earned. It runs in 800 x 600 with 16-bit colour.

Status: see the pull request (stove build and lab).

## What is in the drawer

- `AtomicTanks`: the game.
- `Data/`: upstream's pictures, sounds, texts and `unicode.dat` (the
  font), as they are.
- `Libs/allegro4.library` and `Libs/acgame.library`: Allegro 4 for GFOS
  (gfopenallegro) on ACGame. The game opens them from its own `Libs/`.
- `User/`: your settings (`atanks-config.txt`) and saved games (`*.sav`),
  made as you play; put `.bmp` pictures in it for extra backgrounds and
  `.wav` files in `User/music/` for music. Updates and
  `GFOpenUpTool REMOVE` leave it alone.
- `Controls`, `ReadMe-AtomicTanks` (the game's read-me), `Licences.txt`,
  `COPYING.txt`.

## Playing

Run Install-AtomicTanks (or copy the AtomicTanks drawer where you want
it), then double-click AtomicTanks, or start it from a Shell from
anywhere: the game makes its own drawer its current directory while it
runs. The keys are in `Controls`.

## How the port is made

- The code is upstream's, built against gfopenallegro's Allegro 4 with
  68040 soft float (`amiga/Makefile.amigaos`). The game asks for 16-bit
  colour; gfopenallegro draws it in truecolour and shows it on ACGame's
  8-bit screen through a colour cube, so colours are coarser than on a
  truecolour screen until ACGame has one.
- `src/debug.h` knows AmigaOS (`ATANKS_IS_AMIGA`, from `__amigaos__`);
  the data is read from `PROGDIR:Data` (`DATA_DIR`), and the settings
  and saved games go in `User/` instead of `$HOME/.atanks`
  (`src/environment.cpp`, `src/files.cpp`, `src/main.h`). The changes
  are each marked `ATANKS_IS_AMIGA`.
- libnix has no `<dirent.h>`: `amiga/compat/dirent.h` and
  `amiga/dirent_amiga.cpp` give the game opendir, readdir, closedir,
  scandir, alphasort and strcasestr on Allegro's al_findfirst, which
  allegro4.library does with dos.library.
- The game's worker threads (AI, physics, land) are std::thread on
  libnix's pthreads. Network play is not built (upstream leaves it off
  unless `NETWORK` is defined).
- Sound is silent for now: gfopenallegro plays samples once ACGame has
  audio.

## Building

    make -f amiga/Makefile.amigaos PREFIX=<stove>/prefix A4=<gfopenallegro>/allegro4 A4OUT=... ACGOUT=...

or, in Cradle, stove_build with the os32-gcc16-open6 stove and target
package (`package/build-amiga.sh`), which also builds allegro4.library
and acgame.library at the commits `package/package.env` pins, and packs
AtomicTanks-VERSION-68040.lha and the GFOpenUp part.

    tests/host/run.sh <gfopenallegro>/allegro4

runs the host tests on gfopenallegro's allegro4 core and its fake
acgame.library: the dirent calls (missing and empty drawers, a file is
not a drawer, every entry once, scandir with the game's `.sav` filter in
alphasort order, strcasestr on `.bmp`/`.BMP`); all 188 bitmaps, the 25
sounds and `unicode.dat` load at 16-bit; and the whole game, built with
the GFOS defines, links on the host and a smoke run in its own data
makes `User/` and shows its first screen.

Designed, tested and delivered by The AmiCradle Team.
