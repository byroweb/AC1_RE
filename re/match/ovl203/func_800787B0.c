#include "ac1/ac1.h"
#include "ac1/vsplayer.h"

/* ovl203 copy of ovl202 func_80079CDC: falling debris step, speed capped by the
 * current player's speed_cap. */
typedef struct Func800787B0Obj Func800787B0Obj;
struct Func800787B0Obj {
    u8 pad0[8];
    u8 pos[10];         /* +0x08 */
    s16 y;              /* +0x12 */
    u8 pad14[0x41 - 0x14];
    u8 arg;             /* +0x41 */
    u8 pad42[0x50 - 0x42];
    void (*cb)(Func800787B0Obj *, u8); /* +0x50 */
    u8 pad54[0x66 - 0x54];
    u8 timer;           /* +0x66 */
    u8 pad67[0xCC - 0x67];
    s16 speed;          /* +0xCC */
    s16 accel;          /* +0xCE */
};

extern struct { u8 *base; } D_8019D838;
extern void func_8004CE38(int, void *, void *, int, int, int, int);

void func_800787B0(Func800787B0Obj *o) {
    if (--o->timer == 0) {
        if (o->cb != 0) {
            o->cb(o, o->arg);
        }
    }
    o->speed += o->accel;
    if (o->speed > D_80039D18->speed_cap) {
        o->speed = D_80039D18->speed_cap;
    }
    o->y += o->speed;
    func_8004CE38(D_80039D18->ctx->id, D_8019D838.base + 0x1200, o->pos, 40, 8000, 10000, 0);
}
