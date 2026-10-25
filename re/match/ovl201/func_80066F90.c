#include "ac1/ac1.h"

int open(char *, int);
int read(int, void *, int);
int write(int, void *, int);
int close(int);
char *strcat(char *, char *);
char *strcpy(char *, char *);
int StoreImage(RECT *, u32 *);
int DrawSync(int);
void *func_80050FC4(s32);
void func_80051078(void *);
void func_80066994(u8);
s32 func_80066F68(void *);
s32 func_800684A0(u8);
void func_80086654(void);
void func_800960C0(void);

typedef struct { u8 b[7]; } Raw7;
typedef struct { u8 b[12]; } Raw12;
typedef struct { u8 b[36]; } Raw36;
typedef struct { u8 b[48]; } Raw48;
typedef struct { u8 b[160]; } Raw160;
typedef struct { u8 b[196]; } Raw196;
typedef struct { s32 w[14]; } Blk56;
typedef struct { s32 w[16]; } Blk64;
typedef struct { s32 w[24]; } Blk96;
typedef struct { s32 w[30]; } Blk120;

typedef struct SaveData {       /* 0xE80-byte save file */
    u8 magic[4];                /* +0x000 "SC", icon flag 0x11, 1 block */
    u8 title[64];               /* +0x004 Shift-JIS title */
    u8 pad44[28];
    u32 clut[8];                /* +0x060 icon CLUT */
    u32 icon[3][32];            /* +0x080 three icon frames */
    u8 pad1[0x200 - 0x200];
    s32 sum;                    /* +0x200 checksum of the data from +0x230 */
    Raw36 name;                 /* +0x204 pilot name shown in the slot list */
    u16 h228;
    u8 pad22a[2];
    s32 w22c;
    Raw12 a230;                 /* +0x230 */
    u8 b23c;
    u8 b23d;
    u16 h23e;
    s32 w240;
    Raw36 a244;
    Blk96 a268;
    Raw48 a2c8;
    Raw160 a2f8;
    Raw160 a398;
    u8 emblem[0x800];           /* +0x438 64x64 4bpp emblem image */
    u16 emblem_clut[16];        /* +0xC38 */
    Blk120 ac58;
    Blk56 acd0;
    Blk64 ad08;
    Raw7 ad48;
    Raw196 ad4f;
} SaveData;

typedef struct CardFile { s32 blocks; char name[64]; } CardFile;
typedef struct CardDir {
    s32 used_blocks;
    s32 n_files;
    s32 n_saves;
    CardFile files[16];
    CardFile saves[16];
} CardDir;
extern CardDir D_801EF6D0[2];
extern u16 D_801A257C;

extern char D_8004B700[18] __attribute__((aligned(4)));  /* "bu00:BASCUS-94182" */
extern char D_8004B714[18] __attribute__((aligned(4)));  /* "bu10:BASCUS-94182" */
extern char D_800B7628[];       /* default save title */
extern u32 D_800B7668[8];       /* icon CLUT */
extern u32 D_800B5608[][32];    /* icon per save letter */
extern u8 D_8017E5A0[];         /* name work buffer */
extern Raw12 D_80031A58;
extern Raw48 D_80031A64;
extern Raw160 D_80031A94;
extern Raw160 D_80031B34;
extern Raw36 D_80031BD4;
extern u16 D_80039CA0;
extern u8 D_80039CA2;
extern u8 D_80039CA3;
extern s32 D_80039CA4;
extern Raw7 D_80039D20;
extern Blk120 D_80041B10;
extern Blk96 D_800485A0;
extern Blk56 D_80048630;
extern Blk64 D_80049E70;
extern Raw196 D_80049EC8;
extern RECT D_800B7618;
extern RECT D_800B7620;

s32 func_80066F90(u8 port, s16 slot)
{
    char path[32];
    char tail[16];
    SaveData *buf, *chk;
    u8 *p;
    u32 *src, *dst;
    u32 i;
    s16 j;
    u16 k;
    u8 c, idx, letter;
    s32 t;
    u16 h, tn;
    int fd, len, n;
    u8 tries, rtries, bad;
    u8 *q;
    u32 *cs;

    func_80066994(0);
    buf = func_80050FC4(0xE80);
    q = (u8 *)&buf->a230;
    for (i = 0; i < 3044; i++)
        *q++ = 0;
    p = buf->title;
    for (j = 0; j < 64; j++)
        *p++ = 0;
    p = buf->pad44;
    for (j = 0; j < 28; j++)
        *p++ = 0;
    buf->magic[0] = 'S';
    buf->magic[1] = 'C';
    buf->magic[2] = 0x11;
    buf->magic[3] = 1;
    strcpy(buf->title, D_800B7628);
    dst = buf->clut;
    cs = D_800B7668;
    for (tn = 0; tn < 8; tn++)
        *dst++ = *cs++;
    func_80086654();
    func_800960C0();
    buf->a230 = D_80031A58;
    buf->a268 = D_800485A0;
    buf->a2c8 = D_80031A64;
    buf->b23c = D_80039CA2;
    buf->b23d = D_80039CA3;
    buf->a2f8 = D_80031A94;
    buf->a398 = D_80031B34;
    buf->w240 = D_80039CA4;
    buf->a244 = D_80031BD4;
    buf->h23e = D_80039CA0;
    buf->ac58 = D_80041B10;
    buf->acd0 = D_80048630;
    buf->ad08 = D_80049E70;
    buf->ad48 = D_80039D20;
    buf->ad4f = D_80049EC8;
    StoreImage(&D_800B7618, (u32 *)buf->emblem);
    StoreImage(&D_800B7620, (u32 *)buf->emblem_clut);
    DrawSync(0);
    buf->sum = func_80066F68(&buf->a230);
    buf->name = D_80031BD4;
    buf->w22c = D_80039CA4;
    buf->h228 = D_80041B10.w[11];
    if (slot != -1) {
        tail[0] = D_801EF6D0[port].saves[slot].name[12];
        tail[1] = 0;
    } else {
        for (letter = 'A'; letter < 'P'; letter++) {
            for (idx = 0; idx < D_801A257C; idx++) {
                if (D_801EF6D0[port].saves[idx].name[12] == letter)
                    break;
            }
            if (idx == D_801A257C)
                break;
        }
        tail[0] = letter;
        tail[1] = 0;
    }
    dst = buf->icon[0];
    src = D_800B5608[(u8)tail[0]];
    for (k = 0; k < 32; k++)
        *dst++ = *src++;
    dst = buf->icon[1];
    src = D_800B5608[(u8)tail[0]];
    for (k = 0; k < 32; k++)
        *dst++ = *src++;
    dst = buf->icon[2];
    src = D_800B5608[(u8)tail[0]];
    for (k = 0; k < 32; k++)
        *dst++ = *src++;
    c = tail[0];
    if (c >= 74) {
        c -= 10;
        buf->title[22] = 0x82;
        buf->title[23] = 0x50;
    }
    buf->title[24] = 0x82;
    c += 15;
    buf->title[25] = c;
    t = D_80041B10.w[11];
    h = (u16)t / 100;
    t -= h * 100;
    buf->title[38] = 0x82;
    buf->title[39] = h + 79;
    h = (u16)t / 10;
    t -= h * 10;
    buf->title[40] = 0x82;
    buf->title[41] = h + 79;
    buf->title[42] = 0x82;
    buf->title[43] = t + 79;
    strcpy(D_8017E5A0, D_80031BD4.b);
    j = 0;
    for (;;) {
        if (j >= 16 || D_8017E5A0[j] == '>')
            break;
        if (D_8017E5A0[j] == 0x84 && (u8)(D_8017E5A0[j + 1] + 97) < 10) {
            D_8017E5A0[j] = 0x81;
            D_8017E5A0[j + 1] = 0xA6;
        }
        j += 2;
    }
    for (j = 0; j < 16; j++) {
        if (D_8017E5A0[j] == '>')
            break;
        buf->title[46 + j] = D_8017E5A0[j];
    }
    if (port == 0)
        __builtin_memcpy(path, D_8004B700, 18);
    else
        __builtin_memcpy(path, D_8004B714, 18);
    strcat(path, tail);
    if (func_800684A0(port) != 0) {
        func_80051078(buf);
        return 0;
    }
    if (slot == -1) {
        tries = 0;
        do {
            fd = open(path, 0x10200);
            close(fd);
            if (fd != -1)
                break;
        } while (++tries < 3);
        if (fd == -1) {
            func_80051078(buf);
            return 0;
        }
    }
    tries = 0;
    while ((fd = open(path, 0x10002)) == -1) {
        if (++tries >= 3) {
            func_80051078(buf);
            return 0;
        }
        close(-1);
    }
    tries = 0;
    do {
        if (func_800684A0(port) != 0)
            goto wabort;
        len = write(fd, buf, 0xE80);
    } while (len != 0xE80 && ++tries < 3);
    if (len != 0xE80) {
        close(fd);
        func_80051078(buf);
        return 0;
    }
    close(fd);
    rtries = 0;
    bad = 0;
    chk = func_80050FC4(0xE80);
    if (func_800684A0(port) != 0) {
        func_80051078(chk);
        func_80051078(buf);
        return 0;
    }
    do {
        while ((fd = open(path, 0x10001)) == -1) {
            if (++rtries >= 3) {
                func_80051078(chk);
                func_80051078(buf);
                return 0;
            }
            close(-1);
        }
        rtries = 0;
        do {
            if (func_800684A0(port) != 0)
                goto rfail;
            do {
                n = read(fd, chk, 0xE80);
            } while (0);
        } while (n != 0xE80 && ++rtries < 3);
        close(fd);
        if (n != 0xE80) {
            func_80051078(chk);
            func_80051078(buf);
            return 0;
        }
        if (func_80066F68(&chk->a230) == buf->sum)
            goto ok;
    } while (++bad < 3);
    func_80051078(chk);
    func_80051078(buf);
    return 0;
ok:
    func_80051078(chk);
    func_80051078(buf);
    return 1;
wabort:
    close(fd);
    func_80051078(buf);
    return 0;
rfail:
    close(fd);
    func_80051078(chk);
    func_80051078(buf);
    return 0;
}
