#include "ac1/ac1.h"
/* ovl202 0x8004C340 - mission main: init every subsystem, run the frame loop
 * (alarm / objective messages while the end timer is 0, fade the volume out
 * once it runs), then shut down and return 1 the first time a mission is
 * cleared ('0' -> '1'); D_80048610 = 1/2 from mission flags 0x100/0x200. */

typedef struct {
    u8 kind;            /* +0x00 0x8019F518 mission kind / result code */
    u8 pad1[3];
    void *objective;    /* +0x04 */
    s32 buf;            /* +0x08 */
    u32 flags;          /* +0x0C */
    u16 end_timer;      /* +0x10 0x8019F528 */
    u16 end_seq;        /* +0x12 */
    u16 timer;          /* +0x14 */
} MState;

extern MState D_8019F518_s;
#define D_8019F528 D_8019F518_s
#define D_8019F518 D_8019F518_s.kind
extern u16 D_80040FB0_a[];
#define D_80040FB0 D_80040FB0_a[0]
extern u8 D_80041BB8_a[];
#define D_80041BBB D_80041BB8_a[3]
extern u8 D_800411F8;
extern s32 D_80048610;
extern void *volatile D_8008D924;
extern u8 D_801D0B44;   /* D_801D0B44 as a scalar */
extern u16 D_801D0B46;  /* D_801D0B46 */
extern u8 D_801BCB40[];
extern void func_80015538(void *p, s32 v, s32 n);
extern void func_80057360(void);
extern void func_8006DA78(void);
extern void func_80073BC8(void);
extern void func_80016E64(void);
extern void func_8004F508(void);
extern void func_800835A0(void);
extern void func_8008A344(void);
extern void func_800523DC(void);
extern void func_80082C58(void);
extern void func_8008D8DC(void);
extern void func_80056C6C(s32, s32, s32, s32, s32, s32, s32);
extern void func_8004C730(void);
extern void func_80012298(void);
extern void func_8008AB68(void);
extern void func_80073104(void);
extern void func_800736AC(void);
extern void func_800796F4(void);
extern void func_8008792C(void);
extern void func_800529F0(s32, s32, s32);
extern void func_8004C318(s32);
extern void func_80083038(void *);
extern void func_80062BD8(void *);
extern void func_8004CF7C(void *);
extern void func_8004DB18(s32);
extern void func_80016D64(void);
extern void func_80016DA8(void);
extern void func_8008AF2C(void);

s32 func_8004C340(void)
{
    u8 sp20[16];
    s32 first;

    first = 0;
    SsSetMVol(0, 0);
    DrawSync(0);
    func_80015538(D_801BCB40, 0, 0x2000);
    InitHeap2((void *)D_801BCB40, 0x8000);
    SsUtAllKeyOff(0);
    func_80057360();
    func_8006DA78();
    func_80073BC8();
    func_80016E64();
    func_8004F508();
    func_800835A0();
    func_8008A344();
    func_800523DC();
    func_80082C58();
    func_8008D8DC();
    ((void (**)(void))g_objective_obj)[1]();
    func_80056C6C(512, 320, 0, 1, 1, 64, 64);
    srand(1);
    func_8004C730();
    SsSetMVol(D_80041BBB, D_80041BBB);
    func_80012298();
    for (;;) {
        D_80040FB0 &= 0xFFF8;
        func_8008AB68();
        func_80073104();
        func_800736AC();
        func_800796F4();
        func_8008792C();
        if (D_8019F528.end_timer != 0) {
            if (D_8019F528.end_timer < 20) {
                D_80041BBB -= 6;
                if (D_80041BBB > 128) {
                    D_80041BBB = 0;
                }
                SsSetMVol(D_80041BBB, D_80041BBB);
            }
            if (D_8019F528.end_timer-- == 1) {
                break;
            }
        } else {
            if (D_8019F528.end_seq >= 16 && D_8019F528.timer >= 2) {
                D_8019F528.timer--;
            }
            if (D_8019F528.timer == 1) {
                func_800529F0(0, 0, D_801D0B44);
                func_8004C318(D_801D0B46);
            } else if ((D_80040FB0 & 7) == 1) {
                func_800529F0(0, 0, 105);
                func_8004C318(0x101);
            } else if (g_ac_entities[0].ai_mode == 0) {
                if (D_8019F518 == 70) {
                    func_800529F0(0, 0, 145);
                } else if (D_8019F518 != 0 && (u8)(D_8019F518 - 21) >= 2 && D_8019F518 != 49) {
                    func_800529F0(0, 0, 121);
                }
                func_8004C318(0x200);
            }
        }
        func_80083038(sp20);
        func_80062BD8(sp20);
        func_8004CF7C(sp20);
        {
            u16 *p = &g_mission_end_seq;
            if (*p < 0x4000) {
                (*p)++;
            }
        }
    }
    if (D_8008D924 != 0) {
        CdSyncCallback(D_8008D924);
    }
    SsSetMVol(0, 0);
    func_8004DB18(0xFFFF);
    func_80016D64();
    func_80016DA8();
    DrawSync(0);
    VSync(0);
    func_8008AF2C();
    if (g_mission_flags & 0x100) {
        D_80048610 = 1;
    } else if (g_mission_flags & 0x200) {
        D_80048610 = 2;
    } else {
        D_80048610 = 0;
        {
            u8 *p = &D_800411F8;
            if (*p == '0') {
                *p = '1';
                first = 1;
            }
        }
    }
    return first;
}
