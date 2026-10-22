#include "ac1/ac1.h"

extern u8 D_8004124A;
extern u8 D_8004124B;
typedef struct {
    u8 pad[0x20];
    SVECTOR lo;         /* 0x80040FD8 */
    SVECTOR hi;         /* 0x80040FE0 */
} AimState;

typedef struct {
    u16 vx, vy, vz, pad;
} UVec;

extern AimState D_80040FB8;
extern s16 D_80040FD8;
extern s16 D_80040FDA;
extern s16 D_80040FDC;
extern s16 D_80040FE0;
extern s16 D_80040FE2;
extern s16 D_80040FE4;

extern s32 func_800745D0(AcEntity *e, UVec *out, s32 a, s32 b, s32 c, s32 d, s32 f);
extern void func_8007A334(AcEntity *e, void *v, s32 x);

void func_8007AB04(AcEntity *e, s32 x)
{
    UVec a[2];
    SVECTOR b[2];

    if (func_800745D0(e, a, D_8004124A, D_8004124B, 2, 0, 1) == 0) {
        return;
    }
    a[0].vy = a[1].vy = e->pos.y - (*(u16 *)&e->unk28[2] >> 1);
    a[1].vx = a[0].vx + (a[1].vx - a[0].vx) * 2;
    a[1].vz = a[0].vz + (a[1].vz - a[0].vz) * 2;
    b[0].vx = (D_80040FD8 + (s16)a[0].vx) >> 1;
    b[0].vy = (D_80040FDA + (s16)a[0].vy) >> 1;
    b[0].vz = (D_80040FDC + (s16)a[0].vz) >> 1;
    b[1].vx = (D_80040FE0 + (s16)a[1].vx) >> 1;
    b[1].vy = (D_80040FE2 + (s16)a[1].vy) >> 1;
    b[1].vz = (D_80040FE4 + (s16)a[1].vz) >> 1;
    func_8007A334(e, b, x);
    func_8007A334(e, a, x);
    D_80040FB8.lo = *(SVECTOR *)&a[0];
    D_80040FB8.hi = *(SVECTOR *)&a[1];
}
