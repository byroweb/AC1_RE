#include "ac1/ac1.h"
#include "m2c_compat.h"

typedef struct {
    s16 a;
    u16 b;
    u8 c;
    u8 d;
} Shard6;

extern s32 func_80014D78(s32 range);
extern s32 func_8006D534(void *owner, Projectile *p, void *vel, void *out);
extern void func_800835E8(void *owner, s32 hit, void *a, void *b, void *c);
extern Projectile *func_8008589C(void *owner, void *pos, s32 a, s32 b, s32 c, s32 section);
extern Projectile *func_80086854(void *owner, void *pos, u32 *flags);
extern void func_800873F8(s32 hit, void *pos, s32 a, s32 b, s32 c, s32 d);
extern void func_80087798(void *owner, void *pos, Shard6 *s, s32 n, s32 e);
extern u32 D_80090794;

#define TGT ((u8 *)p + 0x26)

void func_80084EE0(Projectile *p)
{
    SVECTOR unused;     /* retail frame slot, never touched */
    AcPos at;
    Shard6 sh;
    u32 fl;
    s32 hit;
    u32 bits;
    u32 bits2;
    s32 i;
    s32 r;

    p->ang[2] += 128;
    if (p->unk12 != 0 || M2C_FIELD(p, s16 *, 0x2C) == 0) {
        hit = func_8006D534(p->owner, p, (u8 *)p + 0x1E, TGT);
        if (hit == 0) {
            goto move;
        }
        bits = M2C_FIELD(p, u32 *, 0x34);
        func_800873F8(hit, TGT, bits & 0x3FF, (bits >> 10) & 0xFF, (bits >> 18) & 0x1F, 1);
        func_800835E8(p->owner, hit, p->dtint, (u8 *)p + 0x1E, TGT);
        bits2 = M2C_FIELD(p, u32 *, 0x34);
        func_800873F8(M2C_FIELD(p, s32 *, 0x38), TGT, bits2 & 0x3FF, (bits2 >> 10) & 0xFF,
                      (bits2 >> 18) & 0x1F, 1);
        sh = *(Shard6 *)p->dtint;
        sh.b >>= 3;
        sh.c >>= 2;
        func_80087798(0, TGT, &sh, 5, 1);
    }
    fl = M2C_FIELD(p, u32 *, 0x30);
    func_80086854(0, p, &fl);
    fl |= 0xF8001C00;
    for (i = 4; i != 0; i--) {
        at.x = p->x + func_80014D78(1000) - 500;
        at.y = p->y + func_80014D78(1000) - 500;
        r = func_80014D78(1000);
        fl = (fl & 0xFFFE1FFF) | (((i * 2) & 0xF) << 13);
        at.z = p->z + r - 500;
        func_80086854(0, &at, &fl);
    }
    p->unk12 = 0;
    return;
move:
    p->x += p->vel[1];
    p->y += p->vel[2];
    p->z += p->unk22;
    p->section = world_section_find(p->section, p);
    if (p->unkbe != 0) {
        func_8008589C(0, p, 36, 180, 100, p->section)->unk12 = 0;
        func_8008589C(0, p, 36, 220, 100, p->section)->unk12 = 0;
    } else if (!(p->unk12 & 3)) {
        func_80086854(0, p, &D_80090794);
    }
}
