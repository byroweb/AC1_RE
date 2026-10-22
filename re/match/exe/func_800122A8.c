#include "ac1/ac1.h"

register u8 *gp_base asm("$28");

extern char D_80011000[];
extern char D_8001100C[];
extern char D_80011018[];
extern char D_80011028[];
extern char D_80011038[];
extern char D_80011048[];
extern char D_80011054[];
extern char D_80011060[];
extern char D_8001106C[];
extern char D_80011078[];
extern char D_80011084[];
extern char D_80011090[];
extern char D_8001109C[];
extern char D_800110A8[];
extern char D_800110B4[];
extern char D_800110C0[];
extern char D_800110CC[];
extern char D_800110D8[];
extern u32 *D_800112C8;
extern u8 D_80039C68[];
extern s16 D_80041590;
extern s16 D_800411F4;
extern u8 D_800416CC;
extern u8 D_80041330;
extern u8 D_800411F8;
extern s32 D_80048610;
extern s32 (*D_8004ADA4)(void);
extern void func_800167D0(void);
extern void func_80016ACC(void);
extern void func_80016678(u16 id, char *name);
extern void func_800121AC(void);
extern void func_80015538(void *p, s32 v, s32 n);
extern void func_800165E4(s32 a, s32 b, void *dst);
extern void func_8009B414(void);
extern s32 func_800309A8(s32 cmd, s32 arg, s32 param);
extern void ResetGraph(s32 mode);
extern void SetDispMask(s32 mask);
extern void InitGeom(void);
extern void ClearImage(void *rect, s32 r, s32 g, s32 b);

void func_800122A8(void)
{
    ResetGraph(0);
    SetDispMask(0);
    InitGeom();
    func_800167D0();
    func_80016ACC();
    ClearImage(D_80039C68, 0, 0, 0);
    func_80016678(1, D_80011000);
    func_800121AC();
    func_80016678(2, D_8001100C);
    func_80016678(4, D_80011018);
    func_80016678(6, D_80011028);
    func_80016678(5, D_80011038);
    func_80016678(7, D_80011048);
    func_80016678(8, D_80011054);
    func_80016678(9, D_80011060);
    func_80016678(10, D_8001106C);
    func_80016678(11, D_80011078);
    func_80016678(12, D_80011084);
    func_80016678(13, D_80011090);
    func_80016678(14, D_8001109C);
    func_80016678(15, D_800110A8);
    func_80016678(16, D_800110B4);
    func_80016678(17, D_800110C0);
    func_80016678(18, D_800110CC);
    func_80016678(19, D_800110D8);
    D_80041590 = 0;
    D_800411F4 = 0;
    for (;;) {
        func_80015538(D_800112C8, 0, (u32 *)0x801F8000 - D_800112C8);
        func_800165E4(2, 0xC9, D_800112C8);
        func_8009B414();
        func_800121AC();
        gp_base[0xCE] = 0;
        gp_base[0xA4] = 0;
        D_800416CC = 0;
        D_80041330 = 0;
        do {
            func_80015538(D_800112C8, 0, (u32 *)0x801F8000 - D_800112C8);
            switch (gp_base[2]) {
            case 0:
                func_800165E4(2, 0xCA, D_800112C8);
                break;
            case 1:
                func_800165E4(2, 0xCB, D_800112C8);
                break;
            case 2:
                func_800165E4(2, 0xCC, D_800112C8);
                break;
            }
        } while (D_8004ADA4() != 0);
        func_800309A8(2, 0, 0);
        if ((D_800411F8 == 0x15 || D_800411F8 == 0x16 || D_800411F8 == 0x31) && D_80048610 != 1) {
            continue;
        }
        if ((D_800411F8 == 0x2F || D_800411F8 == 0x30) && D_80048610 == 2) {
            continue;
        }
        func_800121AC();
    }
}
