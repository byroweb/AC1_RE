#include "ac1/ac1.h"
#include "ac1/gfxframe.h"

extern void func_8004DCF8(void);

void func_8005715C(void) {
    GfxFrame *f;

    D_801A5E18.flip = D_801A5E18.flip == 0;
    f = &D_801A5E18.f[D_801A5E18.flip];
    D_801A5F1C.frame = f;
    D_801A5F1C.ctx.buf = &f->prim;
    D_801A5F1C.ctx.ot = f->ot;
    ClearOTagR(D_801A5F1C.ctx.ot, 0x2000);
    D_801A5F1C.ctx.buf->cur = D_801A5F1C.ctx.buf->base;
    D_801AC810.b = 0;
    D_801AC810.a = 0;
    func_8004DCF8();
}
