#include "ac1/ac1.h"

typedef struct {
    u8 b[8];
} Bytes8;
typedef struct {
    u8 pad0[0x16];
    u16 flags;          /* +0x16 */
    s16 yaw;            /* +0x18 */
    s16 pitch;          /* +0x1A */
    u8 pad1c[0x14];
} Hit;                  /* 0x30 */
typedef struct {
    s32 w[2];
} Buf8;

extern Bytes8 D_80039C78;
extern s32 rand(void);
extern s16 func_800132CC(s16, s16, s32);
extern s32 func_800745D0(AcEntity *, void *, s32, s32, s32, s32);
extern s32 func_80073C2C(AcEntity *, void *, s16, s32, s32, s32, s32, s32, s32, s32, s32, Hit *);

s32 func_801C513C(AcEntity *e)
{
    Hit hit;
    Buf8 buf;

    if (func_800745D0(e, &buf, 0, 0, 1, 1) != 0
        && func_80073C2C(e->target, &buf, e->yaw, 0, 0, e->faction, 10000, -1024, 1024, -1400, 1400, &hit) != 0
        && (hit.flags & 2)) {
        if (e->unk42[0] == 2) {
            hit.yaw -= rand() & 0xF;
            hit.pitch += (rand() & 0x1F) - 16;
        }
        *(s16 *)((u8 *)e + 0x76) = func_800132CC(*(s16 *)((u8 *)e + 0x76), hit.yaw, 16);
        *(s16 *)((u8 *)e + 0x78) = func_800132CC(*(s16 *)((u8 *)e + 0x78), -hit.pitch, 16);
        return 1;
    }
    *(Bytes8 *)((u8 *)e + 0x76) = D_80039C78;
    return 0;
}
