#include "ac1/ac1.h"

typedef struct {
    s16 pitch;
    s16 yaw;
} Ang;

extern int abs(int);
extern void func_80014AE4(s16, s16, s16, Ang *);
extern void func_80075BC0(AcEntity *, s16 *);
extern void func_80078598(AcEntity *);
extern void func_801C5384(AcEntity *, s32, s32, s32, void *, void *);
extern void func_801C55C0(void);

void func_801C5844(AcEntity *e, s32 st)
{
    s16 v[3];
    Ang ang;
    s16 a;
    s16 b;
    s32 k;

    if (e->faction & 0x10) {
        st = e->unk3a[2];
    }
    func_80078598(e);
    switch (st) {
    case 0:
        func_80075BC0(e, v);
        func_80014AE4(v[0], v[1], v[2], &ang);
        if (ang.pitch > 2048) {
            ang.pitch -= 4096;
        }
        ang.yaw = ang.yaw - e->yaw;
        if (ang.yaw > 2048) {
            ang.yaw -= 4096;
        }
        a = abs(ang.pitch);
        b = abs(ang.yaw);
        if (a < 32 && b < 64) {
            k = 0;
        } else if (a > b) {
            k = 1;
            if (ang.pitch > 0) {
                k = 2;
            }
        } else {
            k = 4;
            if (ang.yaw > 0) {
                k = 3;
            }
        }
        func_801C5384(e, k, 1, 512, (u8 *)e + 0xCC, func_801C55C0);
        break;
    case 0x81:
        func_801C5384(e, e->mover.mode, 0, 512, (u8 *)e + 0xCC, func_801C55C0);
        break;
    }
    if (!(e->faction & 0x10)) {
        e->unk3a[2] = st;
    }
}
