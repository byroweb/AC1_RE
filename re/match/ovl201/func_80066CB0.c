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
int open(char *, int);
int read(int, void *, int);
int close(int);
char *strcat(char *, char *);
void *func_80050FC4(s32);
void func_80051078(void *);

typedef struct { char s[36]; } SaveTitle;
typedef struct SaveSlot {       /* 0x2C, one per save file on the card */
    SaveTitle title;            /* +0x00 */
    u16 unk24;                  /* +0x24 */
    s32 unk28;                  /* +0x28 */
} SaveSlot;
typedef struct SaveHead {       /* first 0x280 bytes of a save file */
    u8 pad[0x204];
    SaveTitle title;            /* +0x204 */
    u16 unk228;
    s32 unk22C;
} SaveHead;

extern char D_8004B6B8[19] __attribute__((aligned(4)));  /* "bu00:BASCUS-94182*" */
extern char D_8004B6CC[19] __attribute__((aligned(4)));  /* "bu10:BASCUS-94182*" */
extern char D_8004B6F0[6] __attribute__((aligned(4)));   /* "bu00:" */
extern char D_8004B6F8[6] __attribute__((aligned(4)));   /* "bu10:" */
extern SaveSlot D_801F5098[16];
extern s16 D_801A257C;

void func_80066CB0(u8 port)
{
    struct DIRENTRY de;
    char pattern[19];
    char path[40];
    s32 n = 0;
    SaveHead *buf;
    int fd;

    buf = func_80050FC4(0xE80);
    if (port == 0)
        __builtin_memcpy(pattern, D_8004B6B8, 19);
    else
        __builtin_memcpy(pattern, D_8004B6CC, 19);
    if (firstfile(pattern, &de) == &de) {
        do {
            if (de.name[12] != 'Z') {
                if (port == 0)
                    __builtin_memcpy(path, D_8004B6F0, 6);
                else
                    __builtin_memcpy(path, D_8004B6F8, 6);
                strcat(path, de.name);
                fd = open(path, 0x10001);
                read(fd, buf, 0x280);
                close(fd);
                D_801F5098[n].title = buf->title;
                D_801F5098[n].unk24 = buf->unk228;
                D_801F5098[n].unk28 = buf->unk22C;
                n++;
                if (n == 16)
                    break;
            }
        } while (nextfile(&de) == &de);
        D_801A257C = n;
    } else {
        D_801A257C = 0;
    }
    func_80051078(buf);
}
