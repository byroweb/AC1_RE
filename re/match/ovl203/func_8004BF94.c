#include "ac1/ac1.h"
/* ovl203: link-cable version of ovl204 0x8004BF3C (handshake over the link first,
 * then the same frame loop with me/other record pointers). ovl204 notes: VS battle main loop: save both player records, init the mode, run
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
typedef struct {                /* 0x8004A288 link cable buffers */
    s32 rfd;                    /* +0x00 */
    volatile union { u8 b[8]; s32 w[2]; } wbuf;    /* +0x04 0x8004A28C */
    volatile s32 w0c;           /* +0x0C 0x8004A294 */
    volatile union { u8 b[8]; s32 w[2]; } rbuf;    /* +0x10 0x8004A298 */
    volatile s32 w18;           /* +0x18 0x8004A2A0 */
} Link3;
extern VRec4 D_80040FA0[2];
extern VRec4 D_8008C490;
extern VRec4 *D_80039D18;
extern VRec4 *D_80039D30;       /* me */
extern VRec4 *D_80039D34;       /* other console */
extern VsState4 D_8019B48C;
extern VScene4 *D_8019B484;
extern VPair4 D_80039D0A;
extern u16 D_80039D0E[];
extern u16 D_80039D08;
extern u8 D_80039D2A;
extern u8 D_80039CAC;
extern u8 D_80041BB8[];
#define D_80041BBB D_80041BB8[3]
extern u8 D_801B40C8[];
extern u8 D_800895C8;
extern volatile s32 D_800895C4;
extern void *volatile D_800895CC;
extern s32 D_8004A284;          /* write fd */
extern Link3 D_8004A288;
extern void SsSetMVol(s32 l, s32 r);
extern void func_80015538(void *p, s32 v, s32 n);
extern void InitHeap2(void *p, s32 n);
extern void SsUtAllKeyOff(s32 a);
extern void func_80056D5C(void);
extern void func_8004BE54(void);
extern void func_8006CF18(void);
extern void func_80072C34(void);
extern void func_80016E64(void);
extern void func_8004FE84(void);
extern void func_8008223C(void);
extern void func_80088F84(void);
extern void func_800818C0(void);
extern void func_80056668(s32 a, s32 b, s32 c, s32 d, s32 e, s32 f, s32 g);
extern void srand(s32 s);
extern void func_8004C938(void);
extern void func_80029B70(s32 fd, volatile void *buf, s32 n);
extern void func_80029B80(s32 fd, volatile void *buf, s32 n);
extern s32 func_8001628C(void);
extern s32 func_80016278(s32 t);
extern s32 func_800309A8(s32 a, s32 b, s32 c);
extern s32 func_8004BEB0(void);
extern void func_80012298(void);
extern void func_80089558(void);
extern void func_80072488(void);
extern void func_80072A30(void);
extern void func_800781BC(void);
extern void func_800865D8(void);
extern void func_8004D1D0(s32 id, s32 vol);
extern void func_80062020(VECTOR *v);
extern void func_80081C9C(VECTOR *v);
extern void func_8004D184(VECTOR *v);
extern void func_800174F8(void *cb);
extern void func_8004DD34(s32 a);
extern void func_80016D64(void);
extern void func_80016DA8(void);
extern void func_8004BE88(void);
extern void VSync(s32 a);

s32 func_8004BF94(void)
{
    VECTOR v;
    s32 wait;
    s32 ret;

    ret = 0;
    wait = 0;
    D_8008C490 = D_80040FA0[0];
    SsSetMVol(0, 0);
    DrawSync(0);
    func_80015538(D_801B40C8, 0, 0x1000);
    InitHeap2(D_801B40C8, 0x4000);
    SsUtAllKeyOff(0);
    func_80056D5C();
    func_8004BE54();
    func_8006CF18();
    func_80072C34();
    func_80016E64();
    func_8004FE84();
    func_8008223C();
    func_80088F84();
    func_800818C0();
    func_80056668(512, 320, 0, 1, 1, 64, 64);
    srand(1);
    func_8004C938();
    if (D_800895C8 != 0) {
        goto exit;
    }
    D_8004A288.wbuf.b[0] = D_8004A288.wbuf.b[7] = 0;
    D_8004A288.rbuf.b[0] = 82;
    D_8004A288.rbuf.b[7] = 67;
    func_80029B70(D_8004A284, D_8004A288.wbuf.b, 8);
    D_800895C4 = func_8001628C();
    for (;;) {
        if (func_800309A8(3, 1, 0) != 0) {
            goto linked;
        }
        if (func_80016278(D_800895C4) >= 1800) {
            D_800895C8 = 1;
            goto exit;
        }
    }
linked:
    D_800895C4 = func_8001628C();
    func_80029B80(D_8004A288.rfd, D_8004A288.rbuf.b, 8);
    if (D_800895C8 != 0) {
        goto exit;
    }
    if (func_8004BEB0() == 0) {
        goto exit;
    }
    D_8004A288.wbuf.w[0] = D_8004A288.wbuf.w[1] = D_8004A288.w0c = D_8004A288.rbuf.w[0]
        = D_8004A288.rbuf.w[1] = D_8004A288.w18 = 0;
    D_8004A288.wbuf.b[0] = D_8004A288.rbuf.b[0] = 82;
    D_8004A288.wbuf.b[7] = D_8004A288.rbuf.b[7] = 67;
    SsSetMVol(D_80041BBB, D_80041BBB);
    func_80012298();
    for (;;) {
        D_80040FA0[0].flags &= ~7;
        D_80040FA0[1].flags &= ~7;
        func_80089558();
        func_80072488();
        func_80072A30();
        func_800781BC();
        func_800865D8();
        D_8019B484->frame();
        if (D_8019B48C.phase != 0) {
            if (D_8019B48C.phase == 90 && (D_8019B48C.flags & 0xB0)) {
                D_8019B48C.flags &= ~0x40;
            } else if (D_8019B48C.phase == 30 && D_800895C8 == 0) {
                D_8019B48C.flags &= ~0x80;
                D_80040FA0[0].flags |= 0x30;
                D_80040FA0[1].flags |= 0x30;
                if (D_80040FA0[0].w4 == 0) {
                    if (D_80040FA0[0].w0 == 64) {
                    dko:
                        func_8004D1D0(55, 100);
                        D_80040FA0[0].flags &= ~0x30;
                        D_80040FA0[1].flags &= ~0x30;
                        func_80062020(0);
                        ret = 1;
                        func_80062020(0);
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
                func_8004D1D0(54, 100);
            }
            if (D_8019B48C.phase < 20) {
                D_80041BBB -= 6;
                if (D_80041BBB > 128) {
                    D_80041BBB = 0;
                }
                SsSetMVol(D_80041BBB, D_80041BBB);
            }
            if (D_8019B48C.phase-- == 1) {
                break;
            }
            goto next;
        }
        if (D_8019B48C.elapsed >= 16 && D_8019B48C.clock >= 2) {
            D_8019B48C.clock--;
        }
        if (D_8019B48C.clock == 1) {
            D_8019B48C.phase = 30;
            D_8019B48C.flags |= 0x40;
            if (D_80039D30->ent->ap > D_80039D34->ent->ap) {
                goto win0;
            }
            if (D_80039D30->ent->ap < D_80039D34->ent->ap) {
                goto win1;
            }
        draw:
            D_8019B48C.clock = 0;
            D_8019B48C.flags |= 0x80;
            D_8019B48C.phase += 90;
            goto next;
        }
        if ((D_80039D30->flags & 7) == 1) {
            if ((D_80039D34->flags & 7) == 1) {
                goto draw;
            }
        win1:
            D_8019B48C.clock = 0;
            D_8019B48C.flags |= 0x30;
            D_8019B48C.phase += 90;
            D_80039D30->flags |= 0x20;
            D_80039D34->flags |= 0x10;
            D_80039D34->wins |= 1 << D_80039D2A;
            if (++D_80039D0A.a >= 1000) {
                D_80039D0A.a = 999;
            }
            if (++D_80039D0A.b >= 1000) {
                D_80039D0A.b = 999;
            }
            goto inc;
        } else if ((D_80039D34->flags & 7) == 1) {
        win0:
            D_8019B48C.clock = 0;
            D_8019B48C.flags |= 0x30;
            D_8019B48C.phase += 90;
            D_80039D30->flags |= 0x10;
            D_80039D34->flags |= 0x20;
            D_80039D30->wins |= 1 << D_80039D2A;
            if (++D_80039D0E[0] >= 1000) {
                D_80039D0E[0] = 999;
            }
            if (++D_80039D08 >= 1000) {
                D_80039D08 = 999;
            }
        inc:
            D_80039D2A++;
        } else if (wait == 0) {
            if (D_80039D30->ent->state == 0 || D_80039D30->ent->state == 254
                || D_80039D34->ent->state == 0 || D_80039D34->ent->state == 254) {
                wait = 12;
            }
        } else if (--wait == 0) {
            if (D_80039D30->ent->state == 0 || D_80039D30->ent->state == 254) {
                if (D_80039D34->ent->state == 0 || D_80039D34->ent->state == 254) {
                    goto draw;
                }
                goto win1;
            }
            if (D_80039D34->ent->state == 0 || D_80039D34->ent->state == 254) {
                goto win0;
            }
        }
    next:
        D_80039D18 = D_80039D30;
        func_80081C9C(&v);
        func_80062020(&v);
        func_8004D184(&v);
        if (D_8019B48C.elapsed < 0x4000) {
            D_8019B48C.elapsed++;
        }
    }
exit:
    if (D_800895CC != 0) {
        func_800174F8(D_800895CC);
    }
    SsSetMVol(0, 0);
    func_8004DD34(0xFFFF);
    func_80016D64();
    func_80016DA8();
    func_8004BE88();
    D_80040FA0[0] = D_8008C490;
    if (D_800895C8 != 0) {
        func_800309A8(2, 3, 0);
        D_80039CAC = 1;
    }
    DrawSync(0);
    VSync(0);
    return ret;
}
