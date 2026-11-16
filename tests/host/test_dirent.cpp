/* Atomic Tanks for GFOS: host test of amiga/dirent_amiga.cpp, the
 * opendir/readdir/closedir/scandir/alphasort/strcasestr the game uses on
 * its User drawer, run against gfopenallegro's al_findfirst.
 * The game's own Filter_File and Find_Saved_Games/Find_Bitmaps (src/files.cpp)
 * are used the way upstream does on Linux.
 *   test_dirent <scratch directory>
 * Copyright (c) 2026 AmiCradle Team. GPL-2 or later, as Atomic Tanks. */
#include <allegro.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include "dirent.h"

static int fails;
#define CHECK(c, ...) do { if (!(c)) { fails++; printf("FAIL: " __VA_ARGS__); printf("\n"); } } while (0)

static void touch(const char *dir, const char *name)
{
    char p[1024];
    snprintf(p, sizeof(p), "%s/%s", dir, name);
    FILE *f = fopen(p, "w");
    if (f) { fputs("x", f); fclose(f); }
}

static int is_sav(const struct dirent *e) { return strstr(e->d_name, ".sav") != NULL; }

int main(int argc, char **argv)
{
    const char *t = argc > 1 ? argv[1] : ".";
    char user[1024], empty[1024], sub[1024];
    struct dirent **list = NULL, *e;
    DIR *d;
    int n, i, seen = 0, bmps = 0;

    CHECK(allegro_init() == 0, "allegro_init");
    snprintf(user, sizeof(user), "%s/User", t);
    snprintf(empty, sizeof(empty), "%s/Empty", t);
    snprintf(sub, sizeof(sub), "%s/User/music", t);
    mkdir(user, 0700); mkdir(empty, 0700); mkdir(sub, 0700);
    touch(user, "zeta.sav"); touch(user, "alpha.sav"); touch(user, "Mid.sav");
    touch(user, "atanks-config.txt"); touch(user, "sky.BMP"); touch(user, "land.bmp");

    /* a missing drawer: NULL, as opendir */
    CHECK(opendir("no/such/drawer") == NULL, "opendir of a missing drawer");
    CHECK(opendir("") == NULL, "opendir of \"\"");
    /* a file is not a drawer */
    {
        char f[1024];
        snprintf(f, sizeof(f), "%s/zeta.sav", user);
        CHECK(opendir(f) == NULL, "opendir of a file");
    }
    /* an empty drawer opens and reads nothing (Create_Music_Folder relies on it) */
    d = opendir(empty);
    CHECK(d != NULL, "opendir of an empty drawer");
    if (d) { CHECK(readdir(d) == NULL, "readdir of an empty drawer"); closedir(d); }

    /* every entry once, the music drawer too, with a trailing / or not */
    for (int pass = 0; pass < 2; pass++) {
        char p[1030];
        snprintf(p, sizeof(p), pass ? "%s/" : "%s", user);
        d = opendir(p);
        CHECK(d != NULL, "opendir %s", p);
        seen = 0;
        int music = 0;
        while (d && (e = readdir(d)) != NULL) {
            seen++;
            if (!strcmp(e->d_name, "music")) music = 1;
            CHECK(strchr(e->d_name, '/') == NULL, "readdir name with a path: %s", e->d_name);
        }
        if (d) { CHECK(readdir(d) == NULL, "readdir after the end"); closedir(d); }
        CHECK(seen == 7, "pass %d: readdir saw %d entries, not 7", pass, seen);
        CHECK(music, "pass %d: the music drawer is not listed", pass);
    }

    /* scandir with the game's .sav filter, sorted as alphasort */
    n = scandir(user, &list, is_sav, alphasort);
    CHECK(n == 3, "scandir found %d .sav, not 3", n);
    if (n == 3) {
        CHECK(!strcmp(list[0]->d_name, "Mid.sav") && !strcmp(list[1]->d_name, "alpha.sav")
              && !strcmp(list[2]->d_name, "zeta.sav"),
              "scandir order %s %s %s", list[0]->d_name, list[1]->d_name, list[2]->d_name);
    }
    for (i = 0; i < n; i++) free(list[i]);
    free(list);
    CHECK(scandir("no/such/drawer", &list, is_sav, alphasort) == -1, "scandir of a missing drawer");
    list = NULL;
    CHECK(scandir(empty, &list, NULL, alphasort) == 0, "scandir of an empty drawer");
    free(list);

    /* strcasestr, as Find_Bitmaps uses it on .bmp / .BMP */
    d = opendir(user);
    while (d && (e = readdir(d)) != NULL)
        if (strcasestr(e->d_name, ".bmp")) bmps++;
    if (d) closedir(d);
    CHECK(bmps == 2, "strcasestr found %d .bmp, not 2", bmps);
    CHECK(strcasestr("abc", "") != NULL, "strcasestr empty needle");
    CHECK(strcasestr("abc", "abcd") == NULL, "strcasestr longer needle");
    CHECK(strcasestr("xxAbC", "abc") && !strcmp(strcasestr("xxAbC", "abc"), "AbC"), "strcasestr position");

    if (!fails)
        printf("dirent shim: all checks passed\n");
    return fails != 0;
}
