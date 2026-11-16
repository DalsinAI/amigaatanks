/* Atomic Tanks for GFOS: opendir/readdir/closedir/scandir/alphasort and
 * strcasestr for libnix (see compat/dirent.h), on Allegro's al_findfirst.
 * Copyright (c) 2026 AmiCradle Team. GPL-2 or later, as Atomic Tanks. */
#include <allegro.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include "dirent.h"

struct atanks_dir {
    struct al_ffblk ff;
    int open;   /* al_findfirst found something and ff holds it */
    int fresh;  /* ff holds an entry readdir has not handed out yet */
    struct dirent ent;
};

extern "C" DIR *atanks_opendir(const char *path)
{
    char pattern[512];
    DIR *dir;
    int attrib = 0;
    /* file_exists() with FA_DIREC also takes files: aret says which */
    if (!path || !*path || !file_exists(path, FA_DIREC, &attrib) || !(attrib & FA_DIREC))
        return NULL;
    dir = (DIR *)calloc(1, sizeof(DIR));
    if (!dir)
        return NULL;
    size_t n = strlen(path);
    const char *sep = (path[n - 1] == '/' || path[n - 1] == ':') ? "" : "/";
    snprintf(pattern, sizeof(pattern), "%s%s*", path, sep);
    if (al_findfirst(pattern, &dir->ff, FA_ALL) == 0)
        dir->open = dir->fresh = 1;
    return dir;
}

extern "C" struct dirent *atanks_readdir(DIR *dir)
{
    if (!dir || !dir->open)
        return NULL;
    if (!dir->fresh && al_findnext(&dir->ff) != 0)
        return NULL;
    dir->fresh = 0;
    strncpy(dir->ent.d_name, dir->ff.name, sizeof(dir->ent.d_name) - 1);
    dir->ent.d_name[sizeof(dir->ent.d_name) - 1] = 0;
    return &dir->ent;
}

extern "C" int atanks_closedir(DIR *dir)
{
    if (!dir)
        return -1;
    if (dir->open)
        al_findclose(&dir->ff);
    free(dir);
    return 0;
}

extern "C" int atanks_alphasort(const struct dirent **a, const struct dirent **b)
{
    return strcmp((*a)->d_name, (*b)->d_name);
}

extern "C" int atanks_scandir(const char *path, struct dirent ***list,
                              int (*filter)(const struct dirent *),
                              int (*compar)(const struct dirent **, const struct dirent **))
{
    DIR *dir = atanks_opendir(path);
    struct dirent **v = NULL, *e;
    int n = 0, cap = 0;
    if (!dir)
        return -1;
    while ((e = atanks_readdir(dir)) != NULL) {
        if (filter && !filter(e))
            continue;
        if (n == cap) {
            cap = cap ? cap * 2 : 16;
            struct dirent **nv = (struct dirent **)realloc(v, cap * sizeof(*v));
            if (!nv)
                break;
            v = nv;
        }
        v[n] = (struct dirent *)malloc(sizeof(struct dirent));
        if (!v[n])
            break;
        memcpy(v[n++], e, sizeof(struct dirent));
    }
    atanks_closedir(dir);
    if (compar && n > 1)
        qsort(v, n, sizeof(*v), (int (*)(const void *, const void *))compar);
    *list = v;
    return n;
}

extern "C" char *atanks_strcasestr(const char *haystack, const char *needle)
{
    size_t n = strlen(needle);
    for (; *haystack; haystack++) {
        size_t i = 0;
        while (i < n && haystack[i]
               && tolower((unsigned char)haystack[i]) == tolower((unsigned char)needle[i]))
            i++;
        if (i == n)
            return (char *)haystack;
    }
    return n ? NULL : (char *)haystack;
}
