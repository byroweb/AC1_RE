#include "ac1/ac1.h"
#include "m2c_compat.h"

typedef struct {
    s32 x0, y0, z0;
    s32 x1, y1, z1;
} Seg32;

typedef struct {
    u8 unk0[0x0E];
    u16 flags;                  /* +0x0E bit 0: hard surface */
} HitObj;

typedef struct {
    HitObj *obj;
    s32 hit;
} HitRec;

extern s32 func_8006D534(void *owner, Projectile *p, void *vel, void *out);
extern HitRec *func_800705D8(Seg32 *seg, void *out, s32 n);
extern void func_800835E8(void *owner, s32 hit, void *a, void *b, void *c);
extern Projectile *func_80086854(void *owner, void *pos, void *flags);
extern Projectile *func_80086C9C(void *, void *, s32, s32, s32, s32, s32, s32, s32, s32, s32, s32);
extern void func_800873F8(s32 hit, void *pos, s32 a, s32 b, s32 c, s32 d);

/* falling shell: hit test, gravity, world test, then impact effects */
void func_80087BC8(Projectile *p)
{
    SVECTOR prev;
    Seg32 seg;
    HitRec *h;
    s32 hit;
    u32 bits;

    hit = func_8006D534(p->owner, p, (u8 *)p + 0x1E, (u8 *)p + 0x26);
    if (hit == 0) {
        prev = *(SVECTOR *)p;
        if ((p->vel[2] += 8) > 500) {
            p->vel[2] = 500;
        }
        p->x += p->vel[1];
        p->y += p->vel[2];
        p->z += p->unk22;
        p->section = world_section_find(p->section, p);
        seg.x0 = prev.vx;
        seg.y0 = prev.vy;
        seg.z0 = prev.vz;
        seg.x1 = p->x;
        seg.y1 = p->y;
        seg.z1 = p->z;
        h = func_800705D8(&seg, (u8 *)p + 0x26, 6);
        if (h == 0) {
            return;
        }
        if (h->obj->flags & 1) {
            M2C_FIELD(p, s16 *, 0x2C) = 2;
        } else {
            M2C_FIELD(p, s16 *, 0x2C) = 1;
        }
        hit = h->hit;
    }
    bits = M2C_FIELD(p, u32 *, 0x34);
    func_800873F8(hit, (u8 *)p + 0x26, bits & 0x3FF, (bits >> 10) & 0xFF, (bits >> 18) & 0x1F, 1);
    func_800835E8(p->owner, hit, p->dtint, (u8 *)p + 0x1E, (u8 *)p + 0x26);
    if (M2C_FIELD(p, s16 *, 0x2C) == 2) {
        func_80086C9C(0, (u8 *)p + 0x26, 40, 192, 768, 12, 48, -1, -24, 48, 65, 0);
    }
    if (M2C_FIELD(p, s16 *, 0x2C) == 1) {
        func_80086854(0, (u8 *)p + 0x26, (u8 *)p + 0x30);
    }
    p->unk12 = 0;
}
