#include "ac1_types.h"
/* ovl202 0x80079CDC  size 212 */
typedef struct Func80079CDCObj Func80079CDCObj;
struct Func80079CDCObj {
    u8 pad0[8];
    u8 pos[10];         /* +0x08 */
    s16 y;              /* +0x12 */
    u8 pad14[0x41 - 0x14];
    u8 arg;             /* +0x41 */
    u8 pad42[0x50 - 0x42];
    void (*cb)(Func80079CDCObj *, u8); /* +0x50 */
    u8 pad54[0x66 - 0x54];
    u8 timer;           /* +0x66 */
    u8 pad67[0xCC - 0x67];
    s16 speed;          /* +0xCC */
    s16 accel;          /* +0xCE */
};
typedef struct {
    u8 pad[0x11];
    u8 id;              /* +0x11 */
} Func80079CDCCtx;
extern u16 D_800411B6;
extern struct { Func80079CDCCtx *ctx; } D_80040FEC;
extern struct { u8 *base; } D_801A5DC0;
extern void func_8004CC30(int, void *, void *, int, int, int, int);

void func_80079CDC(Func80079CDCObj *o)
{
    if (--o->timer == 0) {
        if (o->cb != 0) {
            o->cb(o, o->arg);
        }
    }
    o->speed += o->accel;
    if (o->speed > D_800411B6) {
        o->speed = D_800411B6;
    }
    o->y += o->speed;
    func_8004CC30(D_80040FEC.ctx->id, D_801A5DC0.base + 0x1200, o->pos, 40, 8000, 10000, 0);
}
