#include "ac1/ac1.h"
#include "ac1/ovl202_t42.h"

extern DrawCtx D_801A5F24;
extern u32 D_80090850;
extern u8 D_80090874[4];
extern u8 D_80090878[4];
extern DVECTOR D_8009086C;
extern DVECTOR D_80090870;
extern SprInfo D_80090854;
extern SprInfo D_80090860;
extern u8 D_80041218;
extern void func_80060B54(DVECTOR *xy, SprInfo *spr, s32 rot, u32 *rgb, s32 z, s32 abr);

/* Pulsing colour between two CVECTORs, then one or two sprites with it. */
void func_8008D288(void) {
    u32 rgb;
    T42Spad *sp = SPAD;
    s32 s;

    if (D_80090850 < 0x400) {
        D_80090850 += 0x200;
    } else {
        D_80090850 += 0x40;
    }
    D_80090850 &= 0x7FF;
    s = rsin(D_80090850);
    LoadAverageCol(D_80090874, D_80090878, 0x1000 - s, s, (u8 *)&rgb);
    D_801A5F24.buf->cur = sp->cur;
    func_80060B54(&D_8009086C, &D_80090854, 0, &rgb, 0, 2);
    if (!(D_80041218 & 1)) {
        func_80060B54(&D_80090870, &D_80090860, 0, &rgb, 0, 2);
    }
    sp->cur = D_801A5F24.buf->cur;
}
