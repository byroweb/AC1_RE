#include "ac1/ac1.h"

int open(char *, int);
int read(int, void *, int);
int close(int);
char *strcat(char *, char *);
int LoadImage(RECT *, u32 *);
int DrawSync(int);
void *func_80050FC4(s32);
void func_80051078(void *);
s32 func_80066F68(void *);
s32 func_800684A0(u8);
void func_800AE838(void);
void func_8008C3DC(void);
void func_80086EC8(void);
void func_800866A8(void);
void func_8009646C(void);
void func_80096178(void);

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
    u8 hdr[0x200];              /* +0x000 memory card title block */
    s32 sum;                    /* +0x200 checksum of the data from +0x230 */
    u8 pad204[0x2C];
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

extern char D_8004B700[18] __attribute__((aligned(4)));  /* "bu00:BASCUS-94182" */
extern char D_8004B714[18] __attribute__((aligned(4)));  /* "bu10:BASCUS-94182" */
extern Raw12 D_80031A58;
extern Raw48 D_80031A64;
extern Raw160 D_80031A94;
extern Raw160 D_80031B34;
extern Raw36 D_80031BD4;
extern u16 D_80039CA0;
extern u8 D_80039CA2;
extern u8 D_80039CA3;
extern s32 D_80039CA4;
extern u8 D_80039CB0;
extern Raw7 D_80039D20;
extern Blk120 D_80041B10;
extern Blk96 D_800485A0;
extern Blk56 D_80048630;
extern Blk64 D_80049E70;
extern Raw196 D_80049EC8;
extern RECT D_800B7618;
extern RECT D_800B7620;

s32 func_80067CF0(u8 port, s16 slot)
{
    char path[32];
    char tail[16];
    SaveData *buf;
    int fd, n;
    u8 tries, bad;

    buf = func_80050FC4(0xE80);
    if (func_800684A0(port) != 0) {
        func_80051078(buf);
        return;
    }
    tail[0] = D_801EF6D0[port].saves[slot].name[12];
    tail[1] = 0;
    if (port == 0)
        __builtin_memcpy(path, D_8004B700, 18);
    else
        __builtin_memcpy(path, D_8004B714, 18);
    strcat(path, tail);
    tries = 0;
    bad = 0;
    do {
        while ((fd = open(path, 0x10001)) == -1) {
            if (++tries >= 3) {
                func_80051078(buf);
                return 0;
            }
            close(-1);
        }
        tries = 0;
        do {
            do {
                n = read(fd, buf, 0xE80);
            } while (0);
        } while (n != 0xE80 && ++tries < 3);
        close(fd);
        if (n != 0xE80) {
            func_80051078(buf);
            return 0;
        }
        if (func_80066F68(&buf->a230) == buf->sum)
            goto ok;
    } while (++bad < 3);
    func_80051078(buf);
    return 0;
ok:
    D_80031A58 = buf->a230;
    D_800485A0 = buf->a268;
    D_80031A64 = buf->a2c8;
    D_80039CA2 = buf->b23c;
    D_80039CA3 = buf->b23d;
    D_80031A94 = buf->a2f8;
    D_80031B34 = buf->a398;
    D_80039CA4 = buf->w240;
    D_80031BD4 = buf->a244;
    D_80039CA0 = buf->h23e;
    D_80041B10 = buf->ac58;
    D_80048630 = buf->acd0;
    D_80049E70 = buf->ad08;
    D_80039D20 = buf->ad48;
    D_80049EC8 = buf->ad4f;
    LoadImage(&D_800B7618, (u32 *)buf->emblem);
    LoadImage(&D_800B7620, (u32 *)buf->emblem_clut);
    DrawSync(0);
    func_800AE838();
    func_8008C3DC();
    func_80086EC8();
    func_800866A8();
    func_8009646C();
    func_80096178();
    D_80039CB0 = 1;
    func_80051078(buf);
    return 1;
}
