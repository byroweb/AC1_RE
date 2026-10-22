#include "ac1/ac1.h"
#include "m2c_compat.h"

typedef struct {
    s16 speed;                  /* +0x00 */
    s16 jitter;                 /* +0x02 */
    u16 ang1;                   /* +0x04 */
    u16 ang0;                   /* +0x06 */
    s16 arc;                    /* +0x08 */
    u8 sfx;                     /* +0x0A */
    u8 unkb;
    u32 burst : 8;              /* +0x0C burst count (1 = single) */
    u32 spread : 8;             /* +0x0D */
    u32 count : 8;              /* +0x0E */
    u32 fx : 2;
    u32 fxidx : 6;
    s16 spin0;                  /* +0x10 */
    s16 spin1;                  /* +0x12 */
    u8 unk14[8];
    DVECTOR fxcol;              /* +0x1C */
    s32 unk20;                  /* +0x20 */
    s32 unk24;                  /* +0x24 */
} SpreadDef;

typedef struct {
    s16 a;
    s16 b;
} FxPair;

extern s32 func_8006D534(void *owner, Projectile *p, void *vel, void *out);
extern void func_800835E8(void *owner, s32 hit, void *a, void *b, void *c);
extern Projectile *func_80085948(void *owner, void *pos, s32 a, s32 b, s32 c, s32 section, DVECTOR *d);
extern Projectile *func_80086854(void *owner, void *pos, void *flags);
extern void func_800869F0(void *owner, void *pos, s32 spread, s32 n, void *flags);
extern Projectile *func_80086C9C(void *, void *, s32, s32, s32, s32, s32, s32, s32, s32, s32, s32);
extern void func_800873F8(s32 hit, void *pos, s32 a, s32 b, s32 c, s32 d);
extern SpreadDef D_8008FC48[];
extern FxPair D_8008FEC8[];

#define TGT ((u8 *)p + 0x28)

/* think for func_800889B4's shells */
void func_80088664(Projectile *p)
{
    SpreadDef *def;
    SVECTOR rel;
    s32 hit;
    u32 bits;
    u32 bits2;
    s32 n;

    def = &D_8008FC48[p->vel[1]];
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
                n = (M2C_FIELD(p, u32 *, 0x30) >> 17) & 0x3FF;
                func_80086C9C(0, TGT, 40, n, n * 4, n >> 4, n >> 2, -n >> 8, -n >> 3, n >> 2, 65, 0);
            }
            if (M2C_FIELD(p, s16 *, 0x2E) == 1) {
                if (def->burst == 1) {
                    func_80086854(0, TGT, (u8 *)p + 0x30);
                } else {
                    func_800869F0(0, TGT, def->spread, def->burst, (u8 *)p + 0x30);
                }
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
    p->section = world_section_find(p->section, p);
    if (def->fx) {
        func_80085948(0, p, D_8008FEC8[def->fxidx].a, D_8008FEC8[def->fxidx].b, 100, p->section,
                      &def->fxcol)->unk12 = 0;
    }
    if (def->spin0 != 0) {
        p->ang[0] += 1;
    }
    if (def->spin1 != 0) {
        p->ang[1] += def->spin1;
    }
}
