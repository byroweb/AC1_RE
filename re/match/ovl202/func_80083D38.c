#include "ac1/ac1.h"
#include "m2c_compat.h"

extern s32 func_8006D534(void *owner, Projectile *p, void *vel, void *out);
extern void func_800835E8(void *owner, s32 hit, void *a, void *b, void *c);
extern Projectile *func_80086854(void *owner, void *pos, void *flags);
extern Projectile *func_80086BC4(void *owner, void *pos, s32 a, s32 b, s32 c, s32 d, s32 e);
extern Projectile *func_80086C9C(void *, void *, s32, s32, s32, s32, s32, s32, s32, s32, s32, s32);
extern void func_800873F8(s32 hit, void *pos, s32 a, s32 b, s32 c, s32 d);

#define TGT ((u8 *)p + 0x28)

/* homing missile think: fly, test for hits, and on the last frame or a hit
 * burst at the target point */
void func_80083D38(Projectile *p)
{
    SVECTOR rel;
    s32 hit;
    u32 bits;
    u32 bits2;
    s32 n;

    if (p->unk12 < 2) {
        if (M2C_FIELD(p, s16 *, 0x2E) == 0) {
            goto kill;
        }
        rel.vx = M2C_FIELD(p, s16 *, 0x28) - p->x;
        rel.vy = M2C_FIELD(p, s16 *, 0x2A) - p->y;
        rel.vz = M2C_FIELD(p, s16 *, 0x2C) - p->z;
        hit = func_8006D534(p->owner, p, &rel, TGT);
        if (hit != 0) {
            goto impact;
        }
    final:
        if (M2C_FIELD(p, s16 *, 0x2E) != 0) {
            if (M2C_FIELD(p, s16 *, 0x2E) == 2) {
                n = (M2C_FIELD(p, u32 *, 0x30) >> 17) & 0x3FF;
                func_80086C9C(0, TGT, 40, n, n * 4, n >> 4, n >> 2, -n >> 8, -n >> 3, n >> 2, 65, 0);
            } else {
                func_80086854(0, TGT, (u8 *)p + 0x30);
            }
            bits = M2C_FIELD(p, u32 *, 0x34);
            func_800873F8(M2C_FIELD(p, s32 *, 0x38), TGT, bits & 0x3FF, (bits >> 10) & 0xFF,
                          (bits >> 18) & 0x1F, 1);
            func_800835E8(p->owner, M2C_FIELD(p, s32 *, 0x38), p->dtint, (u8 *)p + 0x20, TGT);
        }
    kill:
        p->unk12 = 0;
        return;
    }
    hit = func_8006D534(p->owner, p, (u8 *)p + 0x20, TGT);
    if (hit != 0) {
    impact:
        bits2 = M2C_FIELD(p, u32 *, 0x34);
        func_800873F8(hit, TGT, bits2 & 0x3FF, (bits2 >> 10) & 0xFF, (bits2 >> 18) & 0x1F, 1);
        func_800835E8(p->owner, hit, p->dtint, (u8 *)p + 0x20, TGT);
        goto final;
    }
    func_80086BC4(0, p, (u16)p->ang[0], p->ang[1], p->ang[1] >> 8, p->vel[1], p->ang[1] >> 3);
    p->x += p->vel[2];
    p->y += p->unk22;
    p->z += p->spin[0];
    p->section = world_section_find(p->section, p);
}
