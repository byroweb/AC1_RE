#include "ac1/ac1.h"

typedef struct {
    u8 pad[0xCC];
    s16 hcc;
    s16 hce;
    s16 hd0;
} AcMoveArgs;

extern void func_80076548(AcEntity *e, s32 sub, void *fn);
extern void func_8004E98C(AcMover *m, s32 a, s32 b, s32 c);

void func_80076658(AcEntity *e, s32 a, s32 b, s32 sub, s32 p, s32 q, void *fn)
{
    func_80076548(e, sub, fn);
    func_8004E98C(&e->mover, a, b, 512);
    ((AcMoveArgs *)e)->hcc = 0;
    ((AcMoveArgs *)e)->hce = p;
    ((AcMoveArgs *)e)->hd0 = q;
    ((u8 *)&e->mover)[2] = 2;
}
