/* Atomic Tanks for GFOS: host test of the game's data through gfopenallegro.
 * At 16-bit colour, as the game runs on GFOS, every .bmp in the folders the
 * game reads (src/environment.cpp: button, misc, missile, stock, tank,
 * tankgun, title) loads with a sane size, the 640 x 480 title picture comes
 * out 640 x 480, every sound/NN.wav (25 of them) loads as a sample, and
 * unicode.dat (the game's font) loads.
 *   test_data <atanks source tree>
 * Copyright (c) 2026 AmiCradle Team. GPL-2 or later, as Atomic Tanks. */
#include <allegro.h>
#include <stdio.h>
#include <string.h>

static int fails;
#define CHECK(c, ...) do { if (!(c)) { fails++; printf("FAIL: " __VA_ARGS__); printf("\n"); } } while (0)

static const char *const dirs[] = {
    "button", "misc", "missile", "stock", "tank", "tankgun", "title"
};

int main(int argc, char **argv)
{
    const char *root = argc > 1 ? argv[1] : ".";
    char path[1024];
    int d, i, total = 0, sounds = 0;

    CHECK(allegro_init() == 0, "allegro_init");
    set_color_depth(16);
    CHECK(set_gfx_mode(GFX_AUTODETECT_WINDOWED, 800, 600, 0, 0) == 0, "set_gfx_mode 800x600x16");

    for (d = 0; d < (int)(sizeof(dirs) / sizeof(dirs[0])); d++) {
        int n = 0;
        for (i = 0; i < 100; i++) {
            BITMAP *b;
            snprintf(path, sizeof(path), "%s/%s/%d.bmp", root, dirs[d], i);
            if (!exists(path))
                break;
            b = load_bitmap(path, NULL);
            CHECK(b != NULL, "load_bitmap %s", path);
            if (!b)
                continue;
            CHECK(b->w > 0 && b->h > 0 && b->w <= 1600 && b->h <= 1200,
                  "%s size %dx%d", path, b->w, b->h);
            CHECK(bitmap_color_depth(b) == 16, "%s depth %d", path, bitmap_color_depth(b));
            if (!strcmp(dirs[d], "title") && i == 0)
                CHECK(b->w == 640 && b->h == 480, "title/0.bmp %dx%d", b->w, b->h);
            destroy_bitmap(b);
            n++;
        }
        CHECK(n > 0, "no bitmaps in %s", dirs[d]);
        printf("%s: %d bitmaps\n", dirs[d], n);
        total += n;
    }
    for (i = 0; i < 100; i++) {
        SAMPLE *s;
        /* the game tries sound/00.wav to SND_COUNT; the numbers have gaps */
        snprintf(path, sizeof(path), "%s/sound/%02d.wav", root, i);
        if (!exists(path))
            continue;
        s = load_sample(path);
        CHECK(s != NULL && s->len > 0, "load_sample %s", path);
        if (s)
            destroy_sample(s);
        sounds++;
    }
    CHECK(sounds == 25, "%d sounds, not 25", sounds);
    snprintf(path, sizeof(path), "%s/unicode.dat", root);
    {
        DATAFILE *df = load_datafile(path);
        CHECK(df != NULL, "load_datafile %s", path);
        if (df)
            unload_datafile(df);
    }
    printf("data: %d bitmaps, %d sounds, unicode.dat\n", total, sounds);
    if (!fails)
        printf("data: all checks passed\n");
    return fails != 0;
}
