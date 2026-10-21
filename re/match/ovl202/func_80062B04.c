#include "ac1/ac1.h"
#include "m2c_compat.h"

typedef struct {
    s32 sxy;
    s32 otz;
    s32 unk8;
    s32 dp;
} ScrVtx;

/* render scratch block at 0x801A6A48 (the screen-vertex table, the
 * world-structure visibility bits and the current world structure) */
typedef struct {
    ScrVtx vtx[1505];
    u8 unk5E10[8];
    u32 vis[16];                /* +0x5E18 one bit per world structure */
    u8 unk5E58[0x6C1C - 0x5E58];
    s32 dpflag;                 /* +0x6C1C (0x801AD664) */
    WorldStruct *cur;           /* +0x6C20 (0x801AD668) */
} ScrBuf;

extern void func_80062974(WorldStruct *w, s32 idx);
extern ScrBuf D_801A6A48;

/* draw the current world structure, then every structure in its
 * neighbour list whose visibility bit is set */
void func_80062B04(void)
{
    WorldStruct *w;
    u16 *list;
    s32 n;
    s32 idx;
    s32 v;

    w = D_801A6A48.cur;
    if (w == 0) {
        return;
    }
    func_80062974(w, w - g_world_structs);
    n = M2C_FIELD(w, s16 *, 0x26);
    list = M2C_FIELD(w, u16 **, 0x28);
    for (n = n - 1; n != -1; n--) {
        v = *list++;
        idx = v & 0x1FF;
        if (D_801A6A48.vis[idx >> 5] & (1 << (v & 0x1F))) {
            func_80062974(&g_world_structs[idx], idx);
        }
    }
}
