#include "ac1/ac1.h"
#include "m2c_compat.h"

typedef struct {
    s16 a;
    u16 b;
    u8 c;
    u8 d;
} Shard6;

typedef struct {
    u8 r, g, b, cd;
} Rgba;

typedef struct {
    s16 speed;                  /* +0x00 */
    u16 size;                   /* +0x02 */
    u8 unk4[2];
    s16 model;                  /* +0x06 */
    u8 sfx;                     /* +0x08 */
    u8 unk9;
    u8 burst;                   /* +0x0A */
    u8 spread;                  /* +0x0B */
    s32 glow;                   /* +0x0C */
    Rgba glowcol;               /* +0x10 (cd = trail jitter) */
    u8 unk14[3];
    u8 shard;                   /* +0x17 */
    u8 split;                   /* +0x18 */
    u8 trail[3];                /* +0x19 */
    s32 unk1c;                  /* +0x1C */
    s32 unk20;                  /* +0x20 */
} MissileDef24;

extern void func_80056B54(s32, void *, s32, s32, s32, Rgba *, s32);
extern s32 func_8006D534(void *owner, Projectile *p, void *vel, void *out);
extern void func_800835E8(void *owner, s32 hit, void *a, void *b, void *c);
extern Projectile *func_8008589C(void *owner, void *pos, s32 a, s32 b, s32 c, s32 section);
extern void func_800869F0(void *owner, void *pos, s32 spread, s32 n, void *flags);
extern Projectile *func_80086C9C(void *, void *, s32, s32, s32, s32, s32, s32, s32, s32, s32, s32);
extern void func_800873F8(s32 hit, void *pos, s32 a, s32 b, s32 c, s32 d);
extern void func_80087798(void *owner, void *pos, Shard6 *s, s32 n, s32 e);
extern Projectile *func_80087A68(void *pos, s32 a, s32 b, s32 c, s32 life, Rgba *col, Rgba *w,
                                 s32 jitter);
extern Projectile *func_80089DE4(void *pos, SVECTOR *vel);
extern s32 rand(void);
extern MissileDef24 D_8008FEE8[];

#define TGT ((u8 *)p + 0x28)

/* think for func_8008915C's guided missiles */
void func_80088C80(Projectile *p)
{
    MissileDef24 *def;
    SVECTOR rel;
    Rgba c0;
    Rgba c1;
    Shard6 sh;
    SVECTOR v;
    s32 kind;
    s32 hit;
    u32 bits;
    u32 bits2;
    s32 n;
    s32 q;
    s16 k;
    s32 i;
    s32 sec;
    s32 m;

    kind = p->vel[1];
    def = &D_8008FEE8[kind];
    if (kind < 6) {
        M2C_FIELD(p, u16 *, 0xC6) = M2C_FIELD(p, u16 *, 0xC6) == 0;
    }
    if (kind == 6) {
        p->unkc4[8] -= 4;
        p->unkc4[9] -= 4;
        p->unkc4[10] -= 4;
    }
    if (kind == 7) {
        p->unkc4[8] -= 12;
        p->unkc4[9] -= 12;
        p->unkc4[10] -= 12;
    }
    if (p->unk12 == 0) {
        if (M2C_FIELD(p, s16 *, 0x2E) == 0) {
            goto move;
        }
        rel.vx = M2C_FIELD(p, s16 *, 0x28) - p->x;
        rel.vy = M2C_FIELD(p, s16 *, 0x2A) - p->y;
        rel.vz = M2C_FIELD(p, s16 *, 0x2C) - p->z;
        if (func_8006D534(p->owner, p, &rel, TGT) != 0) {
            goto fly;
        }
    final:
        if (M2C_FIELD(p, s16 *, 0x2E) != 0) {
            if (M2C_FIELD(p, s16 *, 0x2E) == 2) {
                q = (M2C_FIELD(p, u32 *, 0x30) >> 17) & 0x3FF;
                func_80086C9C(0, TGT, 40, q, q * 4, q >> 4, q >> 2, -q >> 8, -q >> 3, q >> 2, 65, 0);
            }
            k = M2C_FIELD(p, s16 *, 0x2E);
            if (k == 1) {
                n = 5;
                if (def->shard != 0) {
                    sh = *(Shard6 *)p->dtint;
                    if (kind == 6) {
                        n = 17;
                    }
                    sh.b >>= 3;
                    sh.c >>= 2;
                    func_80087798(0, TGT, &sh, n, k);
                } else if (def->split != 0) {
                    func_8008589C(0, TGT, 35, def->split << 6, 0, p->section)->unk12 =
                        ((M2C_FIELD(p, u32 *, 0x30) >> 5) & 0x1F) - 2;
                }
                if (kind == 6) {
                    i = 15;
                    do {
                        i--;
                        v.vx = ((rand() << 6) >> 15) - 32;
                        v.vy = ((rand() * 40) >> 15) - 40;
                        v.vz = ((rand() << 6) >> 15) - 32;
                        func_80089DE4(TGT, &v);
                    } while (i != -1);
                }
                func_800869F0(0, TGT, def->spread, def->burst, (u8 *)p + 0x30);
            }
            *(SVECTOR *)p = *(SVECTOR *)TGT;
            p->section = world_section_find(p->section, p);
            bits = M2C_FIELD(p, u32 *, 0x34);
            func_800873F8(M2C_FIELD(p, s32 *, 0x38), TGT, bits & 0x3FF, (bits >> 10) & 0xFF,
                          (bits >> 18) & 0x1F, 1);
            func_800835E8(p->owner, M2C_FIELD(p, s32 *, 0x38), p->dtint, (u8 *)p + 0x20, TGT);
            p->unk12 = 0;
        }
        goto move;
    }
fly:
    hit = func_8006D534(p->owner, p, (u8 *)p + 0x20, TGT);
    if (hit != 0) {
        bits2 = M2C_FIELD(p, u32 *, 0x34);
        func_800873F8(hit, TGT, bits2 & 0x3FF, (bits2 >> 10) & 0xFF, (bits2 >> 18) & 0x1F, 1);
        func_800835E8(p->owner, hit, p->dtint, (u8 *)p + 0x20, TGT);
        goto final;
    }
move:
    p->x += p->vel[2];
    p->y += p->unk22;
    p->z += p->spin[0];
    sec = world_section_find(p->section, p);
    p->section = sec;
    if (def->glow != 0) {
        func_80056B54(0, p, 0, (s16)sec, 0xFFFF, &def->glowcol, def->glow);
    }
    m = 293;
    if (kind == 6) {
        m = 295;
    }
    c0.b = 72;
    c0.g = 72;
    c0.r = 72;
    c1.b = 8;
    c1.g = 8;
    c1.r = 8;
    func_80087A68(p, m, def->trail[0], def->trail[1], def->trail[2], &c0, &c1, def->glowcol.cd);
}
