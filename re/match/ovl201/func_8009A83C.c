#include "ac1/ac1.h"
#include "ac1/gfxframe.h"
/* ovl201 0x8009A83C - front-end display init: both frames' DRAWENV/DISPENV
 * (draw 0/240 swapped with disp), background on, ten 320x240 layer
 * DRAWENVs per frame in g_frames (0x801BCE18) centred via ofs, and each
 * layer's 0x580-word primitive buffer (two 0x1600-byte halves per layer
 * from 0x80180868). */
typedef struct {
    DRAWENV draw;       /* +0x00 */
    u32 *buf;           /* +0x5C */
    s32 size;           /* +0x60 */
} Layer;
typedef struct {
    DRAWENV draw;       /* +0x00 */
    DISPENV disp;       /* +0x5C */
    Layer layer[10];    /* +0x70 */
    u8 pad[0x19458 - 0x70 - 0x3E8];
} Frame;

extern Frame D_801BCE18[2];
extern u32 D_80180868[2][0x580];
extern u32 D_80183468[2][0x580];
extern u32 D_80186068[2][0x580];
extern u32 D_80188C68[2][0x580];
extern u32 D_8018B868[2][0x580];
extern u32 D_8018E470[2][0x580];
extern u32 D_80191070[2][0x580];
extern u32 D_80193C70[2][0x580];
extern u32 D_80196870[2][0x580];
extern u32 D_80199470[2][0x580];
extern DRAWENV *SetDefDrawEnv(DRAWENV *env, int x, int y, int w, int h);
extern DISPENV *SetDefDispEnv(DISPENV *env, int x, int y, int w, int h);

void func_8009A83C(Frame *f)
{
    SetDefDrawEnv(&f[0].draw, 0, 0, 320, 240);
    SetDefDispEnv(&f[0].disp, 0, 240, 320, 240);
    SetDefDrawEnv(&f[1].draw, 0, 240, 320, 240);
    SetDefDispEnv(&f[1].disp, 0, 0, 320, 240);
    f[1].draw.isbg = 1;
    f[0].draw.isbg = 1;
    f[0].draw.r0 = 0;
    f[0].draw.g0 = 0;
    f[0].draw.b0 = 0;
    f[1].draw.r0 = 0;
    f[1].draw.g0 = 0;
    f[1].draw.b0 = 0;
    SetDefDrawEnv(&D_801BCE18[0].layer[0].draw, 0, 0, 320, 240);
    SetDefDrawEnv(&D_801BCE18[1].layer[0].draw, 0, 240, 320, 240);
    SetDefDrawEnv(&D_801BCE18[0].layer[1].draw, 0, 0, 320, 240);
    SetDefDrawEnv(&D_801BCE18[1].layer[1].draw, 0, 240, 320, 240);
    SetDefDrawEnv(&D_801BCE18[0].layer[2].draw, 0, 0, 320, 240);
    SetDefDrawEnv(&D_801BCE18[1].layer[2].draw, 0, 240, 320, 240);
    SetDefDrawEnv(&D_801BCE18[0].layer[3].draw, 0, 0, 320, 240);
    SetDefDrawEnv(&D_801BCE18[1].layer[3].draw, 0, 240, 320, 240);
    SetDefDrawEnv(&D_801BCE18[0].layer[4].draw, 0, 0, 320, 240);
    SetDefDrawEnv(&D_801BCE18[1].layer[4].draw, 0, 240, 320, 240);
    SetDefDrawEnv(&D_801BCE18[0].layer[5].draw, 0, 0, 320, 240);
    SetDefDrawEnv(&D_801BCE18[1].layer[5].draw, 0, 240, 320, 240);
    SetDefDrawEnv(&D_801BCE18[0].layer[6].draw, 0, 0, 320, 240);
    SetDefDrawEnv(&D_801BCE18[1].layer[6].draw, 0, 240, 320, 240);
    SetDefDrawEnv(&D_801BCE18[0].layer[7].draw, 0, 0, 320, 240);
    SetDefDrawEnv(&D_801BCE18[1].layer[7].draw, 0, 240, 320, 240);
    SetDefDrawEnv(&D_801BCE18[0].layer[8].draw, 0, 0, 320, 240);
    SetDefDrawEnv(&D_801BCE18[1].layer[8].draw, 0, 240, 320, 240);
    SetDefDrawEnv(&D_801BCE18[0].layer[9].draw, 0, 0, 320, 240);
    SetDefDrawEnv(&D_801BCE18[1].layer[9].draw, 0, 240, 320, 240);
    D_801BCE18[0].layer[0].draw.ofs[0] = D_801BCE18[0].layer[0].draw.clip.x + (D_801BCE18[0].layer[0].draw.clip.w - 320) / 2;
    D_801BCE18[0].layer[0].draw.ofs[1] = D_801BCE18[0].layer[0].draw.clip.y + (D_801BCE18[0].layer[0].draw.clip.h - 240) / 2;
    D_801BCE18[1].layer[0].draw.ofs[0] = D_801BCE18[1].layer[0].draw.clip.x + (D_801BCE18[1].layer[0].draw.clip.w - 320) / 2;
    D_801BCE18[1].layer[0].draw.ofs[1] = D_801BCE18[1].layer[0].draw.clip.y + (D_801BCE18[1].layer[0].draw.clip.h - 240) / 2;
    D_801BCE18[0].layer[1].draw.ofs[0] = D_801BCE18[0].layer[1].draw.clip.x + (D_801BCE18[0].layer[1].draw.clip.w - 320) / 2;
    D_801BCE18[0].layer[1].draw.ofs[1] = D_801BCE18[0].layer[1].draw.clip.y + (D_801BCE18[0].layer[1].draw.clip.h - 240) / 2;
    D_801BCE18[1].layer[1].draw.ofs[0] = D_801BCE18[1].layer[1].draw.clip.x + (D_801BCE18[1].layer[1].draw.clip.w - 320) / 2;
    D_801BCE18[1].layer[1].draw.ofs[1] = D_801BCE18[1].layer[1].draw.clip.y + (D_801BCE18[1].layer[1].draw.clip.h - 240) / 2;
    D_801BCE18[0].layer[2].draw.ofs[0] = D_801BCE18[0].layer[2].draw.clip.x + (D_801BCE18[0].layer[2].draw.clip.w - 320) / 2;
    D_801BCE18[0].layer[2].draw.ofs[1] = D_801BCE18[0].layer[2].draw.clip.y + (D_801BCE18[0].layer[2].draw.clip.h - 240) / 2;
    D_801BCE18[1].layer[2].draw.ofs[0] = D_801BCE18[1].layer[2].draw.clip.x + (D_801BCE18[1].layer[2].draw.clip.w - 320) / 2;
    D_801BCE18[1].layer[2].draw.ofs[1] = D_801BCE18[1].layer[2].draw.clip.y + (D_801BCE18[1].layer[2].draw.clip.h - 240) / 2;
    D_801BCE18[0].layer[3].draw.ofs[0] = D_801BCE18[0].layer[3].draw.clip.x + (D_801BCE18[0].layer[3].draw.clip.w - 320) / 2;
    D_801BCE18[0].layer[3].draw.ofs[1] = D_801BCE18[0].layer[3].draw.clip.y + (D_801BCE18[0].layer[3].draw.clip.h - 240) / 2;
    D_801BCE18[1].layer[3].draw.ofs[0] = D_801BCE18[1].layer[3].draw.clip.x + (D_801BCE18[1].layer[3].draw.clip.w - 320) / 2;
    D_801BCE18[1].layer[3].draw.ofs[1] = D_801BCE18[1].layer[3].draw.clip.y + (D_801BCE18[1].layer[3].draw.clip.h - 240) / 2;
    D_801BCE18[0].layer[4].draw.ofs[0] = D_801BCE18[0].layer[4].draw.clip.x + (D_801BCE18[0].layer[4].draw.clip.w - 320) / 2;
    D_801BCE18[0].layer[4].draw.ofs[1] = D_801BCE18[0].layer[4].draw.clip.y + (D_801BCE18[0].layer[4].draw.clip.h - 240) / 2;
    D_801BCE18[1].layer[4].draw.ofs[0] = D_801BCE18[1].layer[4].draw.clip.x + (D_801BCE18[1].layer[4].draw.clip.w - 320) / 2;
    D_801BCE18[1].layer[4].draw.ofs[1] = D_801BCE18[1].layer[4].draw.clip.y + (D_801BCE18[1].layer[4].draw.clip.h - 240) / 2;
    D_801BCE18[0].layer[5].draw.ofs[0] = D_801BCE18[0].layer[5].draw.clip.x + (D_801BCE18[0].layer[5].draw.clip.w - 320) / 2;
    D_801BCE18[0].layer[5].draw.ofs[1] = D_801BCE18[0].layer[5].draw.clip.y + (D_801BCE18[0].layer[5].draw.clip.h - 240) / 2;
    D_801BCE18[1].layer[5].draw.ofs[0] = D_801BCE18[1].layer[5].draw.clip.x + (D_801BCE18[1].layer[5].draw.clip.w - 320) / 2;
    D_801BCE18[1].layer[5].draw.ofs[1] = D_801BCE18[1].layer[5].draw.clip.y + (D_801BCE18[1].layer[5].draw.clip.h - 240) / 2;
    D_801BCE18[0].layer[6].draw.ofs[0] = D_801BCE18[0].layer[6].draw.clip.x + (D_801BCE18[0].layer[6].draw.clip.w - 320) / 2;
    D_801BCE18[0].layer[6].draw.ofs[1] = D_801BCE18[0].layer[6].draw.clip.y + (D_801BCE18[0].layer[6].draw.clip.h - 240) / 2;
    D_801BCE18[1].layer[6].draw.ofs[0] = D_801BCE18[1].layer[6].draw.clip.x + (D_801BCE18[1].layer[6].draw.clip.w - 320) / 2;
    D_801BCE18[1].layer[6].draw.ofs[1] = D_801BCE18[1].layer[6].draw.clip.y + (D_801BCE18[1].layer[6].draw.clip.h - 240) / 2;
    D_801BCE18[0].layer[7].draw.ofs[0] = D_801BCE18[0].layer[7].draw.clip.x + (D_801BCE18[0].layer[7].draw.clip.w - 320) / 2;
    D_801BCE18[0].layer[7].draw.ofs[1] = D_801BCE18[0].layer[7].draw.clip.y + (D_801BCE18[0].layer[7].draw.clip.h - 240) / 2;
    D_801BCE18[1].layer[7].draw.ofs[0] = D_801BCE18[1].layer[7].draw.clip.x + (D_801BCE18[1].layer[7].draw.clip.w - 320) / 2;
    D_801BCE18[1].layer[7].draw.ofs[1] = D_801BCE18[1].layer[7].draw.clip.y + (D_801BCE18[1].layer[7].draw.clip.h - 240) / 2;
    D_801BCE18[0].layer[8].draw.ofs[0] = D_801BCE18[0].layer[8].draw.clip.x + (D_801BCE18[0].layer[8].draw.clip.w - 320) / 2;
    D_801BCE18[0].layer[8].draw.ofs[1] = D_801BCE18[0].layer[8].draw.clip.y + (D_801BCE18[0].layer[8].draw.clip.h - 240) / 2;
    D_801BCE18[1].layer[8].draw.ofs[0] = D_801BCE18[1].layer[8].draw.clip.x + (D_801BCE18[1].layer[8].draw.clip.w - 320) / 2;
    D_801BCE18[1].layer[8].draw.ofs[1] = D_801BCE18[1].layer[8].draw.clip.y + (D_801BCE18[1].layer[8].draw.clip.h - 240) / 2;
    D_801BCE18[0].layer[9].draw.ofs[0] = D_801BCE18[0].layer[9].draw.clip.x + (D_801BCE18[0].layer[9].draw.clip.w - 320) / 2;
    D_801BCE18[0].layer[9].draw.ofs[1] = D_801BCE18[0].layer[9].draw.clip.y + (D_801BCE18[0].layer[9].draw.clip.h - 240) / 2;
    D_801BCE18[1].layer[9].draw.ofs[0] = D_801BCE18[1].layer[9].draw.clip.x + (D_801BCE18[1].layer[9].draw.clip.w - 320) / 2;
    D_801BCE18[1].layer[9].draw.ofs[1] = D_801BCE18[1].layer[9].draw.clip.y + (D_801BCE18[1].layer[9].draw.clip.h - 240) / 2;
    f[0].layer[0].buf = D_80180868[0];
    f[1].layer[0].buf = D_80180868[1];
    f[1].layer[0].size = 0x580;
    f[0].layer[0].size = 0x580;
    f[0].layer[1].buf = D_80183468[0];
    f[1].layer[1].buf = D_80183468[1];
    f[1].layer[1].size = 0x580;
    f[0].layer[1].size = 0x580;
    f[0].layer[2].buf = D_80186068[0];
    f[1].layer[2].buf = D_80186068[1];
    f[1].layer[2].size = 0x580;
    f[0].layer[2].size = 0x580;
    f[0].layer[3].buf = D_80188C68[0];
    f[1].layer[3].buf = D_80188C68[1];
    f[1].layer[3].size = 0x580;
    f[0].layer[3].size = 0x580;
    f[0].layer[4].buf = D_8018B868[0];
    f[1].layer[4].buf = D_8018B868[1];
    f[1].layer[4].size = 0x580;
    f[0].layer[4].size = 0x580;
    f[0].layer[5].buf = D_8018E470[0];
    f[1].layer[5].buf = D_8018E470[1];
    f[1].layer[5].size = 0x580;
    f[0].layer[5].size = 0x580;
    f[0].layer[6].buf = D_80191070[0];
    f[1].layer[6].buf = D_80191070[1];
    f[1].layer[6].size = 0x580;
    f[0].layer[6].size = 0x580;
    f[0].layer[7].buf = D_80193C70[0];
    f[1].layer[7].buf = D_80193C70[1];
    f[1].layer[7].size = 0x580;
    f[0].layer[7].size = 0x580;
    f[0].layer[8].buf = D_80196870[0];
    f[1].layer[8].buf = D_80196870[1];
    f[1].layer[8].size = 0x580;
    f[0].layer[8].size = 0x580;
    f[0].layer[9].buf = D_80199470[0];
    f[1].layer[9].buf = D_80199470[1];
    f[1].layer[9].size = 0x580;
    f[0].layer[9].size = 0x580;
}
