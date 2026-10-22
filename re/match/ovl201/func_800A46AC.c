#include "ac1/ac1.h"
#include "ac1/gfxframe.h"
/* ovl201 0x800A46AC - stream one movie file: find it on the CD, set up the
 * stream state D_8017F3FC and start reading, then per frame decode
 * (func_800B4FF0 / func_800B506C), wait for the frame (re-seeking when the
 * stream position runs past the end), flip the 24-bit display.  Ends when
 * D_8017F448 is set or, if skippable, on START/X; returns 1 when skipped. */
typedef struct {
    u8 minute, second, sector, track;
} CdlLOC;
typedef struct {
    CdlLOC pos;
    u32 size;
    char name[16];
} CdlFILE;
typedef struct {
    s16 x, y, end, pad;
} StrmEnt;
typedef struct {
    u32 *dec[2];        /* +0x00 0x8017F3FC */
    s32 dcur;           /* +0x08 */
    u32 *buf[2];        /* +0x0C */
    s32 cur;            /* +0x14 */
    StrmEnt ent[2];     /* +0x18 */
    s32 sel;            /* +0x28 */
    RECT rect;          /* +0x2C */
    s32 done;           /* +0x34 */
} Strm;

extern Strm D_8017F3FC;
extern s32 D_8017F448;
extern s16 D_8017F44C;
extern CdlFILE *CdSearchFile(CdlFILE *fp, char *name);
extern int StGetBackloc(CdlLOC *loc);
extern int CdControl(u8 com, u8 *param, u8 *result);
extern void StUnSetRing(void);
extern int VSync(int mode);
extern int SetDispMask(int mask);
extern DISPENV *SetDefDispEnv(DISPENV *env, int x, int y, int w, int h);
extern DRAWENV *SetDefDrawEnv(DRAWENV *env, int x, int y, int w, int h);
extern DISPENV *PutDispEnv(DISPENV *env);
extern DRAWENV *PutDrawEnv(DRAWENV *env);
extern void SsSetSerialVol(s32 a, s32 b, s32 c);
extern void SsSetMVol(s32 a, s32 b);
extern u32 func_80016A84(s32 a);
extern void func_800A499C(Strm *s, s32 a, s32 b, s32 c, s32 d);
extern void func_800A4A04(CdlFILE *fp, void (*cb)(void));
extern s32 func_800A4BE8(Strm *s);
extern void func_800A4DA0(Strm *s, s32 a);
extern void func_800A4E38(CdlLOC *loc);
extern void func_800A4A74(void);
extern void func_800AE838(void);
extern void func_800B4FF0(u32 *p, s32 a);
extern void func_800B506C(u32 *p, s32 size);
extern void func_800B5128(s32 a);

s32 func_800A46AC(char *name, u8 skippable)
{
    DISPENV disp;
    DRAWENV draw;
    CdlFILE file;
    CdlLOC loc;
    s32 skipped = 0;
    s32 y;

    if (CdSearchFile(&file, name) == 0) {
        return 0;
    }
    func_800A499C(&D_8017F3FC, 0, 0, 0, 0xF0);
    func_800A4A04(&file, func_800A4A74);
    while (func_800A4BE8(&D_8017F3FC) == -1) {
        loc = file.pos;
        func_800A4E38(&loc);
    }
    D_8017F448 = 0;
    for (;;) {
        func_800B4FF0(D_8017F3FC.dec[D_8017F3FC.dcur], 3);
        func_800B506C(D_8017F3FC.buf[D_8017F3FC.cur], D_8017F3FC.rect.w * D_8017F3FC.rect.h / 2);
        while (func_800A4BE8(&D_8017F3FC) == -1) {
            s32 at = StGetBackloc(&loc);

            if (D_8017F44C < at || at <= 0) {
                loc = file.pos;
            }
            func_800A4E38(&loc);
        }
        func_800A4DA0(&D_8017F3FC, 0);
        VSync(0);
        y = D_8017F3FC.sel == 0;
        y *= 240;
        SetDefDispEnv(&disp, 0, y, 480, 240);
        SetDefDrawEnv(&draw, 0, y, 480, 240);
        disp.isrgb24 = 1;
        disp.disp.w = disp.disp.w * 2 / 3;
        PutDispEnv(&disp);
        PutDrawEnv(&draw);
        SetDispMask(1);
        if (D_8017F448 == 1) {
            break;
        }
        if (skippable && (func_80016A84(1) & 0x840)) {
            skipped = 1;
            break;
        }
    }
    SsSetSerialVol(0, 0, 0);
    SsSetMVol(0, 0);
    func_800B5128(0);
    while (CdControl(9, 0, 0) == 0) {
    }
    StUnSetRing();
    func_800AE838();
    SsSetSerialVol(0, 0x7F, 0x7F);
    return skipped;
}
