#include "ac1/ac1.h"
#include "m2c_compat.h"

extern s32 func_8006D534(void *owner, Projectile *p, void *vel, void *out);
extern void func_800835E8(void *owner, s32 hit, void *a, void *b, void *c);
extern Projectile *func_80086854(void *owner, void *pos, void *flags);
extern Projectile *func_80086C9C(void *, void *, s32, s32, s32, s32, s32, s32, s32, s32, s32, s32);
extern void func_800873F8(s32 hit, void *pos, s32 a, s32 b, s32 c, s32 d);

#define TGT ((u8 *)p + 0x28)

/* func_80083D38 variant: +0x26 counts a launch delay (>= 0x100 while the
 * missile is still on the rail), and the burst moves the shell onto the
 * target point first */
void func_80083A10(Projectile *p)
{
    SVECTOR rel;
    s32 hit;
    u32 bits;
    u32 bits2;
    s32 n;

    if (p->spin[1] < 0x100) {
        if (p->unk12 < 2) {
            if (M2C_FIELD(p, s16 *, 0x2E) == 0) {
                goto done;
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
                *(SVECTOR *)p = *(SVECTOR *)TGT;
                p->section = world_section_find(p->section, p);
                bits = M2C_FIELD(p, u32 *, 0x34);
                func_800873F8(M2C_FIELD(p, s32 *, 0x38), TGT, bits & 0x3FF, (bits >> 10) & 0xFF,
                              (bits >> 18) & 0x1F, 1);
                func_800835E8(p->owner, M2C_FIELD(p, s32 *, 0x38), p->dtint, (u8 *)p + 0x20, TGT);
            }
        done:
            p->unk12 = p->vel[1] - p->spin[1];
            p->spin[1] = p->unk12 + 0x100;
            goto tail;
        }
        hit = func_8006D534(p->owner, p, (u8 *)p + 0x20, TGT);
        if (hit != 0) {
        impact:
            bits2 = M2C_FIELD(p, u32 *, 0x34);
            func_800873F8(hit, TGT, bits2 & 0x3FF, (bits2 >> 10) & 0xFF, (bits2 >> 18) & 0x1F, 1);
            func_800835E8(p->owner, hit, p->dtint, (u8 *)p + 0x20, TGT);
            goto final;
        }
        if (p->spin[1] == 0) {
            p->ang[0] += p->vel[2];
            p->ang[1] += p->unk22;
            p->ang[2] += p->spin[0];
        } else {
            p->spin[1]--;
        }
        p->x += p->vel[2];
        p->y += p->unk22;
        p->z += p->spin[0];
        p->section = world_section_find(p->section, p);
        return;
    }
tail:
    if (p->spin[1] > 0x100) {
        p->ang[0] += p->vel[2];
        p->ang[1] += p->unk22;
        p->ang[2] += p->spin[0];
        p->spin[1]--;
    }
}
