#include "ac1/ac1.h"
#include "ac1/vsplayer.h"

typedef struct Func8007843CObj Func8007843CObj;
struct Func8007843CObj {
    u8 pad0[8];
    u8 pos[2];          /* +0x08 */
    s16 f0A;            /* +0x0A */
    u8 pad0C[0x2A - 0xC];
    u16 f2A;            /* +0x2A */
    u8 pad2C[2];
    s16 f2E;            /* +0x2E */
    u8 pad30[0x41 - 0x30];
    u8 arg;             /* +0x41 */
    u8 pad42[0x50 - 0x42];
    void (*cb)(Func8007843CObj *, u8); /* +0x50 */
    u8 pad54[0x64 - 0x54];
    u8 anim;            /* +0x64 */
    u8 pad65[0x6E - 0x65];
    s16 frame;          /* +0x6E */
};

extern void func_80078258(Func8007843CObj *, s32, s32);
extern void func_800771CC(Func8007843CObj *, s32, s32, s32, s32, s32, s32, s32);
extern void func_80076FA0(Func8007843CObj *, s32, s32, s32, s32, s32, s32, void *);
extern s32 func_8004F240(void *, void *);

/* Walking actor step: turn, footstep effects on the contact frames while
 * grounded, then animate and run the state handler. */
void func_8007843C(Func8007843CObj *o) {
    func_80078258(o, D_80039D18->unk214, D_80039D18->speed_cap);
    if (o->f2E <= o->f0A && o->f2E >= o->f0A - o->f2A) {
        if (o->anim == 22) {
            if (o->frame == 6) {
                func_800771CC(o, 100, 5, 300, 700, 100, 300, 64);
            }
        } else if (o->frame == D_80039D18->ctx->slot1 + 1) {
            func_80076FA0(o, D_80039D18->fx[0], D_80039D18->fx[1], 0, 80, 200, 65, 0);
        } else if (o->frame == D_80039D18->ctx->slot0 + 1) {
            func_80076FA0(o, D_80039D18->fx[2], D_80039D18->fx[3], 0, 80, 200, 65, 0);
        }
    }
    if (func_8004F240(&o->anim, o->pos) == 0) {
        if (o->cb != 0) {
            o->cb(o, o->arg);
        }
    }
}
