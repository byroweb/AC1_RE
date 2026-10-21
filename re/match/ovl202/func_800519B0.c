#include "ac1/ac1.h"

typedef struct {
    s16 unk0[4];
    s16 v;
} Rec10;

extern Rec10 D_8008DABC[];
/* D_8008DABC[3..1].v, written as scalars */
extern s16 D_8008DAE2;
extern s16 D_8008DAD8;
extern s16 D_8008DACE;
extern void func_800513EC(s32, s32, s32, void *);

void func_800519B0(s32 arg0, s32 arg1, s16 v) {
    D_8008DAE2 = v;
    D_8008DAD8 = v;
    D_8008DACE = v;
    D_8008DABC[0].v = v;
    func_800513EC(arg0, arg1, 4, D_8008DABC);
}
