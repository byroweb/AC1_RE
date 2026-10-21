#include "ac1/ac1.h"

typedef struct {
    s16 shape;                  /* +0x00 index into D_8008ED68 */
    s16 arg;                    /* +0x02 */
    u16 dx;                     /* +0x04 */
    u16 dy;                     /* +0x06 */
    s16 color;                  /* +0x08 index into D_8008DA1C */
} HudPart;

typedef struct {
    u8 unk0[12];
} HudShape;

extern HudShape D_8008ED68[];
extern s32 D_8008DA1C[];
extern void func_80060B54(DVECTOR *, HudShape *, s32, s32 *, s32, s32);

void func_800513EC(s32 x, s32 y, s32 n, HudPart *part) {
    DVECTOR p;

    while (--n != -1) {
        p.vx = part->dx + x;
        p.vy = part->dy + y;
        func_80060B54(&p, &D_8008ED68[part->shape], part->arg, &D_8008DA1C[part->color], 16, 2);
        part++;
    }
}
