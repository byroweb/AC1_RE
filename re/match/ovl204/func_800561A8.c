#include "ac1/ac1.h"
#include "ac1/gfxframe.h"

typedef struct {                /* 0x13C: ovl204 display buffer */
    u32 *ot;                    /* +0x00 */
    u32 *ot2;                   /* +0x04 */
    DRAWENV draw;               /* +0x08 */
    u8 pad64[0x11C - 0x64];
    DISPENV disp;               /* +0x11C */
    PrimBuf prim;               /* +0x130 */
} Gfx4Frame;

typedef struct {                /* 0x801ADA48 */
    s32 flip;
    Gfx4Frame f[2];             /* 0x801ADA4C */
} Gfx4FrameSet;

typedef struct {                /* 0x801ADCC4 */
    Gfx4Frame *frame;
    u8 *heap;
    PrimBuf *buf;               /* +0x08 */
} Gfx4State;

typedef struct {                /* 0x801B4108 */
    s32 rcnt;
    s32 a;
    s32 b;
} Gfx4Counters;

extern Gfx4FrameSet D_801ADA48;
extern Gfx4State D_801ADCC4;
extern Gfx4Counters D_801B4108;

extern void func_8004DDF8(void);

void func_800561A8(void) {
    Gfx4Frame *f;

    D_801ADA48.flip = D_801ADA48.flip == 0;
    f = &D_801ADA48.f[D_801ADA48.flip];
    D_801ADCC4.frame = f;
    D_801ADCC4.buf = &f->prim;
    ClearOTagR(f->ot, 0x2000);
    ClearOTagR(D_801ADCC4.frame->ot2, 0x2000);
    D_801ADCC4.buf->cur = D_801ADCC4.buf->base;
    D_801B4108.b = 0;
    D_801B4108.a = 0;
    func_8004DDF8();
}
