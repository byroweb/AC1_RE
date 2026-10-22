#include "ac1/ac1.h"
#include "ac1/gfxframe.h"

typedef struct {                /* 0x13C: ovl204 display buffer */
    u32 *ot;                    /* +0x00 */
    u32 *ot2;                   /* +0x04 */
    DRAWENV draw;               /* +0x08 */
    DRAWENV draw2;              /* +0x64 */
    DRAWENV draw3;              /* +0xC0 */
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

extern Gfx4FrameSet D_801ADA48;
extern Gfx4State D_801ADCC4;
extern u8 D_8013E960[];
extern u32 D_80184960[];
extern u32 D_8018C960[];
extern u32 D_80194960[];
extern u32 D_8019C960[];
extern u8 D_8009D348[];

extern void func_8001604C(void *, s32);

void func_8005631C(void) {
    D_801ADCC4.heap = D_8013E960;
    D_801ADA48.f[0].prim.base = D_801ADCC4.heap;
    D_801ADA48.f[0].prim.end = D_801ADA48.f[0].prim.base + 0x21000;
    D_801ADA48.f[1].prim.base = D_801ADA48.f[0].prim.base + 0x23000;
    D_801ADA48.f[1].prim.end = D_801ADA48.f[1].prim.base + 0x21000;
    D_801ADA48.f[0].ot = D_80184960;
    D_801ADA48.f[1].ot = D_8018C960;
    D_801ADA48.f[0].ot2 = D_80194960;
    D_801ADA48.f[1].ot2 = D_8019C960;
    D_801ADA48.f[0].draw.dtd = D_801ADA48.f[1].draw.dtd = 1;
    D_801ADA48.f[0].draw2.dtd = D_801ADA48.f[1].draw2.dtd = 1;
    D_801ADA48.f[0].draw3.dtd = D_801ADA48.f[1].draw3.dtd = 0;
    D_801ADA48.f[0].draw2.isbg = D_801ADA48.f[1].draw2.isbg = 1;
    D_801ADA48.f[0].draw.isbg = D_801ADA48.f[1].draw.isbg = 1;
    D_801ADA48.f[0].draw3.isbg = D_801ADA48.f[1].draw3.isbg = 0;
    func_8001604C(D_8009D348, 0x10000);
}
