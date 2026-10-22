#include "ac1/ac1.h"

typedef struct {
    u8 pad[0x0E];
    s16 h0e;
} FloorHit;

typedef struct {
    SVECTOR lo;
    SVECTOR hi;
} Box16;

extern s32 D_801A5DC8;
extern FloorHit *func_80071DC4(Box16 *box, s32 kind);

FloorHit *func_80075030(AcEntity *e, s32 h)
{
    Box16 box;
    FloorHit *hit;
    u16 *ext;

    ext = (u16 *)((u8 *)e + 0x6A);
    box.lo.vx = e->pos.x - (*ext >> D_801A5DC8);
    box.hi.vx = e->pos.x + (*ext >> D_801A5DC8);
    box.lo.vz = e->pos.z - (*ext >> D_801A5DC8);
    box.hi.vz = e->pos.z + (*ext >> D_801A5DC8);
    box.lo.vy = e->pos.y - *(u16 *)((u8 *)e + 0x2A);
    box.hi.vy = e->pos.y + h;
    hit = func_80071DC4(&box, *((u8 *)e + 0x3A));
    if (hit != 0 && hit->h0e < e->pos.y) {
        hit->h0e = e->pos.y;
    }
    return hit;
}
