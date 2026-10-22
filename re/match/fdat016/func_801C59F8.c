#include "ac1/ac1.h"

typedef struct {
    u8 b[8];
} Bytes8;
typedef struct {
    u8 pad0[0x10];
    u16 px;             /* +0x10 */
    u16 py;             /* +0x12 */
    u16 pz;             /* +0x14 */
    u16 flags;          /* +0x16 */
    s16 yaw;            /* +0x18 */
    s16 pitch;          /* +0x1A */
    u8 pad1c[0x14];
} Hit;                  /* 0x30 */
typedef struct {
    s32 w[2];
} Buf8;
typedef struct {
    u16 yaw;
    s16 pitch;
} Ang;

extern Bytes8 D_80039C78;
extern void func_80015258(Buf8 *, void *, s32, u16 *, void *, Buf8 *, Ang *, s32);
extern s32 func_800745D0(AcEntity *, void *, s32, s32, s32, s32);
extern s32 func_80073C2C(AcEntity *, void *, s16, s32, s32, s32, s32, s32, s32, s32, s32, Hit *);

s32 func_801C59F8(AcEntity *e)
{
    Hit hit;
    Buf8 buf;
    AcPos d;
    Buf8 o70;
    Ang o78;

    if (func_800745D0(e, &buf, 0, 0, 1, 1) != 0
        && func_80073C2C(e->target, &buf, e->yaw, 0, 0, e->faction, 10000, -128, 1024, -1024, 1024, &hit) != 0
        && (hit.flags & 2)) {
        d.x = hit.px - *(u16 *)((u8 *)e + 0x108);
        d.y = hit.py - *(u16 *)((u8 *)e + 0x10A);
        d.z = hit.pz - *(u16 *)((u8 *)e + 0x10C);
        *(Bytes8 *)((u8 *)e + 0x108) = *(Bytes8 *)&hit.px;
        func_80015258(&buf, (u8 *)e + 0x10, 900, &hit.px, &d, &o70, &o78, 3);
        *(u16 *)((u8 *)e + 0x76) = o78.yaw;
        *(s16 *)((u8 *)e + 0x78) = -o78.pitch;
        return 1;
    }
    *(Bytes8 *)((u8 *)e + 0x76) = D_80039C78;
    return 0;
}
