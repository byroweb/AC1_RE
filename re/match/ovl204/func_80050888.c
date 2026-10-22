#include "ac1/ac1.h"
/* ovl204 HUD warnings for the side in D_80039D18: energy/heat bars and the
 * blinking warning sprites (with their sounds). */
typedef struct {
    u8 pad0[8];
    AcPos pos;                  /* +0x08 */
    u8 pad10[0x160 - 0x10];
    u16 ap;                     /* +0x160 */
    u16 ap2;                    /* +0x162 */
} WEnt4;
typedef struct {
    u8 pad0[0x10];
    u16 flags;                  /* +0x10 */
    u8 pad12[0x1E - 0x12];
    u16 en;                     /* +0x1E */
    u16 h20;                    /* +0x20 */
    u8 pad22[0x236 - 0x22];
    s16 ax0;                    /* +0x236 */
    u8 pad238[2];
    s16 az0;                    /* +0x23A */
    u8 pad23c[2];
    s16 ax1;                    /* +0x23E */
    u8 pad240[2];
    s16 az1;                    /* +0x242 */
    u8 pad244[2];
    s16 h246;                   /* +0x246 */
    s16 h248;                   /* +0x248 */
    s16 h24A;                   /* +0x24A */
    s16 h24C;                   /* +0x24C */
    s16 h24E;                   /* +0x24E */
    s16 h250;                   /* +0x250 */
    u8 pad252[0x264 - 0x252];
    u16 en_max;                 /* +0x264 */
    u8 pad266[4];
    u16 heat;                   /* +0x26A */
    u8 pad26c[0x394 - 0x26C];
    WEnt4 *ent;                 /* +0x394 */
} WSide4;
typedef struct {
    s16 x, y;
} Pt16;
extern WSide4 *D_80039D18;
extern s32 D_801B4104;
extern s32 D_801B4114;
extern u32 D_800894C0[];
extern u32 D_800894C4[];
extern u32 D_800894E8[];
extern u8 D_8004ADA8[];
extern Pt16 D_8008942C, D_80089440, D_80089450, D_80089460, D_80089470, D_80089480;
extern u8 D_80089420[], D_80089430[], D_80089434[], D_80089444[], D_80089454[], D_80089464[], D_80089474[];
extern void func_8005FB00(Pt16 *p, u8 *a, s32 b, u32 *c, s32 d, s32 e);
extern s32 func_8005145C(s32 x, s32 y, s32 ot, s32 font, s32 colour, u8 *s);
extern void func_8005D784(s32 a, s32 b, s32 c, s32 d, void *col, s32 e, s32 f);
extern void func_8004D0C8(s32 id, s32 vol);

void func_80050888(void)
{
    s32 e;
    s32 h;
    s32 d;
    s32 t;
    s32 k;
    WEnt4 *ent;
    SVECTOR unused[2];

    if (D_801B4104 == 0) {
        return;
    }
    func_8005FB00(&D_8008942C, D_80089420, 0, D_800894C0, 15, 0);
    func_8005145C(8, 10, 16, 56, 0, D_8004ADA8);
    e = 60 - D_80039D18->en * 60 / D_80039D18->en_max;
    h = 60 - D_80039D18->heat * 60 / D_80039D18->en_max;
    if (e != 0) {
        func_8005D784(9, 29, 13, e + 29, D_80089430, 15, 2);
    }
    if (e != 60) {
        if (D_80039D18->h20 & 1) {
            func_8005D784(9, e + 29, 13, 89, &D_800894C0[10], 15, -1);
        } else {
            if (e < h) {
                func_8005D784(9, e + 29, 13, h + 29, &D_800894C0[9], 15, -1);
                func_8005D784(9, h + 29, 13, 89, &D_800894C0[10], 15, -1);
                D_80039D18->h246 = 0;
            } else {
                if (D_80039D18->h248 < e) {
                    D_80039D18->h246 -= e - h;
                    if (D_80039D18->h246 <= 0) {
                        func_8004D0C8(41, 100);
                        D_80039D18->h246 = (60 - h) * 2;
                    }
                }
                func_8005D784(9, e + 29, 13, 89, D_800894E8, 15, -1);
            }
            D_80039D18->h248 = e;
        }
    }
    if (D_80039D18->h24E != 0) {
        if ((D_80039D18->h24E >> 2) & 3) {
            func_8005FB00(&D_80089440, D_80089434, 0, D_800894C0, 15, 2);
        }
        D_80039D18->h24E--;
    }
    if (D_80039D18->h250 != 0) {
        if ((D_80039D18->h250 >> 2) & 3) {
            func_8005FB00(&D_80089450, D_80089444, 0, D_800894C4, 15, 2);
        }
        D_80039D18->h250--;
    }
    if ((D_80039D18->flags & 3) == 1) {
        if (D_80039D18->h24A >= D_80039D18->h24C) {
            func_8005FB00(&D_80089460, D_80089454, 0, D_800894C4, 15, 2);
        }
        if (--D_80039D18->h24A == -1) {
            func_8004D0C8(8, 90);
            ent = D_80039D18->ent;
            d = D_80039D18->ax0 - ent->pos.x;
            if (d < 0) {
                d = ent->pos.x - D_80039D18->ax0;
            }
            t = D_80039D18->az0 - ent->pos.z;
            if (t < 0) {
                t = ent->pos.z - D_80039D18->az0;
            }
            if (t < d) {
                d = t;
            }
            t = D_80039D18->ax1 - ent->pos.x;
            if (t < 0) {
                t = ent->pos.x - D_80039D18->ax1;
            }
            if (t < d) {
                d = t;
            }
            t = D_80039D18->az1 - ent->pos.z;
            if (t < 0) {
                t = ent->pos.z - D_80039D18->az1;
            }
            if (t < d) {
                d = t;
            }
            d >>= 8;
            if (d < 3) {
                d = 3;
            }
            D_80039D18->h24A = d;
            D_80039D18->h24C = (d >> 1) - (d >> 3);
        }
    } else {
        D_80039D18->h24C = 0;
        D_80039D18->h24A = 0;
    }
    if (D_80039D18->ent->ap2 >= D_80039D18->ent->ap * 10) {
        k = D_801B4114 & 7;
        if (k == 0) {
            func_8004D0C8(9, 90);
        }
        if (k < 5) {
            func_8005FB00(&D_80089470, D_80089464, 0, D_800894C4, 15, 2);
        }
    }
    if (D_80039D18->h20 & 1) {
        k = (D_801B4114 + 3) & 7;
        if (k < 5) {
            func_8005FB00(&D_80089480, D_80089474, 0, D_800894C4, 15, 2);
            if (k == 0) {
                func_8004D0C8(40, 90);
            }
        }
    }
}
