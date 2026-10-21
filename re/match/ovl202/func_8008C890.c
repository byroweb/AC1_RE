#include "ac1/ac1.h"
#include "ac1/ovl202_t42.h"

extern DrawCtx D_801A5F24;
extern u32 D_8008DA1C[];
extern void func_800610A4(s16 *r, u32 *t, s32 a, s32 b, s32 c);
extern void func_80051150(s32 x, s32 y, s32 a, s32 b, s32 c, s32 d);

void func_8008C890(s16 *r, s32 a1, s32 idx, s32 a3) {
    r[2] = r[0] + 16;
    r[3] = r[1] - 16;
    D_801A5F24.buf->cur = SPAD->cur;
    func_800610A4(r, &D_8008DA1C[idx], 1, a3, 0);
    func_80051150(r[2], r[3] - 3, a3, 128, idx, a1);
    SPAD->cur = D_801A5F24.buf->cur;
}
