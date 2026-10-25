/* ui_obj: queue a DR_MODE (texpage 7) primitive into the object's ordering table at depth unk14.
 * The identical if/else arms are a register lever: they put z/p/o in two basic blocks, so flow marks
 * them global and local-alloc cannot tie z to z<<16>>14 or p to p&0xffffff (retail's allocation);
 * jump2 cross-jumps the arms after reload and the sign-test branch disappears. */
#include "ac1/ac1.h"

typedef struct {
    u32 tag;
    u32 code[2];
} DR_MODE;

extern struct { u8 idx; } D_801EF6C8;      /* ordering table in use */
extern struct { u8 *prim; } D_801EF6CC;   /* primitive cursor */
extern void SetDrawMode(DR_MODE *p, s32 dfe, s32 dtd, s32 tpage, void *tw);

void func_80066898(UiObj *o)
{
    DR_MODE *p;
    s16 z;

    z = o->unk14;
    p = (DR_MODE *)D_801EF6CC.prim;
    D_801EF6CC.prim += sizeof(DR_MODE);
    SetDrawMode(p, 0, 0, 7, 0);
    if ((s32)p < 0)
        addPrim(o->ot[D_801EF6C8.idx] + z, p);
    else
        addPrim(o->ot[D_801EF6C8.idx] + z, p);
}
