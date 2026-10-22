#include "ac1/ac1.h"
#include "m2c_compat.h"

/* render scratch at 0x801B4158 (ovl204 layout of ovl202's ScrBuf tail) */
typedef struct {
    u32 vis[16];                /* +0x000 one bit per world structure */
    u8 unk40[0xE30 - 0x40];
    WorldStruct *cur;           /* +0xE30 (0x801B4F88) */
} ScrVis;

extern void func_80061508(WorldStruct *w, s32 idx);
extern ScrVis D_801B4158;
extern WorldStruct D_801D4488[];

/* draw the current world structure, then every structure in its
 * neighbour list whose visibility bit is set (twin of ovl202 0x80062B04) */
void func_80061698(void)
{
    WorldStruct *w;
    u16 *list;
    s32 n;
    s32 idx;
    s32 v;

    w = D_801B4158.cur;
    if (w == 0) {
        return;
    }
    func_80061508(w, w - D_801D4488);
    n = M2C_FIELD(w, s16 *, 0x26);
    list = M2C_FIELD(w, u16 **, 0x28);
    for (n = n - 1; n != -1; n--) {
        v = *list++;
        idx = v & 0x1FF;
        if (D_801B4158.vis[idx >> 5] & (1 << (v & 0x1F))) {
            func_80061508(&D_801D4488[idx], idx);
        }
    }
}
