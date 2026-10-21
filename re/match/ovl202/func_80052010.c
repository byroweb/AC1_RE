#include "ac1/ac1.h"

typedef struct {
    s16 unk0[4];
    s16 v;
} Rec10;

extern Rec10 D_8008DB84[];
extern s16 D_8008DB96; /* D_8008DB84[1].v, written as a scalar */
extern void func_800513EC(s32, s32, s32, void *);

void func_80052010(s32 arg0, s32 arg1, s16 v) {
    D_8008DB96 = v;
    D_8008DB84[0].v = v;
    func_800513EC(arg0, arg1, 2, D_8008DB84);
}
