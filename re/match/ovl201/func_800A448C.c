#include "ac1/ac1.h"
/* ovl201 0x800A448C - play one movie: set up sound/CD mix, allocate the
 * decode buffers, clear VRAM, play entry idx of the movie table (falling back
 * to entry 2 when entry 0 fails), then restore the display and free buffers. */
typedef struct {
    s32 file;
    u16 frames;
    u16 pad;
} MovieEnt;
typedef struct { u8 b[0x14]; } DISPENV_;

extern MovieEnt D_800BDC68[];
extern u8 D_800BDCB8[];
extern RECT D_800BDCC4;
extern RECT D_8004D234;
extern DISPENV_ D_8017F3E8;
extern void *D_8017F434;
extern void *D_8017F438;
extern void *D_8017F43C;
extern void *D_8017F440;
extern void *D_8017F444;
extern u16 D_8017F44C;
extern void *func_80050FC4(s32 size);
extern void func_80051078(void *p);
extern s32 func_800A46AC(s32 file, u8 mode);
extern void SsSetSerialAttr(s32 a, s32 b, s32 c);
extern void SsSetSerialVol(s32 a, s32 b, s32 c);
extern void SsSetMVol(s32 a, s32 b);
extern int CdMix(void *vol);
extern int SetDispMask(int mask);
extern int ClearImage(RECT *r, int a, int b, int c);
extern int DrawSync(int mode);
extern void *SetDefDispEnv(void *env, int x, int y, int w, int h);
extern void *PutDispEnv(void *env);

void func_800A448C(s16 idx, u8 mode)
{
    RECT r;
    s32 ok;

    SsSetSerialAttr(0, 0, 1);
    SsSetSerialVol(0, 0x7F, 0x7F);
    SsSetMVol(0x7F, 0x7F);
    CdMix(D_800BDCB8);
    SetDispMask(0);
    D_8017F434 = func_80050FC4(0x8000);
    D_8017F438 = func_80050FC4(0x25800);
    D_8017F43C = func_80050FC4(0x25800);
    D_8017F440 = func_80050FC4(0x2D00);
    D_8017F444 = func_80050FC4(0x2D00);
    r = D_8004D234;
    ClearImage(&r, 0, 0, 0);
    DrawSync(0);
    D_8017F44C = D_800BDC68[idx].frames;
    ok = func_800A46AC(D_800BDC68[idx].file, mode);
    if (idx == 0 && ok == 0) {
        D_8017F44C = D_800BDC68[2].frames;
        func_800A46AC(D_800BDC68[2].file, mode);
    }
    SetDispMask(0);
    SetDefDispEnv(&D_8017F3E8, 0, 0xF0, 0x140, 0xF0);
    PutDispEnv(&D_8017F3E8);
    ClearImage(&D_800BDCC4, 0, 0, 0);
    DrawSync(0);
    SetDispMask(1);
    func_80051078(D_8017F444);
    func_80051078(D_8017F440);
    func_80051078(D_8017F43C);
    func_80051078(D_8017F438);
    func_80051078(D_8017F434);
}
