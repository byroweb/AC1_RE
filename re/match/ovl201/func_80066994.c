#include "ac1/ac1.h"

struct DIRENTRY {
    char name[20];
    s32 attr;
    s32 size;
    struct DIRENTRY *next;
    s32 head;
    char system[4];
};
struct DIRENTRY *firstfile(char *, struct DIRENTRY *);
struct DIRENTRY *nextfile(struct DIRENTRY *);
char *strcpy(char *, char *);

typedef struct CardFile {       /* 0x44 */
    s32 blocks;                 /* +0x00 size in 8 KB blocks */
    char name[64];              /* +0x04 */
} CardFile;
typedef struct CardDir {        /* 0x88C per memory card port, at 0x801EF6D0 */
    s32 used_blocks;            /* +0x000 sum of files[].blocks */
    s32 n_files;                /* +0x004 */
    s32 n_saves;                /* +0x008 */
    CardFile files[16];         /* +0x00C every file on the card */
    CardFile saves[16];         /* +0x44C this game's files */
} CardDir;

extern char D_8004B6B8[19] __attribute__((aligned(4)));  /* "bu00:BASCUS-94182*" */
extern char D_8004B6CC[19] __attribute__((aligned(4)));  /* "bu10:BASCUS-94182*" */
extern char D_8004B6E0[7] __attribute__((aligned(4)));   /* "bu00:*" */
extern char D_8004B6E8[7] __attribute__((aligned(4)));   /* "bu10:*" */
extern CardDir D_801EF6D0[2];

void func_80066994(u8 port)
{
    struct DIRENTRY de;
    char pattern[19];
    s32 n = 0;
    s32 i, total;

    if (port == 0)
        __builtin_memcpy(pattern, D_8004B6B8, 19);
    else
        __builtin_memcpy(pattern, D_8004B6CC, 19);
    if (firstfile(pattern, &de) == &de) {
        do {
            if (de.name[12] != 'Z') {
                strcpy(D_801EF6D0[port].saves[n].name, de.name);
                D_801EF6D0[port].saves[n].blocks = de.size / 8192;
                n++;
            }
        } while (nextfile(&de) == &de);
    }
    D_801EF6D0[port].n_saves = n;
    n = 0;
    if (port == 0)
        __builtin_memcpy(pattern, D_8004B6E0, 7);
    else
        __builtin_memcpy(pattern, D_8004B6E8, 7);
    if (firstfile(pattern, &de) == &de) {
        do {
            strcpy(D_801EF6D0[port].files[n].name, de.name);
            D_801EF6D0[port].files[n].blocks = de.size / 8192;
            n++;
        } while (nextfile(&de) == &de);
    }
    D_801EF6D0[port].n_files = n;
    total = 0;
    for (i = 0; i < n; i++)
        total += D_801EF6D0[port].files[i].blocks;
    D_801EF6D0[port].used_blocks = total;
}
