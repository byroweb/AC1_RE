#include "ac1/ac1.h"

int open(char *, int);
int read(int, void *, int);
int close(int);
char *strcat(char *, char *);
void LoadImage(RECT *, u32 *);
int DrawSync(int);
void *func_80050FC4(s32);
void func_80051078(void *);
s32 func_800684A0(u8 port);
u32 func_800B4370(u8 *p);

extern char D_8004DBEC[18] __attribute__((aligned(4)));  /* "bu00:BASCUS-94182" */
extern char D_8004DC00[18] __attribute__((aligned(4)));  /* "bu10:BASCUS-94182" */
extern RECT D_800BE204;         /* emblem CLUT rect */
extern RECT D_800BE20C;         /* emblem image rect */

/* Read the "Z" save file of a card port, check its sum and upload all 7 emblems. */
int func_800B4BD4(u8 port) {
    char name[32];
    char tail[16];
    u8 *buf;
    u8 tries;
    u8 bad;
    int fd;
    u8 *p;
    s16 cx;
    s16 cy;
    s16 bx;
    s16 y;
    s16 i;

    buf = func_80050FC4(0x3B00);
    if (func_800684A0(port) == 0) {
        tail[0] = 'Z';
        tail[1] = 0;
        if (port == 0)
            __builtin_memcpy(name, D_8004DBEC, 18);
        else
            __builtin_memcpy(name, D_8004DC00, 18);
        strcat(name, tail);
        tries = 0;
        bad = 0;
        do {
            while ((fd = open(name, 0x20001)) == -1) {
                if (++tries >= 3)
                    goto fail;
                close(-1);
            }
            tries = 0;
            while (read(fd, buf, 0x3B00) != 0x3B00 && ++tries < 3)
                ;
            close(fd);
            if (func_800B4370(buf + 0x204) == *(u32 *)(buf + 0x200))
                goto ok;
        } while (++bad < 3);
    }
fail:
    func_80051078(buf);
    return 0;
ok:
    cx = 0x380;
    cy = 0;
    bx = 0x380;
    y = 0x80;
    i = 0;
    D_800BE204.x = bx;
    D_800BE204.y = y;
    D_800BE20C.x = cx;
    D_800BE20C.y = cy;
    do {
        p = buf + (i * 0x820 + 0x204);
        LoadImage(&D_800BE204, (u32 *)p);
        LoadImage(&D_800BE20C, (u32 *)(p + 0x20));
        DrawSync(0);
        cx += 0x10;
        if (cx == 0x3C0) {
            cx = 0x380;
            cy += 0x40;
        }
        y++;
        i++;
        D_800BE204.x = bx;
        D_800BE204.y = y;
        D_800BE20C.x = cx;
        D_800BE20C.y = cy;
    } while (i < 7);
    func_80051078(buf);
    return 1;
}
