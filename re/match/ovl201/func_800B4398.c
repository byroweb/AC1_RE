/* MATCHED (wave4 a6). The wclose path calls close through a second name (func_80029B90, --defsym to
 * close's address) so jump2 does not cross-jump it into the identical write-fail block; retail keeps both. */
#include "ac1/ac1.h"

int open(char *, int);
int read(int, void *, int);
int write(int, void *, int);
int close(int);
int func_80029B90(int);
char *strcat(char *, char *);
char *strcpy(char *, char *);
void StoreImage(RECT *, u32 *);
int DrawSync(int);
void *func_80050FC4(s32);
void func_80051078(void *);
s32 func_800684A0(u8 port);
u32 func_800B4370(u8 *p);
void func_800B4054(u8 port);

typedef struct CardFile {       /* 0x44 */
    s32 blocks;
    char name[64];
} CardFile;
typedef struct CardDir {        /* 0x88C per memory card port, at 0x801EF6D0 */
    s32 used_blocks;
    s32 n_files;
    s32 n_saves;
    CardFile files[16];
    CardFile saves[16];
} CardDir;

extern char D_8004DBEC[18] __attribute__((aligned(4)));  /* "bu00:BASCUS-94182" */
extern char D_8004DC00[18] __attribute__((aligned(4)));  /* "bu10:BASCUS-94182" */
extern RECT D_800BE104;         /* emblem image rect */
extern RECT D_800BE10C;         /* emblem CLUT rect */
extern char D_800BE114[];       /* save title (SJIS) */
extern u32 D_800BE154[8];       /* icon CLUT */
extern u32 D_800BE174[32];      /* icon image */
extern RECT D_800BE1F4;         /* emblem CLUT rect (bank) */
extern RECT D_800BE1FC;         /* emblem image rect (bank) */
extern CardDir D_801EF6D0[2];

/* Write the emblem save file ("Z") to a card port: header, icon, the 7 emblem
 * slots read back from VRAM with slot idx replaced by the edited one; then
 * read it back and compare the sum. */
int func_800B4398(u8 port, s16 idx)
{
    char name[32];
    char tail[16];
    u8 *buf;
    u8 *rb;
    u8 *p;
    u8 *q;
    u8 *z;
    u8 *p2;
    u8 tries2;
    u32 *src;
    u32 *dst;
    u32 n;
    s16 j;
    u16 k;
    s16 cx;
    s16 cy;
    s16 y;
    s16 i;
    u8 tries;
    u8 bad;
    int fd;
    int r;
    int wr;
    int bx;

    func_800B4054(0);
    buf = func_80050FC4(0x3B00);
    z = buf + 0x204;
    n = 0;
    do {
        *z = 0;
        n++;
        z++;
    } while (n < 0x38E2);
    q = buf + 4;
    j = 0;
    do {
        *q = 0;
        j++;
        q++;
    } while (j < 64);
    q = buf + 0x44;
    j = 0;
    do {
        *q = 0;
        j++;
        q++;
    } while (j < 28);
    buf[0] = 'S';
    buf[1] = 'C';
    buf[2] = 0x11;
    buf[3] = 2;
    strcpy((char *)(buf + 4), D_800BE114);
    dst = (u32 *)(buf + 0x60);
    src = D_800BE154;
    k = 0;
    do {
        *dst++ = *src++;
        k++;
    } while (k < 8);
    dst = (u32 *)(buf + 0x80);
    src = D_800BE174;
    k = 0;
    do {
        *dst++ = *src++;
        k++;
    } while (k < 32);
    dst = (u32 *)(buf + 0x100);
    src = D_800BE174;
    k = 0;
    do {
        *dst++ = *src++;
        k++;
    } while (k < 32);
    dst = (u32 *)(buf + 0x180);
    src = D_800BE174;
    bx = 0x380;
    k = 0;
    do {
        *dst++ = *src++;
        k++;
    } while (k < 32);
    cx = 0x380;
    cy = 0;
    y = 0x80;
    i = 0;
    D_800BE1F4.x = 0x380;
    D_800BE1F4.y = y;
    D_800BE1FC.x = cx;
    D_800BE1FC.y = cy;
    do {
        p = buf + (i * 0x820 + 0x204);
        StoreImage(&D_800BE1F4, (u32 *)p);
        StoreImage(&D_800BE1FC, (u32 *)(p + 0x20));
        DrawSync(0);
        cx += 0x10;
        if (cx == 0x3C0) {
            cx = bx;
            cy += 0x40;
        }
        y++;
        i++;
        D_800BE1F4.x = bx;
        D_800BE1F4.y = y;
        D_800BE1FC.x = cx;
        D_800BE1FC.y = cy;
    } while (i < 7);
    p2 = buf + (idx * 0x820 + 0x204);
    StoreImage(&D_800BE10C, (u32 *)p2);
    StoreImage(&D_800BE104, (u32 *)(p2 + 0x20));
    DrawSync(0);
    *(u32 *)(buf + 0x200) = func_800B4370(buf + 0x204);
    tail[0] = 'Z';
    tail[1] = 0;
    if (port == 0)
        __builtin_memcpy(name, D_8004DBEC, 18);
    else
        __builtin_memcpy(name, D_8004DC00, 18);
    strcat(name, tail);
    if (func_800684A0(port) != 0)
        goto fail;
    tries = 0;
    if (D_801EF6D0[port].n_saves == 0) {
        do {
            fd = open(name, 0x20200);
            close(fd);
        } while (fd == -1 && ++tries < 3);
        if (fd == -1)
            goto fail;
        tries = 0;
    }
    while ((fd = open(name, 0x20002)) == -1) {
        if (++tries >= 3)
            goto fail;
        close(-1);
    }
    tries = 0;
    do {
        if (func_800684A0(port) != 0)
            goto wclose;
        wr = write(fd, buf, 0x3B00);
    } while (wr != 0x3B00 && ++tries < 3);
    if (wr != 0x3B00) {
        close(fd);
        goto fail;
    }
    close(fd);
    tries2 = 0;
    bad = 0;
    rb = func_80050FC4(0x3B00);
    if (func_800684A0(port) != 0)
        goto fail2;
    do {
        while ((fd = open(name, 0x20001)) == -1) {
            if (++tries2 >= 3)
                goto fail2;
            close(-1);
        }
        tries2 = 0;
        do {
            if (func_800684A0(port) != 0)
                goto rclose;
            r = read(fd, rb, 0x3B00);
        } while (r != 0x3B00 && ++tries2 < 3);
        close(fd);
        if (r != 0x3B00)
            goto fail2;
        if (func_800B4370(rb + 0x204) == *(u32 *)(buf + 0x200))
            goto ok;
    } while (++bad < 3);
    goto fail2;
ok:
    func_80051078(rb);
    func_80051078(buf);
    return 1;
wclose:
    func_80029B90(fd);
    func_80051078(buf);
    return 0;
rclose:
    close(fd);
fail2:
    func_80051078(rb);
fail:
    func_80051078(buf);
    return 0;
}
