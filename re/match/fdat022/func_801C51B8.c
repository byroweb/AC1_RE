#include "ac1/ac1.h"

typedef struct {
    u8 pad0[0x1C];
    u16 f1c;
    u16 f1e;
    u8 f20;
    u8 f21;
} Part;
typedef struct {
    Part *part;         /* +0x00 */
    u8 pad4[0x32];
    u16 f36;
    u8 pad38[6];
    u8 f3e;
    u8 f3f;
    u8 f40;
    u8 pad41[0x1];
    s16 f12x;
    u8 pad44[4];
    s32 target;         /* +0x48 */
    u8 pad4c[0xFC];
    u16 f148;
    u8 f14a;
    u8 f14b;
    u8 f14c;
    u8 pad14d[0xF];
    s32 f15c;
} Ent;
extern s32 func_80073C2C();
extern s32 func_800745D0();
extern s32 func_800754D0();
extern void func_80077B78();
extern void func_80078E1C();
extern s32 func_801C50C8();
extern s32 rand();
extern u8 D_801C7468[];
extern u8 D_801C748C[];
extern u8 D_801C74AC[];

void func_801C51B8(Ent *e)
{
    union { s8 b[0x38]; s32 w; } sp_frame;
    u8 t;
    u8 nv;
    u8 q;
    Part *p;

    e->f148 = 0;
    e->f15c = 0;
    if (e->f40 != 0xFF) {
        if (func_800754D0(e, D_801C7468) != 0) {
            func_80078E1C(e);
        }
    } else {
        if (rand() < 0x148 && func_800745D0(e, sp_frame.b + 0x30, 0, 9, 1, 1) != 0 && func_80073C2C(e->target, sp_frame.b + 0x30, *(s16 *)((u8 *)e + 0x12), 0, 8, e->f36, 0x2710, -800, 800, -500, 500, sp_frame.b + 0) != 0 && (*(u16 *)(sp_frame.b + 0x16) & 2)) {
            p = e->part;
            func_80077B78(e, 0x17, p->f1e, p->f21, D_801C748C, 1, 0, 1, 0, -256, e->target);
            p = e->part;
            func_80077B78(e, 0x17, p->f1e, p->f21, D_801C748C, 1, 2, 3, 0, 256, e->target);
            e->f15c = 0x200;
        }
        t = e->f3f;
        if (t == 0) {
            if (func_801C50C8(e) != 0) {
                if (e->f3e == 0) {
                    if (rand() < 0x290) {
                        e->f3e = 1;
                    }
                } else {
                    p = e->part;
                    func_80077B78(e, 0x27, p->f1c, p->f20, D_801C74AC, 1, 0, 1);
                    q = e->f3e;
                    e->f148 = 0xF;
                    e->f14a = 0xB4;
                    e->f14b = 0xA0;
                    e->f14c = 0x8C;
                    e->f15c = 0x100;
                    e->f3e = q + 0xFF;
                    if (q == 0) {
                        nv = 10;
                        goto blk15;
                    }
                }
            }
        } else {
            nv = t - 1;
        blk15:
            e->f3f = nv;
        }
    }
}
