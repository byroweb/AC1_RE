#include "ac1/ac1.h"
#include "ac1/ovl202_camera.h"

typedef struct {
    u32 flags;
} MFlags;

extern MFlags D_8019F524;       /* g_mission_flags, read as a struct field here */
extern void func_80082D04(void);
extern void func_80014774(SVECTOR *ang, SVECTOR *dir);

/* Camera eye (o[0..2]) and target (o[3..5]) from a position and view angles. */
void func_80082E7C(SVECTOR *pos, SVECTOR *ang, s32 *o, SVECTOR *dir) {
    SVECTOR d;
    SVECTOR a;

    o[0] = pos->vx;
    o[1] = pos->vy;
    o[2] = pos->vz;
    if (!(D_8019F524.flags & 0x2000)) {
        func_80082D04();
    }
    a.vx = ang->vx + D_801987D0.ox;
    a.vy = ang->vy + D_801987D0.oy - 2048;
    if (a.vx >= 2048) {
        a.vx -= 4096;
    }
    if (a.vx > 800) {
        a.vx = 800;
    }
    if (a.vx < -800) {
        a.vx = -800;
    }
    func_80014774(&a, &d);
    d.vx = (d.vx * D_801987D0.zoom) >> 12;
    d.vy = (d.vy * D_801987D0.zoom) >> 12;
    d.vz = (d.vz * D_801987D0.zoom) >> 12;
    o[3] = o[0] + (d.vx >> 1);
    o[5] = o[2] + (d.vz >> 1);
    o[4] = o[1] + (d.vy >> 1) - D_801987D0.height;
    *dir = d;
}
