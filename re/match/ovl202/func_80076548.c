#include "ac1/ac1.h"

typedef struct {
    s32 w[4];
} Quad;

typedef struct {
    u8 pad00[0x3D];
    u8 sub3d;           /* +0x3D */
    u8 pad3e[0x0E];
    void *fn4c;         /* +0x4C */
    u8 pad50[0x8C];
    Quad save;          /* +0xDC */
    Quad cur;           /* +0xEC */
} AcMoveCtx;

void func_80076548(AcMoveCtx *e, u8 sub, void *fn)
{
    e->sub3d = sub;
    e->fn4c = fn;
    e->save = e->cur;
}
