#include "ac1/ac1.h"
/* ovl204 HUD for the side in D_80039D18: weapon list with ammo, AP counter,
 * radar frame and blips. */
typedef struct {                /* 0x3C weapon slot */
    u8 pad0[0x0E];
    u16 max;                    /* +0x0E */
    u16 count;                  /* +0x10 */
    u8 pad12[0x2A - 0x12];
    u8 name[0x12];              /* +0x2A */
} WSlot4;
typedef struct {
    u8 pad0[0x12];
    s16 yaw;                    /* +0x12 */
    u8 pad14[0x160 - 0x14];
    u16 ap;                     /* +0x160 */
    u16 ap2;                    /* +0x162 */
} HEnt4;
typedef struct {                /* 0x39C */
    u8 pad0[0x10];
    u16 flags;                  /* +0x10 */
    u8 pad12[0x19 - 0x12];
    u8 cur;                     /* +0x19 */
    u8 pad1a[0x226 - 0x1A];
    SVECTOR v226;               /* +0x226 */
    u8 pad22e[8];
    SVECTOR v236;               /* +0x236 */
    u8 pad23e[0x270 - 0x23E];
    u16 range;                  /* +0x270 */
    u8 pad272[0x277 - 0x272];
    u8 b277;                    /* +0x277 */
    u8 pad278;
    u8 radar;                   /* +0x279 */
    u8 pad27a[0x2D8 - 0x27A];
    u8 nweap;                   /* +0x2D8 */
    u8 pad2d9;
    WSlot4 weap[3];             /* +0x2DA */
    u8 pad38e[0x394 - 0x38E];
    HEnt4 *ent;                 /* +0x394 */
    u8 pad398[4];
} HSide4;
typedef struct {
    s16 x, y;
} Pt16;
extern HSide4 *D_80039D18;
extern HSide4 D_80040FA0[2];
extern s32 D_800AD958;
extern Pt16 D_80089484;
extern u8 D_80089488[];
extern u8 D_8008948C[];
extern u8 D_80089490[];
extern u8 D_80089494[];
extern u8 D_80089498[];
extern u8 D_8008949C[];
extern u8 D_800894A0[];
extern u8 D_800893E4[][12];
extern u8 D_80089414[];
extern s32 func_8005145C(s32 x, s32 y, s32 ot, s32 font, s32 colour, u8 *s);
extern s32 func_800125D0(u8 *buf, s32 v);
extern void func_80012670(u8 *buf, u32 v, s32 w);
extern s32 rsin(s32 a);
extern s32 rcos(s32 a);
extern void func_80060050(DVECTOR *p, u8 *c, s32 n, s32 ot, s32 f);
extern void func_8005FB00(Pt16 *p, u8 *a, s32 b, u8 *c, s32 d, s32 e);
extern void func_800505E8(void *pos, u8 *a, u8 *b, u8 *c, s32 s, s32 co);
extern void func_80050490(SVECTOR *v, u8 *c, s32 s, s32 co);
extern void func_80050190(s32 s, s32 co);

void func_80050E60(void)
{
    u8 buf[16];
    u8 abuf[16];
    DVECTOR pts[5];
    WSlot4 *w;
    s32 col;
    s32 y;
    s32 n;
    s32 i;
    s32 len;
    s32 x;
    s32 s;
    s32 c;
    s32 k;

    if (D_80039D18->nweap != 0) {
        w = &D_80039D18->weap[D_80039D18->cur];
        col = 8;
        if (w->count == 0) {
            col = 1;
        }
        func_8005145C(D_800AD958 + 54, 168, 3, 128, col, w->name);
        y = 186;
        len = func_800125D0(buf, w->count);
        buf[len] = '/';
        buf[len + 1] = 0;
        { s32 p = (4 - len) * 6; s32 t = D_800AD958 + 96;
        func_8005145C(p + t, 176, 3, 128, col, buf); }
        { s32 p = (4 - func_800125D0(buf, w->max)) * 6; s32 t = D_800AD958 + 126;
        func_8005145C(p + t, 176, 3, 128, col, buf); }
        w++;
        i = D_80039D18->cur + 1;
        n = D_80039D18->nweap;
        while (--n > 0) {
            if (i >= D_80039D18->nweap) {
                w = D_80039D18->weap;
                i = 0;
            }
            col = w->count == 0;
            func_8005145C(D_800AD958 + 66, y, 3, 56, col, w->name);
            len = func_800125D0(buf, w->count);
            w++;
            { s32 p = len * 4; s32 t = D_800AD958 + 150;
            func_8005145C(t - p, y, 3, 56, col, buf); }
            y += 8;
            i++;
        }
    }
    func_80012670(abuf, (D_80039D18->ent->ap + 3U) >> 2, 4);
    x = 15;
    if (D_800AD958 != 0) {
        x = 243;
    }
    func_8005145C(x, 13, 3, 220, D_80039D18->ent->ap * 10 <= D_80039D18->ent->ap2, abuf);
    s = rsin(D_80039D18->ent->yaw);
    c = rcos(D_80039D18->ent->yaw);
    D_80089484.x = D_800AD958 + 20;
    if (D_80039D18->range != 0) {
        k = 80000 / D_80039D18->range;
        pts[0].vx = D_800AD958 - (s16)(k - 43);
        pts[0].vy = 191 - k;
        pts[1].vx = D_800AD958 + (s16)(k + 44);
        pts[1].vy = 191 - k;
        pts[2].vx = D_800AD958 + (s16)(k + 44);
        pts[2].vy = k + 191;
        pts[3].vx = D_800AD958 - (s16)(k - 43);
        pts[3].vy = k + 191;
        pts[4] = pts[0];
        func_80060050(pts, D_800894A0, 4, 3, 0);
        func_8005FB00(&D_80089484, D_800893E4[D_80039D18->radar], 0, D_80089488, 3, 2);
        func_800505E8(&(D_80039D18 == &D_80040FA0[0] ? D_80040FA0[1].ent : D_80040FA0[0].ent)->pad0[8],
                      D_80089494, D_80089498, D_8008949C, s, c);
        if (D_80039D18->flags & 1) {
            func_80050490(&D_80039D18->v226, D_8008948C, s, c);
            func_80050490(&D_80039D18->v236, D_80089490, s, c);
        }
        if (D_80039D18->b277 & 1) {
            func_80050190(s, c);
        }
    } else {
        func_8005FB00(&D_80089484, D_80089414, 0, D_80089488, 15, 2);
        if (D_80040FA0[0].flags & 1) {
            D_80039D18->range = 5000;
            func_80050490(&D_80040FA0[0].v226, D_8008948C, s, c);
            func_80050490(&D_80040FA0[0].v236, D_80089490, s, c);
            D_80039D18->range = 0;
        }
    }
}
