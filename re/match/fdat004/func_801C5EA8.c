/* fdat004 0x801C5EA8: matched (--rodata 0x801C4C9C --no-rodata-align-fix); pos.y temp pinned to v0 with a register asm local so the quotient takes v1 like retail */
#include "ac1/ac1.h"
#include "m2c_compat.h"

typedef struct {
    s16 f0;             /* +0x00 */
    s16 f2;             /* +0x02 */
    s16 f4;             /* +0x04 */
    s16 f6;             /* +0x06 */
    u16 dur;            /* +0x08 */
    u8 mode3;           /* +0x0A */
    u8 sfx;             /* +0x0B */
    u8 c;               /* +0x0C */
    u8 d;               /* +0x0D */
    u8 e0;              /* +0x0E */
    u8 mode2;           /* +0x0F */
} Rec;
typedef struct {
    u8 pad0[4];
    WorldStruct *ws;    /* +0x04 */
    u8 pad8[6];
    u16 h0e;            /* +0x0E */
} Probe;
typedef struct {
    u8 b[8];
} Bytes8;

extern void func_80014728(s32, s16 *);
extern void func_8004CC30(s32, s32, void *, s32, s32, s32, s32);
extern void func_8004D178(s32, s32, s32, s32, s32);
extern void func_8004E98C(void *, s32, s32, s32);
extern s32 func_8004EBF8(void *);
extern void func_8004ECA8(void *);
extern void func_800529F0(s32, s32, s32);
extern Probe *func_8006D4BC(s32, s32);
extern s16 func_8006E6B4(s16, void *, s32);
extern Probe *func_80075030(void *, s32);
extern s32 func_800754D0(void *, void *);
extern void func_80078E1C(void *);
extern Rec D_801C70E8[];
extern u8 D_801C71C8[];
extern u8 D_801C71EC[];
extern u8 D_801C71F0[];
extern u8 D_801C71F4[];
extern u8 D_801C71F8[];
extern u8 D_801C71FC[];
extern u8 D_801C7200[];
extern u16 D_801987D8;
extern u8 D_8019F518;
extern s32 D_8019F524;
extern u16 D_8019F52A;
extern Bytes8 D_801A2784;
extern s32 D_801A5DC0;
extern s32 D_801A5DC8;
extern s16 D_801A26EA;
extern s16 D_801A26EE;
typedef struct { u8 pad[0x32]; s16 f32; s16 f34; s16 f36; } PlView;

void func_801C5EA8(AcEntity *e)
{
    Rec *r;
    Probe *o;
    s16 dir[3];
    s16 pad[4];
    s16 yy;
    u16 u0;
    u16 u1;
    s32 mk;
    s32 *pg;
    u16 *qg;
    AcEntity *pl;
    s16 zz;
    u8 *tbl;
    s32 t;
    s32 dy;
    s32 c0;
    s32 d0;
    s32 c;
    s32 d;
    s32 v;
    void *m;
    u8 mode;

    mode = e->ai_mode;
    if (mode != 0xFF) {
        o = func_8006D4BC(1, mode);
        if (o != 0) {
            M2C_FIELD(e, u16 *, 0x2E) = o->h0e;
        } else {
            M2C_FIELD(e, u16 *, 0x2E) = 0x7FFF;
        }
        e->pos.y = e->pos.y + 50;
        if (e->pos.y >= M2C_FIELD(e, s16 *, 0x2C)) {
            e->ai_mode = 0;
        }
        if (func_800754D0(e, D_801C71C8) != 0) {
            func_80078E1C(e);
        }
    } else {
        if (M2C_FIELD(e, u16 *, 0x44) == 0) {
            e->unk42[0] = e->unk42[0] + 1;
            r = &D_801C70E8[e->unk42[0]];
            M2C_FIELD(e, u16 *, 0x44) = r->dur;
            if (r->sfx != mode) {
                switch (D_8019F518) {
                case 6:
                    tbl = D_801C71F0;
                    break;
                case 8:
                    tbl = D_801C71F4;
                    break;
                case 21:
                    tbl = D_801C71F8;
                    break;
                case 26:
                    tbl = D_801C71FC;
                    break;
                case 36:
                    tbl = D_801C7200;
                    break;
                default:
                    tbl = D_801C71EC;
                    break;
                }
                func_800529F0(0, 0, tbl[r->sfx]);
            }
            if (r->e0 != 0) {
                func_8004D178(r->e0, 0, 127, 127, 0);
            }
        } else {
            r = &D_801C70E8[e->unk42[0]];
        }
        func_80014728(e->yaw, dir);
        t = (r[-1].f4 - r->f4) * M2C_FIELD(e, u16 *, 0x44) / r->dur + r->f4;
        e->pos.x = e->pos.x + ((dir[0] * t) >> 12);
        e->pos.z = e->pos.z + ((dir[2] * t) >> 12);
        c = r[-1].f2;
        d = r->f2;
        {
            s32 qv = (c - d) * M2C_FIELD(e, u16 *, 0x44) / r->dur;
            register u16 py asm("$2");
            py = e->pos.y;
            dy = d + qv;
            e->pos.y = py + dy;
        }
        e->section = func_8006E6B4(e->section, &e->pos, dy);
        func_8004CC30(0x85, D_801A5DC0 + 0x1000, &e->pos,
                      (r[-1].c - r->c) * M2C_FIELD(e, u16 *, 0x44) / r->dur + r->c, 13000, 14000,
                      (r[-1].d - r->d) * M2C_FIELD(e, u16 *, 0x44) / r->dur + r->d);
        D_801A5DC8 = 1;
        o = func_80075030(e, 0x1800);
        if (o != 0) {
            M2C_FIELD(e, u16 *, 0x2C) = o->h0e;
            v = o->ws - g_world_structs;
        } else {
            M2C_FIELD(e, s16 *, 0x2C) = 0x7FFF;
            v = -1;
        }
        M2C_FIELD(e, s16 *, 0x30) = v;
        D_801A5DC8 = 0;
        switch (r->mode2) {
        case 0:
            m = (u8 *)e + 0x64;
            func_8004E98C(m, 0, 0, 0);
            func_8004ECA8(m);
            break;
        case 1:
            m = (u8 *)e + 0x64;
            func_8004E98C(m, 1, 1, 0);
            func_8004ECA8(m);
            break;
        case 2:
            m = (u8 *)e + 0x64;
            func_8004E98C(m, 1, -1, 0);
            func_8004ECA8(m);
            break;
        case 3:
            m = (u8 *)e + 0x64;
            func_8004E98C(m, 2, 1, 0);
            func_8004ECA8(m);
            if (func_8004EBF8(m) == 0) {
                func_8004D178(0x86, D_801A5DC0 + 0x1100, 127, 127, 0);
            }
            break;
        case 4:
            m = (u8 *)e + 0x64;
            func_8004E98C(m, 2, -1, 0);
            func_8004ECA8(m);
            if (func_8004EBF8(m) == 0) {
                func_8004D178(0x86, D_801A5DC0 + 0x1100, 127, 127, 0);
            }
            break;
        }
        switch (r->mode3) {
        case 0:
            g_ac_entities[0].pos.x = e->pos.x;
            yy = e->pos.y;
            g_ac_entities[0].pos.y = yy;
            M2C_FIELD(e, u16 *, 0x108) = yy;
            g_ac_entities[0].pos.z = e->pos.z;
            g_ac_entities[0].section = e->section;
            break;
        case 1:
            g_ac_entities[0].pos.x = e->pos.x;
            g_ac_entities[0].pos.z = e->pos.z;
            ((PlView *)&g_ac_entities[0])->f32 = 0;
            g_ac_entities[0].faction = 1;
            u0 = M2C_FIELD(e, u16 *, 0xCE) + 0x100;
            u1 = M2C_FIELD(e, u16 *, 0x108) + ((s16)u0 >> 6);
            M2C_FIELD(e, u16 *, 0xCE) = u0;
            M2C_FIELD(e, u16 *, 0x108) = u1;
            g_ac_entities[0].pos.y = u1;
            break;
        case 2:
            g_ac_entities[0].state = 75;
            {
                s32 *p = &D_8019F524;
                s32 mk2 = ~0x2400;
                u16 *q = &D_801987D8;
                *p &= mk2;
                *q &= 0xFFFE;
            }
            D_801A2784 = *(Bytes8 *)((u8 *)e + 0xCC);
            D_8019F52A = 0xFFFE;
            break;
        case 9:
            if ((u16)(e->pos.x + 32000) > 0xFA00 || (u16)(e->pos.z + 32000) > 0xFA00) {
                func_80078E1C(e);
            }
            break;
        }
        M2C_FIELD(e, u16 *, 0x44) = M2C_FIELD(e, u16 *, 0x44) - 1;
    }
}
