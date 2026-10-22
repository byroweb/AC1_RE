#include "ac1/ac1.h"
/* ovl204 VS battle main loop: save both player records, init the mode, run
 * frames until a side wins / draws (round counters at 0x80039D08..0E), then
 * restore the records.  Returns 1 when the round ended in a double KO. */
typedef struct {
    u8 pad00[0x40];
    u8 state;                   /* +0x40 0 / 0xFE = down */
    u8 pad41[0x160 - 0x41];
    u16 ap;                     /* +0x160 */
} VEnt4;
typedef struct {                /* 0x39C */
    s32 w0;                     /* +0x000 */
    s32 w4;                     /* +0x004 */
    u8 pad008[8];
    u16 flags;                  /* +0x010 */
    u8 pad012[0x390 - 0x12];
    u8 wins;                    /* +0x390 */
    u8 pad391[3];
    VEnt4 *ent;                 /* +0x394 */
    u8 pad398[4];
} VRec4;
typedef struct {                /* 0x801AB684 */
    u32 flags;
    u16 phase;                  /* +0x04 */
    u16 elapsed;                /* +0x06 */
    u16 clock;                  /* +0x08 */
} VsState4;
typedef struct {
    void (*frame)(void);
} VScene4;
typedef struct {
    u16 a;
    u16 b;
} VPair4;
extern VRec4 D_80040FA0[2];
extern VRec4 D_8008C2B8;
extern VRec4 D_8008C654;
extern VRec4 *D_80039D18;
extern VsState4 D_801AB684;
extern VScene4 *D_801AB67C;
extern VPair4 D_80039D0A;
extern u16 D_80039D0E[];
extern u16 D_80039D08;
extern u8 D_80039D2A;
extern u8 D_80041BB8[];
#define D_80041BBB D_80041BB8[3]
extern u8 D_801C4460[];
extern RECT D_800893C8;
extern void *volatile D_800893D0;
extern void SsSetMVol(s32 l, s32 r);
extern void func_80015538(void *p, s32 v, s32 n);
extern void InitHeap2(void *p, s32 n);
extern void SsUtAllKeyOff(s32 a);
extern void func_80056434(void);
extern void func_8006CD54(void);
extern void func_80072A70(void);
extern void func_80016E64(void);
extern void func_8004F964(void);
extern void func_80081FD0(void);
extern void func_80088D34(void);
extern void func_8008156C(void);
extern void func_80055CB8(s32 a, s32 b, s32 c, s32 d, s32 e, s32 f, s32 g);
extern void srand(s32 s);
extern void func_8004C7C8(void);
extern void func_80012298(void);
extern void ClearImage(RECT *r, s32 a, s32 b, s32 c);
extern void func_80089354(void);
extern void func_800722C4(void);
extern void func_8007286C(void);
extern void func_80077E78(void);
extern void func_80086388(void);
extern void func_8004D09C(s32 id, s32 vol);
extern void func_80061AD4(VECTOR *a, VECTOR *b);
extern void func_80081A00(VECTOR *v);
extern void func_8004D004(VECTOR *a, VECTOR *b);
extern void func_800174F8(void *cb);
extern void func_8004DC18(s32 a);
extern void func_80016D64(void);
extern void func_80016DA8(void);
extern void VSync(s32 a);

s32 func_8004BF3C(void)
{
    VECTOR v0;
    VECTOR v1;
    s32 wait;
    s32 ret;

    wait = 0;
    ret = 0;
    D_8008C2B8 = D_80040FA0[0];
    D_8008C654 = D_80040FA0[1];
    SsSetMVol(0, 0);
    DrawSync(0);
    func_80015538(D_801C4460, 0, 0x1000);
    InitHeap2(D_801C4460, 0x4000);
    SsUtAllKeyOff(0);
    func_80056434();
    func_8006CD54();
    func_80072A70();
    func_80016E64();
    func_8004F964();
    func_80081FD0();
    func_80088D34();
    D_80039D18 = &D_80040FA0[0];
    func_8008156C();
    D_80039D18 = &D_80040FA0[1];
    func_8008156C();
    func_80055CB8(512, 320, 0, 1, 1, 64, 64);
    srand(1);
    func_8004C7C8();
    SsSetMVol(D_80041BBB, D_80041BBB);
    func_80012298();
    ClearImage(&D_800893C8, 0, 0, 0);
    for (;;) {
        D_80040FA0[0].flags &= ~7;
        D_80040FA0[1].flags &= ~7;
        func_80089354();
        func_800722C4();
        func_8007286C();
        func_80077E78();
        func_80086388();
        D_801AB67C->frame();
        if (D_801AB684.phase != 0) {
            if (D_801AB684.phase == 90 && (D_801AB684.flags & 0xB0)) {
                D_801AB684.flags &= ~0x40;
            } else if (D_801AB684.phase == 30) {
                D_801AB684.flags &= ~0x80;
                D_80040FA0[0].flags |= 0x30;
                D_80040FA0[1].flags |= 0x30;
                if (D_80040FA0[0].w4 == 0) {
                    if (D_80040FA0[0].w0 == 64) {
                    dko:
                        func_8004D09C(55, 100);
                        D_80040FA0[0].flags &= ~0x30;
                        D_80040FA0[1].flags &= ~0x30;
                        func_80061AD4(0, 0);
                        ret = 1;
                        func_80061AD4(0, 0);
                        break;
                    }
                    if (D_80040FA0[0].w0 == 32) {
                        goto ko;
                    }
                }
                if (D_80040FA0[1].w4 != 0) {
                    goto next;
                }
                if (D_80040FA0[1].w0 == 64) {
                    goto dko;
                }
                if (D_80040FA0[1].w0 != 32) {
                    goto next;
                }
            ko:
                func_8004D09C(54, 100);
            }
            if (D_801AB684.phase < 20) {
                D_80041BBB -= 6;
                if (D_80041BBB > 128) {
                    D_80041BBB = 0;
                }
                SsSetMVol(D_80041BBB, D_80041BBB);
            }
            if (D_801AB684.phase-- == 1) {
                break;
            }
            goto next;
        }
        if (D_801AB684.elapsed >= 16 && D_801AB684.clock >= 2) {
            D_801AB684.clock--;
        }
        if (D_801AB684.clock == 1) {
            D_801AB684.phase = 30;
            D_801AB684.flags |= 0x40;
            if (D_80040FA0[0].ent->ap > D_80040FA0[1].ent->ap) {
                goto win0;
            }
            if (D_80040FA0[0].ent->ap < D_80040FA0[1].ent->ap) {
                goto win1;
            }
        draw:
            D_801AB684.clock = 0;
            D_801AB684.flags |= 0x80;
            D_801AB684.phase += 90;
            goto next;
        }
        if ((D_80040FA0[0].flags & 7) == 1) {
            if ((D_80040FA0[1].flags & 7) == 1) {
                goto draw;
            }
        win1:
            D_801AB684.clock = 0;
            D_801AB684.flags |= 0x30;
            D_801AB684.phase += 90;
            D_80040FA0[0].flags |= 0x20;
            D_80040FA0[1].flags |= 0x10;
            D_80040FA0[1].wins |= 1 << D_80039D2A;
            if (++D_80039D0A.a >= 1000) {
                D_80039D0A.a = 999;
            }
            if (++D_80039D0A.b >= 1000) {
                D_80039D0A.b = 999;
            }
            goto inc;
        } else if ((D_80040FA0[1].flags & 7) == 1) {
        win0:
            D_801AB684.clock = 0;
            D_801AB684.flags |= 0x30;
            D_801AB684.phase += 90;
            D_80040FA0[0].flags |= 0x10;
            D_80040FA0[1].flags |= 0x20;
            D_80040FA0[0].wins |= 1 << D_80039D2A;
            if (++D_80039D0E[0] >= 1000) {
                D_80039D0E[0] = 999;
            }
            if (++D_80039D08 >= 1000) {
                D_80039D08 = 999;
            }
        inc:
            D_80039D2A++;
        } else if (wait == 0) {
            if (D_80040FA0[0].ent->state == 0 || D_80040FA0[0].ent->state == 254
                || D_80040FA0[1].ent->state == 0 || D_80040FA0[1].ent->state == 254) {
                wait = 12;
            }
        } else if (--wait == 0) {
            if (D_80040FA0[0].ent->state == 0 || D_80040FA0[0].ent->state == 254) {
                if (D_80040FA0[1].ent->state == 0 || D_80040FA0[1].ent->state == 254) {
                    goto draw;
                }
                goto win1;
            }
            if (D_80040FA0[1].ent->state == 0 || D_80040FA0[1].ent->state == 254) {
                goto win0;
            }
        }
    next:
        D_80039D18 = &D_80040FA0[0];
        func_80081A00(&v0);
        D_80039D18 = &D_80040FA0[1];
        func_80081A00(&v1);
        func_80061AD4(&v0, &v1);
        func_8004D004(&v0, &v1);
        if (D_801AB684.elapsed < 0x4000) {
            D_801AB684.elapsed++;
        }
    }
    if (D_800893D0 != 0) {
        func_800174F8(D_800893D0);
    }
    SsSetMVol(0, 0);
    func_8004DC18(0xFFFF);
    func_80016D64();
    func_80016DA8();
    D_80040FA0[0] = D_8008C2B8;
    D_80040FA0[1] = D_8008C654;
    DrawSync(0);
    VSync(0);
    return ret;
}
