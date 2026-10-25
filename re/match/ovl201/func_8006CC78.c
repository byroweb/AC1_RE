/* ovl201 0x8006CC78 - colour-value readout: a swatch tile, then for each of r,g,b (>>3) a two-digit 4x8 sprite number. Levers: q = *p >> 3 in a u32 then q /= 10 (stops C narrowing (*p>>3)/10 to u8), u8 tens = q copy with the test on q, h=8 before v0=248 (loop.c hoists 8), x0 uses (u16)(SCR.w - 4) so fold keeps x - (w - 4). */
#include "ac1_types.h"

typedef struct {
    u8 r, g, b, cd;
} CVECTOR;

typedef struct {
    s16 x, y, w, h;
} RECT;

typedef struct {
    u32 addr : 24;
    u32 len : 8;
    u8 r0, g0, b0, code;
} P_TAG;

typedef struct {
    u32 tag;
    u8 r0, g0, b0, code;
    s16 x0, y0;
    s16 w, h;
} TILE;

typedef struct {
    u32 tag;
    u8 r0, g0, b0, code;
    s16 x0, y0;
    u8 u0, v0;
    u16 clut;
    s16 w, h;
} SPRT;

typedef struct {
    u32 tag;
    u32 code[2];
} DR_MODE;

#define setaddr(p, _addr) (((P_TAG *)(p))->addr = (u32)(_addr))
#define getaddr(p) (u32)(((P_TAG *)(p))->addr)
#define addPrim(ot, p) setaddr(p, getaddr(ot)), setaddr(ot, p)
#define setlen(p, _len) (((P_TAG *)(p))->len = (u8)(_len))
#define setcode(p, _code) (((P_TAG *)(p))->code = (u8)(_code))
#define setTile(p) setlen(p, 3), setcode(p, 0x60)
#define setSprt(p) setlen(p, 4), setcode(p, 0x64)

typedef struct {
    u8 pad[0x18];
    u32 *ot[2];             /* +0x18 UiObj ordering tables */
} EmbOtObj;

typedef struct {
    u8 pad[0x86];
    u8 color;               /* +0x86 selected palette entry */
    u8 pad87[0x98 - 0x87];
    EmbOtObj *cursor;       /* +0x98 */
} EmbEdit;

typedef struct {
    u16 x;      /* +0x00 */
    u16 y;      /* +0x02 */
    u8 pad4[4];
    u16 w;      /* +0x08 */
    u16 h;      /* +0x0A */
    u8 padC[103512 - 0xC];
} Screen;

typedef struct {
    u8 player;  /* +0x00 */
    u8 pad[3];
    void *prim; /* +0x04 */
} Gpu;

extern Gpu D_801EF6C8;
extern Screen D_801BCEEC[];
extern CVECTOR D_8017E5D8[];

void SetDrawMode(DR_MODE *p, int dfe, int dtd, int tpage, RECT *tw);

#define SCR D_801BCEEC[D_801EF6C8.player]
#define OT(e) (&ots->ot[D_801EF6C8.player][2])

void func_8006CC78(EmbEdit *e)
{
    TILE *t;
    SPRT *s;
    DR_MODE *d;
    u8 *p;
    s16 i;
    s32 x, y;
    u32 v;
    u8 tens;
    EmbOtObj *ots;

    t = D_801EF6C8.prim;
    ots = e->cursor;
    D_801EF6C8.prim = t + 1;
    setTile(t);
    t->x0 = SCR.x + 42 - SCR.w;
    t->y0 = SCR.y + 188 - SCR.h;
    t->w = 15;
    t->h = 15;
    *(CVECTOR *)&t->r0 = D_8017E5D8[e->color];
    addPrim(OT(e), t);
    p = &D_8017E5D8[e->color].r;
    x = 136;
    for (i = 0, y = 184; i < 3; i++, y += 8) {
        {
            u32 q = *p >> 3;

            q /= 10;
            tens = q;
            if (q & 0xFF) {
                s = D_801EF6C8.prim;
                D_801EF6C8.prim = s + 1;
                setSprt(s);
                s->x0 = SCR.x + (x - SCR.w);
                s->y0 = SCR.y + (y - SCR.h);
                s->w = 4;
                s->h = 8;
                s->v0 = 248;
                s->u0 = tens * 4;
                s->clut = 0x3957;
                s->b0 = 128;
                s->g0 = 128;
                s->r0 = 128;
                addPrim(OT(e), s);
            }
        }
        s = D_801EF6C8.prim;
        v = *p++ >> 3;
        D_801EF6C8.prim = s + 1;
        setSprt(s);
        tens = v - tens * 10;
        s->x0 = SCR.x + (x - (u16)(SCR.w - 4));
        s->y0 = SCR.y + (y - SCR.h);
        s->w = 4;
        s->h = 8;
        s->v0 = 248;
        s->u0 = tens * 4;
        s->clut = 0x3957;
        s->b0 = 128;
        s->g0 = 128;
        s->r0 = 128;
        addPrim(OT(e), s);
    }
    d = D_801EF6C8.prim;
    D_801EF6C8.prim = d + 1;
    SetDrawMode(d, 0, 0, 0x49, 0);
    addPrim(OT(e), d);
}
