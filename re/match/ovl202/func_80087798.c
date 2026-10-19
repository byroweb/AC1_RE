#include "ac1_types.h"
/* 4- and 6-byte structs with 2-byte alignment: copied with lwl/lwr + lh. */
typedef struct {
    s16 x;
    s16 y;
} Func80087798Pair;
typedef struct {
    u8 kind;
    u8 pad;
    s16 a;
    s16 b;
} Func80087798Vec;
typedef struct {
    u8 b0;          /* +0x00 */
    u8 b1;          /* +0x01 */
    u16 h2;         /* +0x02 */
    u16 h4;         /* +0x04 */
    u8 pad6[0x14 - 6];
    u16 h14;        /* +0x14 */
    u16 h16;        /* +0x16 */
    u8 pad18[0x22 - 0x18];
    Func80087798Pair p22;   /* +0x22 */
    u8 pad26[0x30 - 0x26];
} Func80087798Def;
typedef struct {
    u8 pad[6];
    s16 h6;         /* +0x06 */
    u8 pad8[0x10 - 8];
    u8 b10;         /* +0x10 */
    u8 pad11;
    s16 h12;        /* +0x12 */
    void (*f14)(void);  /* +0x14 */
    Func80087798Vec pos;    /* +0x18 */
    u8 pad1e[2];
    s32 w20;        /* +0x20 */
    u16 h24;        /* +0x24 */
    u16 h26;        /* +0x26 */
    u16 h28;        /* +0x28 */
    s16 h2a;        /* +0x2A */
    u8 b2c;         /* +0x2C */
    u8 b2d;         /* +0x2D */
    u8 pad2e[0xB8 - 0x2E];
    s16 hb8;        /* +0xB8 */
    s16 hba;        /* +0xBA */
    s16 hbc;        /* +0xBC */
    s16 hbe;        /* +0xBE */
    s16 hc0;        /* +0xC0 */
    s16 hc2;        /* +0xC2 */
    u16 hc4;        /* +0xC4 */
    u16 hc6;        /* +0xC6 */
    u16 hc8;        /* +0xC8 */
    u8 padca[2];
    Func80087798Pair pcc;   /* +0xCC */
} Func80087798Ent;
extern Func80087798Ent D_8008EC88;
extern Func80087798Def D_80090364[];
extern Func80087798Ent *func_80089ECC(s32 kind);
extern void func_80085764(Func80087798Ent *e, s32 a, s32 b, s32 c);
extern s32 func_8002A4A0(void);
extern void func_80085580(void);
Func80087798Ent *func_80087798(s32 a, s32 b, Func80087798Vec *pos, s32 idx, s32 has_pos)
{
    Func80087798Ent *e;
    Func80087798Def *d;
    u8 k;
    e = func_80089ECC(1);
    if (e == &D_8008EC88) {
        return e;
    }
    func_80085764(e, a, b, -1);
    e->b10 = 4;
    d = &D_80090364[idx];
    e->hb8 = d->b0;
    if (d->b0 == 0xFF) {
        e->h6 = 1;
    }
    e->hba = 0;
    e->hbc = func_8002A4A0() & 0xFFF;
    e->hbe = func_8002A4A0() & 0xFFF;
    e->hc0 = func_8002A4A0() & 0xFFF;
    e->hc4 = e->hc6 = e->hc8 = d->h2;
    e->pcc = d->p22;
    if (has_pos) {
        e->pos = *pos;
        e->h26 = d->h14;
        e->h28 = d->h16;
    } else {
        e->pos.kind = 0;
        e->h28 = 0;
        e->h26 = 0;
    }
    e->w20 = 0;
    e->b2d = idx;
    e->h24 = d->h4;
    e->h2a = 0;
    k = d->b1;
    e->f14 = func_80085580;
    e->h12 = k;
    e->b2c = k;
    return e;
}
