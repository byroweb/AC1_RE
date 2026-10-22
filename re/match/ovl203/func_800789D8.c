#include "ac1/ac1.h"
#include "ac1/vsplayer.h"

typedef struct Func800789D8Obj Func800789D8Obj;
struct Func800789D8Obj {
    u8 pad0[8];
    u8 pos[8];          /* +0x08 */
    u8 pad10[0x41 - 0x10];
    u8 arg;             /* +0x41 */
    u8 pad42[0x50 - 0x42];
    void (*cb)(Func800789D8Obj *, u8); /* +0x50 */
    u8 pad54[0x64 - 0x54];
    u8 mover[0xCC - 0x64]; /* +0x64 */
    s16 vx;             /* +0xCC */
    s16 padCE;
    s16 vz;             /* +0xD0 */
};

extern struct { u8 *base; } D_8019D838;
extern void func_80078258(Func800789D8Obj *, s32, s32);
extern void func_8004CE38(int, void *, void *, int, int, int, int);
extern s32 func_8004F29C(void *, void *);

/* Turning actor step: turn, effect while a turn button is held, brake the
 * velocity, then move and run the state handler. */
void func_800789D8(Func800789D8Obj *o) {
    func_80078258(o, D_80039D18->turn_acc, D_80039D18->turn_max);
    if (D_80039D18->pad_cur & (D_80039D18->turn_l | D_80039D18->turn_r)) {
        func_8004CE38(D_80039D18->ctx->id, D_8019D838.base + 0x1200, o->pos, 40, 8000, 10000, 0);
    }
    o->padCE = 0;
    if (abs(o->vx) <= 512) {
        o->vx = 0;
    } else {
        o->vx -= o->vx >> 3;
    }
    if (abs(o->vz) <= 512) {
        o->vz = 0;
    } else {
        o->vz -= o->vz >> 3;
    }
    if (func_8004F29C(o->mover, o->pos) == 0) {
        if (o->cb != 0) {
            o->cb(o, o->arg);
        }
    }
}
