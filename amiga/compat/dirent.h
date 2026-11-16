/* Atomic Tanks for GFOS: <dirent.h> for libnix.
 * The game reads its User drawer (saved games, .bmp backgrounds, music/)
 * with opendir/readdir/scandir; here they run on Allegro's al_findfirst,
 * which allegro4.library does with dos.library Lock/Examine/ExNext.
 * amiga/Makefile.amigaos puts amiga/compat first on the include path.
 * Copyright (c) 2026 AmiCradle Team. GPL-2 or later, as Atomic Tanks. */
#ifndef ATANKS_AMIGA_DIRENT_H
#define ATANKS_AMIGA_DIRENT_H

#ifdef __cplusplus
extern "C" {
#endif

struct dirent {
    char d_name[256];
};

typedef struct atanks_dir DIR;

#define opendir    atanks_opendir
#define readdir    atanks_readdir
#define closedir   atanks_closedir
#define scandir    atanks_scandir
#define alphasort  atanks_alphasort
#define strcasestr atanks_strcasestr

DIR *atanks_opendir(const char *path);
struct dirent *atanks_readdir(DIR *dir);
int atanks_closedir(DIR *dir);
int atanks_scandir(const char *path, struct dirent ***list,
                   int (*filter)(const struct dirent *),
                   int (*compar)(const struct dirent **, const struct dirent **));
int atanks_alphasort(const struct dirent **a, const struct dirent **b);
char *atanks_strcasestr(const char *haystack, const char *needle);

#ifdef __cplusplus
}
#endif

#endif /* ATANKS_AMIGA_DIRENT_H */
