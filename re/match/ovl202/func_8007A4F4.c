#include "ac1/ac1.h"

extern u8 D_8004124A;
extern u8 D_8004124B;
typedef struct {
    u8 pad[0x20];
    SVECTOR lo;         /* 0x80040FD8 */
    SVECTOR hi;         /* 0x80040FE0 */
} AimState;

extern AimState D_80040FB8;
extern s16 D_80040FD8;
extern s16 D_80040FDA;
extern s16 D_80040FDC;
extern s16 D_80040FE0;
extern s16 D_80040FE2;
extern s16 D_80040FE4;

extern s32 func_800745D0(AcEntity *e, SVECTOR *out, s32 a, s32 b, s32 c, s32 d, s32 f);
extern void func_8007A334(AcEntity *e, SVECTOR *v, s32 x);

void func_8007A4F4(AcEntity *e, s32 x)
{
    SVECTOR a[2];
    SVECTOR b[2];

    if (func_800745D0(e, a, D_8004124A, D_8004124B, 2, 0, 1) == 0) {
        return;
    }
    b[0].vx = (D_80040FD8 + a[0].vx) >> 1;
    b[0].vy = (D_80040FDA + a[0].vy) >> 1;
    b[0].vz = (D_80040FDC + a[0].vz) >> 1;
    b[1].vx = (D_80040FE0 + a[1].vx) >> 1;
    b[1].vy = (D_80040FE2 + a[1].vy) >> 1;
    b[1].vz = (D_80040FE4 + a[1].vz) >> 1;
    func_8007A334(e, b, x);
    func_8007A334(e, a, x);
    D_80040FB8.lo = a[0];
    D_80040FB8.hi = a[1];
}
