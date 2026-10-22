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

extern void func_80056B54(s32, void *, s32, s32, s32, Rgba *, s32);
extern s32 func_8006D534(void *owner, Projectile *p, void *vel, void *out);
extern void func_800835E8(void *owner, s32 hit, void *a, void *b, void *c);
extern Projectile *func_80086854(void *owner, void *pos, void *flags);
extern Projectile *func_80086C9C(void *, void *, s32, s32, s32, s32, s32, s32, s32, s32, s32, s32);
extern void func_800873F8(s32 hit, void *pos, s32 a, s32 b, s32 c, s32 d);
extern void func_80087798(void *owner, void *pos, Shard6 *s, s32 n, s32 e);
extern Rgba D_8008FB48;

#define TGT ((u8 *)p + 0x26)

/* shell think (weapon table 0x8008FB58): fly, hit test, burst at the target */
void func_800880BC(Projectile *p)
{
    SVECTOR rel;
    Shard6 sh;
    s32 hit;
    u32 bits;
    u32 bits2;
    s32 n;
    s16 k;

    if (p->unk12 == 0) {
        if (M2C_FIELD(p, s16 *, 0x2C) == 0) {
            goto done;
        }
        rel.vx = M2C_FIELD(p, s16 *, 0x26) - p->x;
        rel.vy = M2C_FIELD(p, s16 *, 0x28) - p->y;
        rel.vz = M2C_FIELD(p, s16 *, 0x2A) - p->z;
        if (func_8006D534(p->owner, p, &rel, TGT) != 0) {
            goto fly;
        }
    final:
        if (M2C_FIELD(p, s16 *, 0x2C) != 0) {
            if (M2C_FIELD(p, s16 *, 0x2C) == 2) {
                n = (M2C_FIELD(p, u32 *, 0x30) >> 17) & 0x3FF;
                func_80086C9C(0, TGT, 40, n, n * 4, n >> 4, n >> 2, -n >> 8, -n >> 3, n >> 2, 65, 0);
            }
            k = M2C_FIELD(p, s16 *, 0x2C);
            if (k == 1) {
                if (*(u16 *)&p->unkc4[0] == 34) {
                    sh = *(Shard6 *)p->dtint;
                    sh.b >>= 3;
                    sh.c >>= 2;
                    func_80087798(0, TGT, &sh, 11, k);
                } else {
                    func_80086854(0, TGT, (u8 *)p + 0x30);
                }
            }
            *(SVECTOR *)p = *(SVECTOR *)TGT;
            p->section = world_section_find(p->section, p);
            bits = M2C_FIELD(p, u32 *, 0x34);
            func_800873F8(M2C_FIELD(p, s32 *, 0x38), TGT, bits & 0x3FF, (bits >> 10) & 0xFF,
                          (bits >> 18) & 0x1F, 1);
            func_800835E8(p->owner, M2C_FIELD(p, s32 *, 0x38), p->dtint, (u8 *)p + 0x1E, TGT);
        }
        goto done;
    }
fly:
    hit = func_8006D534(p->owner, p, (u8 *)p + 0x1E, TGT);
    if (hit != 0) {
        bits2 = M2C_FIELD(p, u32 *, 0x34);
        func_800873F8(hit, TGT, bits2 & 0x3FF, (bits2 >> 10) & 0xFF, (bits2 >> 18) & 0x1F, 1);
        func_800835E8(p->owner, hit, p->dtint, (u8 *)p + 0x1E, TGT);
        goto final;
    }
done:
    if (p->spin[0]-- < 0) {
        p->ang[0] += p->vel[1];
        p->ang[1] += p->vel[2];
        p->ang[2] += p->unk22;
    }
    p->x += p->vel[1];
    p->y += p->vel[2];
    p->z += p->unk22;
    p->section = world_section_find(p->section, p);
    if (*(u16 *)&p->unkc4[0] == 34) {
        func_80056B54(0, p, 0, 600, 0xFFFF, &D_8008FB48, 500);
    }
}
