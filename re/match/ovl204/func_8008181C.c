#include "ac1/ac1.h"
#include "ac1/ovl202_camera.h"

/* per-player record (0x39C, array at 0x80040FA0); D_80039D18 = current */
typedef struct {
    u8 unk0[0x24];
    s32 unk24;                  /* +0x24 (0x80040FC4 for player 0) */
    u8 unk28[0x58];
    TargetSlot *target;         /* +0x80 lock list head */
    u8 unk84[0x254 - 0x84];
    u32 flags;                  /* +0x254 (0x800411F4) */
    u8 unk258[0x398 - 0x258];
    CamParm *cam;               /* +0x398 */
} Player;

extern Player *D_80039D18;
typedef struct {
    u32 flags;
} MFlags;

extern MFlags D_801AB684;       /* g_mission_flags, read as a struct field here */
extern void func_8008166C(void);
extern void func_80014774(SVECTOR *ang, SVECTOR *dir);

/* Camera eye (o[0..2]) and target (o[3..5]) from a position and view angles. */
void func_8008181C(SVECTOR *pos, SVECTOR *ang, s32 *o, SVECTOR *dir) {
    SVECTOR d;
    SVECTOR a;

    o[0] = pos->vx;
    o[1] = pos->vy;
    o[2] = pos->vz;
    if (!(D_801AB684.flags & 0x2000)) {
        func_8008166C();
    }
    a.vx = ang->vx + D_80039D18->cam->ox;
    a.vy = ang->vy + D_80039D18->cam->oy - 2048;
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
    d.vx = (d.vx * D_80039D18->cam->zoom) >> 12;
    d.vy = (d.vy * D_80039D18->cam->zoom) >> 12;
    d.vz = (d.vz * D_80039D18->cam->zoom) >> 12;
    o[3] = o[0] + (d.vx >> 1);
    o[5] = o[2] + (d.vz >> 1);
    o[4] = o[1] + (d.vy >> 1) - D_80039D18->cam->height;
    *dir = d;
}
