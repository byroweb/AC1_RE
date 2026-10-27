/* ovl201 0x8009C964 ac_calc_performance (25504 B). NOT MATCHED: 4176 words, size 25508/25504, first diff 0x560 (section 1: clamped value a gets s2/s3, retail s4/s5). See PROGRESS.md. */
#include "ac1/ac1.h"
extern void func_800A2D04(void *sl);   /* empty stub (jr ra) */
extern u8 D_80031A58[];
extern u8 D_80031A63_s[] asm("D_80031A63");
#define D_80031A59 (D_80031A58[1])
#define D_80031A5A (D_80031A58[2])
#define D_80031A5B (D_80031A58[3])
#define D_80031A5C (D_80031A58[4])
#define D_80031A5D (D_80031A58[5])
#define D_80031A5E (D_80031A58[6])
#define D_80031A5F (D_80031A58[7])
#define D_80031A60 (D_80031A58[8])
#define D_80031A61 (D_80031A58[9])
#define D_80031A63 (D_80031A58[11])
extern u16 D_80039CA0;
extern u8 D_80039D20;
typedef struct { s16 vx, vy; } DVEC_;
extern u8 D_800411F4[];
extern u8 D_80048664;

typedef struct { u8 key; u8 pad[3]; u8 *l1; u8 *l2; } FrameTab;
extern FrameTab D_800BCFD8[];
typedef union { u8 b[0x3C]; u16 h[0x3C / 2]; s16 sh[0x3C / 2]; s8 sb[0x3C]; } R3C;
typedef union { u8 b[0x34]; u16 h[0x34 / 2]; s16 sh[0x34 / 2]; s8 sb[0x34]; } R34;
typedef union { u8 b[0x38]; u16 h[0x38 / 2]; s16 sh[0x38 / 2]; s8 sb[0x38]; } R38;
typedef union { u8 b[0x48]; u16 h[0x48 / 2]; s16 sh[0x48 / 2]; s8 sb[0x48]; } R48;
typedef union { u8 b[0x60]; u16 h[0x60 / 2]; s16 sh[0x60 / 2]; s8 sb[0x60]; } R60;
typedef union { u8 b[0x2C]; u16 h[0x2C / 2]; s16 sh[0x2C / 2]; s8 sb[0x2C]; } R2C;
typedef union { u8 b[0x30]; u16 h[0x30 / 2]; s16 sh[0x30 / 2]; s8 sb[0x30]; } R30;
extern R3C D_800B921C[];   /* head */
extern R34 D_800B9E1C[];   /* core */
extern R38 D_800B9474[];   /* arms */
extern R48 D_800B951C[];   /* legs */
extern R60 D_800BACF4[];   /* fcs */
extern R2C D_800BA15C[];   /* 5D */
extern R34 D_800BAB04[];   /* 5C */
extern R34 D_800BA41C[];   /* 5E/5F */
extern R30 D_800BABD4[];   /* 59 */
extern R30 D_800BAFF4[];   /* 60 */
#define TH(i,o) (D_800B921C[i].h[(o) >> 1])
#define TC(i,o) (D_800B9E1C[i].h[(o) >> 1])
#define TA(i,o) (D_800B9474[i].h[(o) >> 1])
#define TL(i,o) (D_800B951C[i].h[(o) >> 1])
#define TL8(i,o) (D_800B951C[i].b[o])
#define TF(i,o) (D_800BACF4[i].h[(o) >> 1])
#define T5D(i,o) (D_800BA15C[i].h[(o) >> 1])
#define T5C(i,o) (D_800BAB04[i].h[(o) >> 1])
#define T5E(i,o) (D_800BA41C[i].h[(o) >> 1])
#define T59(i,o) (D_800BABD4[i].h[(o) >> 1])
#define T60(i,o) (D_800BAFF4[i].h[(o) >> 1])
typedef struct {
    u32 h0 : 16;
    u32 f16 : 1;
    u32 f17 : 1;
    u32 f18 : 1;
    u32 f19 : 1;
    u32 hi : 12;
} RecHdr;
#define R ((RecHdr *)rec)
#define TA8(i,o) (D_800B9474[i].b[o])
#define TH8(i,o) (D_800B921C[i].b[o])
#define T5Cs(i,o) (D_800BAB04[i].sh[(o) >> 1])
#define RB(o) (rec[o])
#define RH(o) (*(u16 *)(rec + (o)))
#define RS(o) (*(s16 *)(rec + (o)))
typedef struct { u8 *p; u16 h4; u16 h6; u16 h8; u16 hA; u16 hC; u16 hE; } FrameRow;
extern FrameRow D_800BD73C[];
#define T59s(i,o) (D_800BABD4[i].sh[(o) >> 1])
#define TLs(i,o) (D_800B951C[i].sh[(o) >> 1])
typedef struct { u8 a; u8 pad; u16 b; u8 c; u8 pad2; } S6;
typedef struct { u8 b0, b1, b2, b3, b4, b5, b6, b7; u16 h8, hA, hC, hE; u8 b10, b11; S6 s12; } Wpn;
extern Wpn D_800BB300[];   /* arm weapons (core id - 8) */
extern Wpn D_800BB3C0[];   /* 5D */
extern Wpn D_800BB540[];   /* back weapons 5E/5F */
typedef struct { u16 h0, h2, h4, h6, h8; s16 hA; } Sub12;
typedef struct { u8 pad0[4]; u8 b4; u8 b5; Sub12 sub[7]; u8 pad5A[6]; } FCRow;
extern FCRow D_800BACF4_f[] asm("D_800BACF4");
typedef struct { s16 h[8]; } B16;
extern B16 D_800BD338[];
typedef struct {
    u8 b0, b1, b2, b3, b4, b5, b6, b7, b8;
    u8 b9;
    s8 bA;
    u8 bB, bC;
    u8 padD;
    u16 hE, h10, h12, h14, h16, h18, h1A, h1C, h1E, h20;
    u8 b22, b23;
    S6 s24;
    B16 v2A;
    u16 pad3A;
} Slot;
#define T5E8(i,o) (D_800BA41C[i].b[o])

void func_8009C964(s32 arg0) {
    u8 *rec; s16 i; u8 *p; u16 speed; double x, a, b, c, dd1, dd2, dd3, dd4, dd5, dd6, dd7, dd8, dd9, dd10, dd11, dd12, dd13, dd14, dd15, dd16, dd17, dd18, dd19, dd20, dd21, dd22, dd23, dd24, dd25, dd26, dd27, dd28, dd29, dd30, dd31, dd32, dd33, dd34, dd35, dd36, dd37, dd38, dd39, dd40, dd41, dd42, bz, tq1, tq2, tq3, tq4, tq5, tq6, tq7, tq8, tq9, tq10; u8 ci; u8 *pp; u8 e5, f5, d5, w5; s16 w; Slot *sl; u8 id; double k; u32 dummy;
    u8 *slot[4];
    s16 rec_i;
    u32 weight;
    u32 sum20;
    u32 sum28;
    u16 nslot;

    rec = (u8 *)(arg0 * 0x39C + (u32)D_800411F4);
    for (i = 0; ; i++) {
        if (D_80031A58[0] == D_800BCFD8[i].key) {
            p = D_800BCFD8[i].l1;
            for (; *p != 255; p++) {
                if (D_80031A58[3] == *p) break;
            }
            if (*p != 255) {
                p = D_800BCFD8[i].l2;
                for (; *p != 255; p++) {
                    if (D_80031A58[11] == *p) break;
                }
                if (*p != 255) {
                    rec[0x27] = i;
                    rec_i = i;
                    break;
                }
            }
        }
    }
    weight = TH(D_80031A5A, 0);
    weight += TC(D_80031A5B, 0);
    weight += TA(D_80031A58[0], 0);
    weight += TL(D_80031A63, 0);
    weight += TF(D_80031A61, 0);
    if (D_80031A5D != 255) weight += T5D(D_80031A5D, 0);
    if (D_80031A5C != 255) weight += T5C(D_80031A5C, 0);
    if (D_80031A5E != 255) weight += T5E(D_80031A5E, 0);
    if (D_80031A5F != 255) weight += T5E(D_80031A5F, 0);
    if (D_80031A59 != 255) weight += T59(D_80031A59, 0);
    if (TL(D_80031A63, 0) + TL(D_80031A63, 0xA) < weight) weight = TL(D_80031A63, 0) + TL(D_80031A63, 0xA);
    sum20 = TH(D_80031A5A, 2) + TC(D_80031A5B, 2);
    sum20 += TL(D_80031A63, 2);
    sum20 += TA(D_80031A58[0], 2);
    sum20 += TF(D_80031A61, 2);
    sum28 = TH(D_80031A5A, 2) + TC(D_80031A5B, 2);
    sum28 += TL(D_80031A63, 0xC);
    sum28 += TA(D_80031A58[0], 2);
    sum28 += TF(D_80031A61, 2);
    if (D_80048664 == 0) {
        R->h0 = 120;
    } else if (D_80048664 == 1) {
        R->h0 = 300;
    } else {
        R->h0 = 0;
    }
    R->f16 = 0;
    R->f17 = 1;
    R->f18 = 1;
    R->f19 = TL8(D_80031A63, 0x1A);
    speed = 9800;
    if (D_80031A63 < 27) speed = T59(D_80031A59, 4);
    dd2 = (double)speed / (double)weight * 48.0 / 2.625;
    dd1 = dd2;
    if (dd1 < 1.0) dd1 = 1.0;
    if (dd1 > 64.0) dd1 = 64.0;
    bz = dd2 * 0.8;
    if (bz < 1.0) bz = 1.0;
    if (bz > 50.0) bz = 50.0;
    dd2 = dd2 * 1.2;
    if (dd2 < 1.0) dd2 = 1.0;
    if (dd2 > 64.0) dd2 = 64.0;
    ci = dd2;
    rec[5] = dd1;
    rec[6] = bz;
    rec[7] = dd1;
    rec[8] = bz;
    rec[9] = ci;
    rec[10] = ci;
    pp = D_80031A63_s;
    {
        u16 v = TL(pp[0], 0x10);
        bz = (double)v * 48.0 / 160.0;
        k = 3573.0;
        dd3 = bz * k / (double)weight;
        if (dd3 < 1.0) dd3 = 1.0;
        if (dd3 > 64.0) dd3 = 64.0;
        rec[0xB] = dd3;
    }
    {
        u8 v = TL8(pp[0], 0x12);
        bz = (double)v * (k / (double)weight);
        if (bz < 0.0) bz = 0.0;
        if (bz > 64.0) bz = 64.0;
        rec[0xC] = bz;
    }
    dd5 = (double)weight * 64.0 / 8000.0;
    if (dd5 < 32.0) dd5 = 32.0;
    if (dd5 > 96.0) dd5 = 96.0;
    rec[0xD] = dd5;
    rec[0xE] = TA8(pp[-11], 0xC);
    rec[0xF] = TA8(pp[-11], 0xD);
    RH(0x10) = T60(pp[-3], 4);
    {
        s32 v;
        if (pp[-10] != 255) {
            v = T59s(pp[-10], 6);
        } else {
            { u32 t; do { t = pp[0]; } while (0); v = TLs(t, 0x1C); }
        }
        dd6 = (double)(u16)v / 10.0;
        if (dd6 < 1.0) dd6 = 1.0;
        RH(0x12) = dd6;
    }
    dd7 = (double)(T60(D_80031A60, 2) - sum28) * 64.0 / 1000.0;
    if (dd7 < 1.0) dd7 = 1.0;
    if (dd7 > 384.0) dd7 = 384.0;
    RH(0x14) = dd7;
    dd8 = (double)(T60(D_80031A60, 2) - sum20);
    dd8 = dd8 * 64.0;
    dd8 = dd8 / 1000.0;
    if (dd8 < 1.0) dd8 = 1.0;
    if (dd8 > 192.0) dd8 = 192.0;
    RH(0x18) = dd8;
    RH(0x16) = T60(D_80031A60, 6);
    if (D_80031A5C == 255) {
        RH(0x1A) = 0;
    } else {
        RH(0x1A) = T5C(D_80031A5C, 4);
    }
    RS(0x1E) = (TH(D_80031A5A, 4) + TA(D_80031A58[0], 4) + TL(D_80031A63, 4) + TC(D_80031A5B, 4)) * 4;
    e5 = D_80031A5E;
    f5 = D_80031A5F;
    if ((e5 >= 24 && e5 < 31) || (f5 >= 24 && f5 < 31)) {
        RH(0x1C) = 0;
        rec[0x23] = 0;
        rec[0x25] = 0;
    } else {
        RH(0x1C) = TH(D_80031A5A, 0x10);
        rec[0x23] = TH8(D_80031A5A, 0x12);
        rec[0x25] = TH8(D_80031A5A, 0x13);
    }
    rec[0x24] = TH8(D_80031A5A, 0xB);
    rec[0x28] = TH8(D_80031A5A, 0xA);
    {
        u32 v = TH(D_80031A5A, 6);
        v += TC(D_80031A5B, 6);
        v += TL(D_80031A63, 6);
        v += TA(D_80031A58[0], 6);
        dd9 = 45.0 - ((D_80039CA0 & 0x80) ? (double)v * 1.15 - 745.0 : (double)(v - 745)) / 50.0;
        if (dd9 < 1.0) dd9 = 1.0;
        if (dd9 > 45.0) dd9 = 45.0;
        rec[0x20] = dd9;
    }
    {
        u32 v = TH(D_80031A5A, 8);
        v += TC(D_80031A5B, 8);
        v += TL(D_80031A63, 8);
        v += TA(D_80031A58[0], 8);
        dd10 = 45.0 - ((D_80039CA0 & 0x100) ? (double)v * 1.15 - 875.0 : (double)(v - 875)) / 50.0;
        if (dd10 < 1.0) dd10 = 1.0;
        if (dd10 > 45.0) dd10 = 45.0;
        rec[0x21] = dd10;
    }
    dd11 = ((D_80039CA0 & 8) ? (double)(TL(D_80031A63, 0x18) + 800) : (double)(u16)TLs(D_80031A63, 0x18)) / 1018.0;
    dd12 = (double)weight / 8000.0;
    dd11 = dd11 * 384.0 * dd12;
    if (dd11 <= 32.0) dd11 = 32.0;
    if (dd11 > 255.0) dd11 = 255.0;
    rec[0x22] = dd11;
    if (D_80031A5C != 255) {
        *(S6 *)(rec + 0x64) = *(S6 *)&T5C(D_80031A5C, 6);
    } else {
        ((S6 *)(rec + 0x64))->a = 0;
        ((S6 *)(rec + 0x64))->b = 0;
        ((S6 *)(rec + 0x64))->c = 0;
    }
    {
        u8 *fp = D_800BD73C[(u16)rec_i].p;
        RS(0x6A) = (1 << fp[0x10]) | (1 << fp[0x11]);
    }
    rec[0x26] = D_800BD73C[(u16)rec_i].h4;
    rec[0x31] = D_80031A5C;
    rec[0x29] = D_80031A58[0];
    rec[0x2A] = D_80031A63;
    rec[0x2B] = D_80031A5A;
    rec[0x2C] = D_80031A5B;
    rec[0x2D] = D_80031A59;
    d5 = D_80031A5D;
    if (d5 == 255) {
        rec[0x2E] = 0;
    } else {
        rec[0x2E] = d5;
    }
    rec[0x2F] = D_80031A5E;
    rec[0x30] = D_80031A5F;
    RH(0x32) = 3;
    RH(0x34) = D_800BD73C[(u16)rec_i].h6;
    RH(0x36) = D_800BD73C[(u16)rec_i].h8;
    RH(0x38) = D_800BD73C[(u16)rec_i].hA;
    RH(0x3A) = D_800BD73C[(u16)rec_i].hC;
    rec[0x54] = D_800BD73C[(u16)rec_i].p[0];
    rec[0x55] = D_800BD73C[(u16)rec_i].p[1];
    rec[0x56] = D_800BD73C[(u16)rec_i].p[2];
    rec[0x57] = D_800BD73C[(u16)rec_i].p[3];
    rec[0x58] = D_800BD73C[(u16)rec_i].p[4];
    rec[0x59] = D_800BD73C[(u16)rec_i].p[5];
    rec[0x5A] = D_800BD73C[(u16)rec_i].p[6];
    rec[0x5B] = D_800BD73C[(u16)rec_i].p[7];
    rec[0x5C] = D_800BD73C[(u16)rec_i].p[8];
    rec[0x5D] = D_800BD73C[(u16)rec_i].p[9];
    rec[0x5E] = D_800BD73C[(u16)rec_i].p[10];
    rec[0x5F] = D_800BD73C[(u16)rec_i].p[11];
    rec[0x60] = D_800BD73C[(u16)rec_i].p[12];
    rec[0x61] = D_800BD73C[(u16)rec_i].p[13];
    rec[0x62] = D_800BD73C[(u16)rec_i].p[14];
    rec[0x63] = D_800BD73C[(u16)rec_i].p[15];
    {
        u8 j = 0;
        while (1) {
            (rec + j)[0x6C] = (D_800BD73C[(u16)rec_i].p + j)[0x22];
            if ((rec + j)[0x6C] == 255) break;
            j++;
        }
    }
    {
        u8 n = 0; if (D_80031A5D != 255) n++;
        if (D_80031A5B >= 8) n++;
        if (D_80031A5E != 255 && !(D_80031A5E >= 24 && D_80031A5E < 31)) n++;
        if (D_80031A5F != 255 && !(D_80031A5F >= 24 && D_80031A5F < 31)) n++;
        rec[0x84] = n;
    }
    nslot = 0;
    if (D_80031A5B >= 8) {
        w = D_80031A5B - 8;
        nslot = 1;
        sl = (Slot *)(rec + 0x86);
        slot[0] = (u8 *)sl;
        if (w == 0 || w == 3 || w == 5) {
            sl->b2 = D_800BD73C[(u16)rec_i].p[0x16];
            sl->b3 = D_800BD73C[(u16)rec_i].p[0x17];
            sl->b4 = D_800BD73C[(u16)rec_i].p[0x18];
            sl->b5 = D_800BD73C[(u16)rec_i].p[0x19];
        } else {
            rec[0x88] = D_800BD73C[(u16)rec_i].p[0x12];
            rec[0x89] = D_800BD73C[(u16)rec_i].p[0x13];
            rec[0x8A] = D_800BD73C[(u16)rec_i].p[0x14];
            rec[0x8B] = D_800BD73C[(u16)rec_i].p[0x15];
        }
        {
            u8 t = D_800BB300[w].b6;
            if (t != 4) {
                if (D_80039CA0 & 0x40) {
                    if (D_800BB300[w].s12.a & 0x10) {
                        dd14 = (double)D_800BACF4_f[D_80031A61].sub[t].hA * 1.15;
                    } else {
                        dd14 = (double)D_800BACF4_f[D_80031A61].sub[t].hA * 0.85;
                    }
                    if (dd14 < 1.0) dd14 = 1.0;
                    if (dd14 > 255.0) dd14 = 255.0;
                    sl->h20 = (s32)dd14;
                } else {
                    sl->h20 = D_800BACF4_f[D_80031A61].sub[t].hA;
                }
            } else {
                sl->h20 = 1;
            }
        }
        sl->b22 = D_800BACF4_f[D_80031A61].b4;
        sl->b0 = D_800BB300[w].b0;
        sl->b1 = D_800BB300[w].b1;
        sl->bA = D_800BB300[w].b3;
        sl->bB = D_800BB300[w].b4;
        sl->bC = D_800BB300[w].b5;
        sl->hE = sl->h10 = D_800BB300[w].h8;
        switch (sl->b0) {
        case 0:
            sl->b9 = D_800BB300[w].b2;
            break;
        case 1:
            if (D_80039CA0 & 4) sl->bC |= 0x80;
            sl->b9 = (0x100 - D_800BACF4_f[D_80031A61].sub[D_800BB300[w].b6].hA) / 8;
            if (sl->b9 == 0) sl->b9 = 1;
            if (sl->b9 > 64) sl->b9 = 64;
            break;
        default:
            {
                s32 m;
                if (D_80039CA0 & 0x10) {
                    m = TL(D_80031A63, 0x18); m += 800;
                } else {
                    m = TL(D_80031A63, 0x18);
                }
                do { dd15 = 1018.0 / (double)m; } while (0);
                dd16 = 8000.0 / (double)weight;
                tq1 = D_800BB300[w].b2 * dd15 * dd16;
                if (tq1 < 1.0) tq1 = 1.0;
                if (tq1 > 64.0) tq1 = 64.0;
                sl->b9 = tq1;
            }
            break;
        }
        if (D_800BB300[w].s12.a & 0x10) {
            sl->h12 = D_800BB300[w].hC;
        } else {
            sl->h12 = D_800BB300[w].hA;
        }
        {
            s16 e = D_800BB300[w].hE;
            s32 x;
            sl->h16 = e;
            x = e;
            if (D_800BACF4_f[D_80031A61].sub[D_800BB300[w].b6].h0 < x) {
                sl->h14 = D_800BACF4_f[D_80031A61].sub[D_800BB300[w].b6].h0;
            } else {
                sl->h14 = e;
            }
        }
        {
            u8 t = D_800BB300[w].b6;
            if (t != 4) {
                sl->h18 = D_800BACF4_f[D_80031A61].sub[t].h2;
                sl->h1A = D_800BACF4_f[D_80031A61].sub[D_800BB300[w].b6].h4;
                sl->h1C = D_800BACF4_f[D_80031A61].sub[D_800BB300[w].b6].h6;
                sl->h1E = D_800BACF4_f[D_80031A61].sub[D_800BB300[w].b6].h8;
            } else {
                sl->h18 = 0;
                sl->h1A = 0;
                sl->h1C = 0;
                sl->h1E = 0;
            }
        }
        if (D_800BB300[w].b10 < sl->b22) sl->b22 = D_800BB300[w].b10;
        sl->b23 = D_800BB300[w].b11;
        sl->s24 = D_800BB300[w].s12;
        if (D_800BB300[w].s12.a & 0x20) {
            if (D_80039CA0 & 0x200) {
                sl->b23 = (double)sl->b23 * 100.0 / 120.0;
            }
            if (D_80039CA0 & 0x400) {
                sl->s24.b = (double)(u16)sl->s24.b * 1.15;
            }
        }
        sl->v2A = D_800BD338[D_800BB300[w].b7];
    }
    id = D_80031A5D;
    if (id != 255) {
        w = id;
        slot[(s16)nslot] = (u8 *)(sl = (Slot *)(rec + ((s16)nslot++ * 0x3C + 0x86)));
        sl->b2 = D_800BD73C[(u16)rec_i].p[0x12];
        sl->b3 = D_800BD73C[(u16)rec_i].p[0x13];
        {
            u8 t = D_800BB3C0[w].b6;
            if (t != 4) {
                if (D_80039CA0 & 0x40) {
                    s16 v;
                    if (D_800BB3C0[w].s12.a & 0x10) {
                        dd18 = (double)D_800BACF4_f[D_80031A61].sub[t].hA * 1.15;
                    } else {
                        dd18 = (double)D_800BACF4_f[D_80031A61].sub[t].hA * 0.85;
                    }
                    v = (s32)dd18;
                    if (v <= 0) v = 1;
                    if (v >= 256) v = 255;
                    sl->h20 = v;
                } else {
                    sl->h20 = D_800BACF4_f[D_80031A61].sub[t].hA;
                }
            } else {
                sl->h20 = 1;
            }
        }
        sl->b22 = D_800BACF4_f[D_80031A61].b4;
        sl->b0 = D_800BB3C0[w].b0;
        sl->b1 = D_800BB3C0[w].b1;
        sl->bA = D_800BB3C0[w].b3;
        sl->bB = D_800BB3C0[w].b4;
        sl->bC = D_800BB3C0[w].b5;
        sl->hE = sl->h10 = D_800BB3C0[w].h8;
        switch (sl->b0) {
        case 0:
            sl->b9 = D_800BB3C0[w].b2;
            break;
        case 1:
            if (D_80039CA0 & 4) sl->bC |= 0x80;
            sl->b9 = (0x100 - D_800BACF4_f[D_80031A61].sub[D_800BB3C0[w].b6].hA) / 8;
            if (sl->b9 == 0) sl->b9 = 1;
            if (sl->b9 > 64) sl->b9 = 64;
            break;
        default:
            {
                s32 m;
                if (D_80039CA0 & 0x10) {
                    m = TL(D_80031A63, 0x18); m += 800;
                } else {
                    m = TL(D_80031A63, 0x18);
                }
                do { dd19 = 1018.0 / (double)m; } while (0);
                dd20 = 8000.0 / (double)weight;
                tq2 = D_800BB3C0[w].b2 * dd19 * dd20;
                if (tq2 < 1.0) tq2 = 1.0;
                if (tq2 > 64.0) tq2 = 64.0;
                sl->b9 = tq2;
            }
            break;
        }
        if (D_800BB3C0[w].s12.a & 0x10) {
            sl->h12 = D_800BB3C0[w].hC;
        } else {
            sl->h12 = D_800BB3C0[w].hA;
        }
        {
            s16 e = D_800BB3C0[w].hE;
            s32 x;
            sl->h16 = e;
            x = e;
            if (D_800BACF4_f[D_80031A61].sub[D_800BB3C0[w].b6].h0 < x) {
                sl->h14 = D_800BACF4_f[D_80031A61].sub[D_800BB3C0[w].b6].h0;
            } else {
                sl->h14 = e;
            }
        }
        {
            u8 t = D_800BB3C0[w].b6;
            if (t != 4) {
                sl->h18 = D_800BACF4_f[D_80031A61].sub[t].h2;
                sl->h1A = D_800BACF4_f[D_80031A61].sub[D_800BB3C0[w].b6].h4;
                sl->h1C = D_800BACF4_f[D_80031A61].sub[D_800BB3C0[w].b6].h6;
                sl->h1E = D_800BACF4_f[D_80031A61].sub[D_800BB3C0[w].b6].h8;
            } else {
                sl->h18 = 0;
                sl->h1A = 0;
                sl->h1C = 0;
                sl->h1E = 0;
            }
        }
        if (D_800BB3C0[w].b10 < sl->b22) sl->b22 = D_800BB3C0[w].b10;
        sl->b23 = D_800BB3C0[w].b11;
        sl->s24 = D_800BB3C0[w].s12;
        if (D_800BB3C0[w].s12.a & 0x20) {
            if (D_80039CA0 & 0x200) {
                sl->b23 = (double)sl->b23 * 100.0 / 120.0;
            }
            if (D_80039CA0 & 0x400) {
                sl->s24.b = (double)(u16)sl->s24.b * 1.15;
            }
        }
        sl->v2A = D_800BD338[D_800BB3C0[w].b7];
    }
    id = D_80031A5E;
    if (id != 255) {
        sl = (Slot *)(rec + ((s16)nslot * 0x3C + 0x86));
        slot[(s16)nslot] = (u8 *)sl;
        w = id;
        if (!(D_80031A5E >= 24 && D_80031A5E < 31)) nslot++;
        {
            u8 t = D_800BB540[w].b6;
            if (t != 4) {
                sl->h20 = D_800BACF4_f[D_80031A61].sub[t].hA;
            } else {
                sl->h20 = 0;
            }
        }
        sl->b22 = D_800BACF4_f[D_80031A61].b4;
        if (D_80039CA0 & 0x40) {
            s16 v;
            dd21 = (double)(s16)sl->h20 * ((D_800BB540[w].s12.a & 0x10) ? 1.15 : 0.85);
            v = (s32)dd21;
            if (v <= 0) v = 1;
            if (v >= 256) v = 255;
            sl->h20 = v;
        }
        {
            u16 v = TL(D_80031A63, 0x14);
            bz = (double)v * 48.0 / 160.0;
            dd22 = bz * 3573.0 / (double)weight;
            if (dd22 < 1.0) dd22 = 1.0;
            if (dd22 > 64.0) dd22 = 64.0;
            sl->b7 = dd22;
        }
        {
            u16 v = TL(D_80031A63, 0x14);
            dd23 = (double)v * (3573.0 / (double)weight);
            if (dd23 < 1.0) dd23 = 1.0;
            if (dd23 > 64.0) dd23 = 64.0;
            sl->b8 = dd23;
        }
        if (w < 12) {
            sl->b2 = D_800BD73C[(u16)rec_i].p[0x1A];
            sl->b3 = D_800BD73C[(u16)rec_i].p[0x1B];
            sl->b0 = D_800BB540[w].b0;
            sl->b1 = D_800BB540[w].b1;
            sl->bA = D_800BB540[w].b3;
            sl->bB = D_800BB540[w].b4;
            sl->bC = D_800BB540[w].b5;
            sl->hE = sl->h10 = D_800BB540[w].h8;
            switch (sl->b0) {
            case 0:
                sl->b9 = D_800BB540[w].b2;
                break;
            case 1:
                if (D_80039CA0 & 4) sl->bC |= 0x80;
                sl->b9 = (0x100 - D_800BACF4_f[D_80031A61].sub[D_800BB540[w].b6].hA) / 8;
                if (sl->b9 == 0) sl->b9 = 1;
                if (sl->b9 > 64) sl->b9 = 64;
                break;
            default:
                {
                    s32 m;
                    if (D_80039CA0 & 0x10) {
                        m = TL(D_80031A63, 0x18); m += 800;
                    } else {
                        m = TL(D_80031A63, 0x18);
                    }
                    do { dd24 = 1018.0 / (double)m; } while (0);
                    dd25 = 8000.0 / (double)weight;
                    tq3 = D_800BB540[w].b2 * dd24 * dd25;
                    if (tq3 < 1.0) tq3 = 1.0;
                    if (tq3 > 64.0) tq3 = 64.0;
                    sl->b9 = tq3;
                }
                break;
            }
            if (D_800BB540[w].s12.a & 0x10) {
                sl->h12 = D_800BB540[w].hC;
            } else {
                sl->h12 = D_800BB540[w].hA;
            }
            {
                s16 e = D_800BB540[w].hE;
                s32 x;
                sl->h16 = e;
                x = e;
                if (D_800BACF4_f[D_80031A61].sub[D_800BB540[w].b6].h0 < x) {
                    sl->h14 = D_800BACF4_f[D_80031A61].sub[D_800BB540[w].b6].h0;
                } else {
                    sl->h14 = e;
                }
            }
            {
                u8 t = D_800BB540[w].b6;
                if (t != 4) {
                    sl->h18 = D_800BACF4_f[D_80031A61].sub[t].h2;
                    sl->h1A = D_800BACF4_f[D_80031A61].sub[D_800BB540[w].b6].h4;
                    sl->h1C = D_800BACF4_f[D_80031A61].sub[D_800BB540[w].b6].h6;
                    sl->h1E = D_800BACF4_f[D_80031A61].sub[D_800BB540[w].b6].h8;
                } else {
                    sl->h18 = 0;
                    sl->h1A = 0;
                    sl->h1C = 0;
                    sl->h1E = 0;
                }
            }
            if (D_800BB540[w].b10 < sl->b22) sl->b22 = D_800BB540[w].b10;
            sl->b23 = D_800BB540[w].b11;
            sl->s24 = D_800BB540[w].s12;
            if (D_800BB540[w].s12.a & 0x20) {
                if (D_80039CA0 & 0x200) {
                    sl->b23 = (double)sl->b23 * 100.0 / 120.0;
                }
                if (D_80039CA0 & 0x400) {
                    sl->s24.b = (double)(u16)sl->s24.b * 1.15;
                }
            }
            sl->v2A = D_800BD338[D_800BB540[w].b7];

        } else if (w < 17) {
            sl->b2 = D_800BD73C[(u16)rec_i].p[0x1A];
            sl->b3 = D_800BD73C[(u16)rec_i].p[0x1B];
            sl->b0 = D_800BB540[w].b0;
            sl->b1 = D_800BB540[w].b1;
            sl->bA = D_800BB540[w].b3;
            sl->bB = D_800BB540[w].b4;
            sl->bC = D_800BB540[w].b5;
            sl->hE = sl->h10 = D_800BB540[w].h8;
            switch (sl->b0) {
            case 0:
                sl->b9 = D_800BB540[w].b2;
                break;
            case 1:
                if (D_80039CA0 & 4) sl->bC |= 0x80;
                sl->b9 = (0x100 - D_800BACF4_f[D_80031A61].sub[D_800BB540[w].b6].hA) / 8;
                if (sl->b9 == 0) sl->b9 = 1;
                if (sl->b9 > 64) sl->b9 = 64;
                break;
            default:
                {
                    s32 m;
                    if (D_80039CA0 & 0x10) {
                        m = TL(D_80031A63, 0x18); m += 800;
                    } else {
                        m = TL(D_80031A63, 0x18);
                    }
                    do { dd26 = 1018.0 / (double)m; } while (0);
                    dd27 = 8000.0 / (double)weight;
                    tq4 = D_800BB540[w].b2 * dd26 * dd27;
                    if (tq4 < 1.0) tq4 = 1.0;
                    if (tq4 > 64.0) tq4 = 64.0;
                    sl->b9 = tq4;
                }
                break;
            }
            if (D_800BB540[w].s12.a & 0x10) {
                sl->h12 = D_800BB540[w].hC;
            } else {
                sl->h12 = D_800BB540[w].hA;
            }
            {
                s16 e = D_800BB540[w].hE;
                s32 x;
                sl->h16 = e;
                x = e;
                if (D_800BACF4_f[D_80031A61].sub[D_800BB540[w].b6].h0 < x) {
                    sl->h14 = D_800BACF4_f[D_80031A61].sub[D_800BB540[w].b6].h0;
                } else {
                    sl->h14 = e;
                }
            }
            {
                u8 t = D_800BB540[w].b6;
                if (t != 4) {
                    sl->h18 = D_800BACF4_f[D_80031A61].sub[t].h2;
                    sl->h1A = D_800BACF4_f[D_80031A61].sub[D_800BB540[w].b6].h4;
                    sl->h1C = D_800BACF4_f[D_80031A61].sub[D_800BB540[w].b6].h6;
                    sl->h1E = D_800BACF4_f[D_80031A61].sub[D_800BB540[w].b6].h8;
                } else {
                    sl->h18 = 0;
                    sl->h1A = 0;
                    sl->h1C = 0;
                    sl->h1E = 0;
                }
            }
            if (D_800BB540[w].b10 < sl->b22) sl->b22 = D_800BB540[w].b10;
            sl->b23 = D_800BB540[w].b11;
            sl->s24 = D_800BB540[w].s12;
            if (D_800BB540[w].s12.a & 0x20) {
                if (D_80039CA0 & 0x200) {
                    sl->b23 = (double)sl->b23 * 100.0 / 120.0;
                }
                if (D_80039CA0 & 0x400) {
                    sl->s24.b = (double)(u16)sl->s24.b * 1.15;
                }
            }
            sl->v2A = D_800BD338[D_800BB540[w].b7];

        } else if (w < 24) {
            if (w == 17 || w == 20) {
                sl->b2 = D_800BD73C[(u16)rec_i].p[0x1C];
                sl->b3 = D_800BD73C[(u16)rec_i].p[0x1D];
            } else {
                sl->b2 = D_800BD73C[(u16)rec_i].p[0x1A];
                sl->b3 = D_800BD73C[(u16)rec_i].p[0x1B];
            }
            sl->b6 = D_800BD73C[(u16)rec_i].p[0x1B];
            sl->b0 = D_800BB540[w].b0;
            sl->b1 = D_800BB540[w].b1;
            switch (sl->b0) {
            case 0:
                sl->b9 = D_800BB540[w].b2;
                break;
            case 1:
                if (D_80039CA0 & 4) sl->bC |= 0x80;
                sl->b9 = (0x100 - D_800BACF4_f[D_80031A61].sub[D_800BB540[w].b6].hA) / 8;
                if (sl->b9 == 0) sl->b9 = 1;
                if (sl->b9 > 64) sl->b9 = 64;
                break;
            default:
                {
                    s32 m;
                    if (D_80039CA0 & 0x10) {
                        m = TL(D_80031A63, 0x18); m += 800;
                    } else {
                        m = TL(D_80031A63, 0x18);
                    }
                    do { dd28 = 1018.0 / (double)m; } while (0);
                    dd29 = 8000.0 / (double)weight;
                    tq5 = D_800BB540[w].b2 * dd28 * dd29;
                    if (tq5 < 1.0) tq5 = 1.0;
                    if (tq5 > 64.0) tq5 = 64.0;
                    sl->b9 = tq5;
                }
                break;
            }
            sl->bA = D_800BB540[w].b3;
            sl->bB = D_800BB540[w].b4;
            sl->bC = D_800BB540[w].b5;
            sl->hE = sl->h10 = D_800BB540[w].h8;
            if (D_800BB540[w].s12.a & 0x10) {
                sl->h12 = D_800BB540[w].hC;
            } else {
                sl->h12 = D_800BB540[w].hA;
            }
            {
                s16 e = D_800BB540[w].hE;
                s32 x;
                sl->h16 = e;
                x = e;
                if (D_800BACF4_f[D_80031A61].sub[D_800BB540[w].b6].h0 < x) {
                    sl->h14 = D_800BACF4_f[D_80031A61].sub[D_800BB540[w].b6].h0;
                } else {
                    sl->h14 = e;
                }
            }
            {
                u8 t = D_800BB540[w].b6;
                if (t != 4) {
                    sl->h18 = D_800BACF4_f[D_80031A61].sub[t].h2;
                    sl->h1A = D_800BACF4_f[D_80031A61].sub[D_800BB540[w].b6].h4;
                    sl->h1C = D_800BACF4_f[D_80031A61].sub[D_800BB540[w].b6].h6;
                    sl->h1E = D_800BACF4_f[D_80031A61].sub[D_800BB540[w].b6].h8;
                } else {
                    sl->h18 = 0;
                    sl->h1A = 0;
                    sl->h1C = 0;
                    sl->h1E = 0;
                }
            }
            if (D_800BB540[w].b10 < sl->b22) sl->b22 = D_800BB540[w].b10;
            sl->b23 = D_800BB540[w].b11;
            sl->s24 = D_800BB540[w].s12;
            if (D_800BB540[w].s12.a & 0x20) {
                if (D_80039CA0 & 0x200) {
                    sl->b23 = (double)sl->b23 * 100.0 / 120.0;
                }
                if (D_80039CA0 & 0x400) {
                    sl->s24.b = (double)(u16)sl->s24.b * 1.15;
                }
            }
            sl->v2A = D_800BD338[D_800BB540[w].b7];

        } else if (w < 31) {
            if (RH(0x1C) < T5E(w, 6)) {
                RH(0x1C) = T5E(w, 6);
                rec[0x23] = T5E8(w, 8);
                rec[0x25] = T5E8(w, 9);
            }
        } else {
            rec[0x30] = D_80031A5E;
            sl->b2 = D_800BD73C[(u16)rec_i].p[0x1A];
            sl->b3 = D_800BD73C[(u16)rec_i].p[0x1B];
            sl->b4 = D_800BD73C[(u16)rec_i].p[0x1E];
            sl->b5 = D_800BD73C[(u16)rec_i].p[0x1F];
            sl->b0 = D_800BB540[w].b0;
            sl->b1 = D_800BB540[w].b1;
            sl->bA = -1;
            sl->bB = 0xFF;
            sl->bC = D_800BB540[w].b5;
            sl->hE = sl->h10 = D_800BB540[w].h8;
            switch (sl->b0) {
            case 0:
                sl->b9 = D_800BB540[w].b2;
                break;
            case 1:
                if (D_80039CA0 & 4) sl->bC |= 0x80;
                sl->b9 = (0x100 - D_800BACF4_f[D_80031A61].sub[D_800BB540[w].b6].hA) / 8;
                if (sl->b9 == 0) sl->b9 = 1;
                if (sl->b9 > 64) sl->b9 = 64;
                break;
            default:
                {
                    s32 m;
                    if (D_80039CA0 & 0x10) {
                        m = TL(D_80031A63, 0x18); m += 800;
                    } else {
                        m = TL(D_80031A63, 0x18);
                    }
                    do { dd30 = 1018.0 / (double)m; } while (0);
                    dd31 = 8000.0 / (double)weight;
                    tq6 = D_800BB540[w].b2 * dd30 * dd31;
                    if (tq6 < 1.0) tq6 = 1.0;
                    if (tq6 > 64.0) tq6 = 64.0;
                    sl->b9 = tq6;
                }
                break;
            }
            if (D_800BB540[w].s12.a & 0x10) {
                sl->h12 = D_800BB540[w].hC;
            } else {
                sl->h12 = D_800BB540[w].hA;
            }
            {
                s16 e = D_800BB540[w].hE;
                s32 x;
                sl->h16 = e;
                x = e;
                if (D_800BACF4_f[D_80031A61].sub[D_800BB540[w].b6].h0 < x) {
                    sl->h14 = D_800BACF4_f[D_80031A61].sub[D_800BB540[w].b6].h0;
                } else {
                    sl->h14 = e;
                }
            }
            {
                u8 t = D_800BB540[w].b6;
                if (t != 4) {
                    sl->h18 = D_800BACF4_f[D_80031A61].sub[t].h2;
                    sl->h1A = D_800BACF4_f[D_80031A61].sub[D_800BB540[w].b6].h4;
                    sl->h1C = D_800BACF4_f[D_80031A61].sub[D_800BB540[w].b6].h6;
                    sl->h1E = D_800BACF4_f[D_80031A61].sub[D_800BB540[w].b6].h8;
                } else {
                    sl->h18 = 0;
                    sl->h1A = 0;
                    sl->h1C = 0;
                    sl->h1E = 0;
                }
            }
            if (D_800BB540[w].b10 < sl->b22) sl->b22 = D_800BB540[w].b10;
            sl->b23 = D_800BB540[w].b11;
            sl->s24 = D_800BB540[w].s12;
            if (D_800BB540[w].s12.a & 0x20) {
                if (D_80039CA0 & 0x200) {
                    sl->b23 = (double)sl->b23 * 100.0 / 120.0;
                }
                if (D_80039CA0 & 0x400) {
                    sl->s24.b = (double)(u16)sl->s24.b * 1.15;
                }
            }
            sl->v2A = D_800BD338[D_800BB540[w].b7];

        }
    }
    id = D_80031A5F;
    if (id != 255) {
        sl = (Slot *)(rec + ((s16)nslot * 0x3C + 0x86));
        slot[(s16)nslot] = (u8 *)sl;
        w = id;
        if (!(D_80031A5F >= 24 && D_80031A5F < 31)) nslot++;
        {
            u8 t = D_800BB540[w].b6;
            if (t != 4) {
                sl->h20 = D_800BACF4_f[D_80031A61].sub[t].hA;
            } else {
                sl->h20 = 0;
            }
        }
        sl->b22 = D_800BACF4_f[D_80031A61].b4;
        if (D_80039CA0 & 0x40) {
            s16 v;
            dd32 = (double)(s16)sl->h20 * ((D_800BB540[w].s12.a & 0x10) ? 1.15 : 0.85);
            v = (s32)dd32;
            if (v <= 0) v = 1;
            if (v >= 256) v = 255;
            sl->h20 = v;
        }
        {
            u16 v = TL(D_80031A63, 0x14);
            bz = (double)v * 48.0 / 160.0;
            dd33 = bz * 3573.0 / (double)weight;
            if (dd33 < 1.0) dd33 = 1.0;
            if (dd33 > 64.0) dd33 = 64.0;
            sl->b7 = dd33;
        }
        {
            u16 v = TL(D_80031A63, 0x14);
            dd34 = (double)v * (3573.0 / (double)weight);
            if (dd34 < 1.0) dd34 = 1.0;
            if (dd34 > 64.0) dd34 = 64.0;
            sl->b8 = dd34;
        }
        if (w < 12) {
            sl->b2 = D_800BD73C[(u16)rec_i].p[0x1E];
            sl->b3 = D_800BD73C[(u16)rec_i].p[0x1F];
            sl->b0 = D_800BB540[w].b0;
            sl->b1 = D_800BB540[w].b1;
            if ((s8)D_800BB540[w].b3 != -1) sl->bA = D_800BB540[w].b3 + 1; else sl->bA = -1;
            if (D_800BB540[w].b4 == 255) sl->bB = 255; else sl->bB = D_800BB540[w].b4 + 1;
            sl->bC = D_800BB540[w].b5;
            sl->hE = sl->h10 = D_800BB540[w].h8;
            switch (sl->b0) {
            case 0:
                sl->b9 = D_800BB540[w].b2;
                break;
            case 1:
                if (D_80039CA0 & 4) sl->bC |= 0x80;
                sl->b9 = (0x100 - D_800BACF4_f[D_80031A61].sub[D_800BB540[w].b6].hA) / 8;
                if (sl->b9 == 0) sl->b9 = 1;
                if (sl->b9 > 64) sl->b9 = 64;
                break;
            default:
                {
                    s32 m;
                    if (D_80039CA0 & 0x10) {
                        m = TL(D_80031A63, 0x18); m += 800;
                    } else {
                        m = TL(D_80031A63, 0x18);
                    }
                    do { dd35 = 1018.0 / (double)m; } while (0);
                    dd36 = 8000.0 / (double)weight;
                    tq7 = D_800BB540[w].b2 * dd35 * dd36;
                    if (tq7 < 1.0) tq7 = 1.0;
                    if (tq7 > 64.0) tq7 = 64.0;
                    sl->b9 = tq7;
                }
                break;
            }
            if (D_800BB540[w].s12.a & 0x10) {
                sl->h12 = D_800BB540[w].hC;
            } else {
                sl->h12 = D_800BB540[w].hA;
            }
            {
                s16 e = D_800BB540[w].hE;
                s32 x;
                sl->h16 = e;
                x = e;
                if (D_800BACF4_f[D_80031A61].sub[D_800BB540[w].b6].h0 < x) {
                    sl->h14 = D_800BACF4_f[D_80031A61].sub[D_800BB540[w].b6].h0;
                } else {
                    sl->h14 = e;
                }
            }
            {
                u8 t = D_800BB540[w].b6;
                if (t != 4) {
                    sl->h18 = D_800BACF4_f[D_80031A61].sub[t].h2;
                    sl->h1A = D_800BACF4_f[D_80031A61].sub[D_800BB540[w].b6].h4;
                    sl->h1C = D_800BACF4_f[D_80031A61].sub[D_800BB540[w].b6].h6;
                    sl->h1E = D_800BACF4_f[D_80031A61].sub[D_800BB540[w].b6].h8;
                } else {
                    sl->h18 = 0;
                    sl->h1A = 0;
                    sl->h1C = 0;
                    sl->h1E = 0;
                }
            }
            if (D_800BB540[w].b10 < sl->b22) sl->b22 = D_800BB540[w].b10;
            sl->b23 = D_800BB540[w].b11;
            sl->s24 = D_800BB540[w].s12;
            if (D_800BB540[w].s12.a & 0x20) {
                if (D_80039CA0 & 0x200) {
                    sl->b23 = (double)sl->b23 * 100.0 / 120.0;
                }
                if (D_80039CA0 & 0x400) {
                    sl->s24.b = (double)(u16)sl->s24.b * 1.15;
                }
            }
            sl->v2A = D_800BD338[D_800BB540[w].b7];

        } else if (w < 17) {
            sl->b2 = D_800BD73C[(u16)rec_i].p[0x1E];
            sl->b3 = D_800BD73C[(u16)rec_i].p[0x1F];
            sl->b0 = D_800BB540[w].b0;
            sl->b1 = D_800BB540[w].b1;
            if ((s8)D_800BB540[w].b3 != -1) sl->bA = D_800BB540[w].b3 + 1; else sl->bA = -1;
            if (D_800BB540[w].b4 == 255) sl->bB = 255; else sl->bB = D_800BB540[w].b4 + 1;
            sl->bC = D_800BB540[w].b5;
            sl->hE = sl->h10 = D_800BB540[w].h8;
            switch (sl->b0) {
            case 0:
                sl->b9 = D_800BB540[w].b2;
                break;
            case 1:
                if (D_80039CA0 & 4) sl->bC |= 0x80;
                sl->b9 = (0x100 - D_800BACF4_f[D_80031A61].sub[D_800BB540[w].b6].hA) / 8;
                if (sl->b9 == 0) sl->b9 = 1;
                if (sl->b9 > 64) sl->b9 = 64;
                break;
            default:
                {
                    s32 m;
                    if (D_80039CA0 & 0x10) {
                        m = TL(D_80031A63, 0x18); m += 800;
                    } else {
                        m = TL(D_80031A63, 0x18);
                    }
                    do { dd37 = 1018.0 / (double)m; } while (0);
                    dd38 = 8000.0 / (double)weight;
                    tq8 = D_800BB540[w].b2 * dd37 * dd38;
                    if (tq8 < 1.0) tq8 = 1.0;
                    if (tq8 > 64.0) tq8 = 64.0;
                    sl->b9 = tq8;
                }
                break;
            }
            if (D_800BB540[w].s12.a & 0x10) {
                sl->h12 = D_800BB540[w].hC;
            } else {
                sl->h12 = D_800BB540[w].hA;
            }
            {
                s16 e = D_800BB540[w].hE;
                s32 x;
                sl->h16 = e;
                x = e;
                if (D_800BACF4_f[D_80031A61].sub[D_800BB540[w].b6].h0 < x) {
                    sl->h14 = D_800BACF4_f[D_80031A61].sub[D_800BB540[w].b6].h0;
                } else {
                    sl->h14 = e;
                }
            }
            {
                u8 t = D_800BB540[w].b6;
                if (t != 4) {
                    sl->h18 = D_800BACF4_f[D_80031A61].sub[t].h2;
                    sl->h1A = D_800BACF4_f[D_80031A61].sub[D_800BB540[w].b6].h4;
                    sl->h1C = D_800BACF4_f[D_80031A61].sub[D_800BB540[w].b6].h6;
                    sl->h1E = D_800BACF4_f[D_80031A61].sub[D_800BB540[w].b6].h8;
                } else {
                    sl->h18 = 0;
                    sl->h1A = 0;
                    sl->h1C = 0;
                    sl->h1E = 0;
                }
            }
            if (D_800BB540[w].b10 < sl->b22) sl->b22 = D_800BB540[w].b10;
            sl->b23 = D_800BB540[w].b11;
            sl->s24 = D_800BB540[w].s12;
            if (D_800BB540[w].s12.a & 0x20) {
                if (D_80039CA0 & 0x200) {
                    sl->b23 = (double)sl->b23 * 100.0 / 120.0;
                }
                if (D_80039CA0 & 0x400) {
                    sl->s24.b = (double)(u16)sl->s24.b * 1.15;
                }
            }
            sl->v2A = D_800BD338[D_800BB540[w].b7];
            func_800A2D04(sl);

        } else if (w < 24) {
            if (w == 17 || w == 20) {
                sl->b2 = D_800BD73C[(u16)rec_i].p[0x20];
                sl->b3 = D_800BD73C[(u16)rec_i].p[0x21];
            } else {
                sl->b2 = D_800BD73C[(u16)rec_i].p[0x1E];
                sl->b3 = D_800BD73C[(u16)rec_i].p[0x1F];
            }
            sl->b6 = D_800BD73C[(u16)rec_i].p[0x1F];
            sl->b0 = D_800BB540[w].b0;
            sl->b1 = D_800BB540[w].b1;
            if ((s8)D_800BB540[w].b3 != -1) sl->bA = D_800BB540[w].b3 + 1; else sl->bA = -1;
            if (D_800BB540[w].b4 == 255) sl->bB = 255; else sl->bB = D_800BB540[w].b4 + 1;
            sl->bC = D_800BB540[w].b5;
            sl->hE = sl->h10 = D_800BB540[w].h8;
            switch (sl->b0) {
            case 0:
                sl->b9 = D_800BB540[w].b2;
                break;
            case 1:
                if (D_80039CA0 & 4) sl->bC |= 0x80;
                sl->b9 = (0x100 - D_800BACF4_f[D_80031A61].sub[D_800BB540[w].b6].hA) / 8;
                if (sl->b9 == 0) sl->b9 = 1;
                if (sl->b9 > 64) sl->b9 = 64;
                break;
            default:
                {
                    s32 m;
                    if (D_80039CA0 & 0x10) {
                        m = TL(D_80031A63, 0x18); m += 800;
                    } else {
                        m = TL(D_80031A63, 0x18);
                    }
                    do { dd39 = 1018.0 / (double)m; } while (0);
                    dd40 = 8000.0 / (double)weight;
                    tq9 = D_800BB540[w].b2 * dd39 * dd40;
                    if (tq9 < 1.0) tq9 = 1.0;
                    if (tq9 > 64.0) tq9 = 64.0;
                    sl->b9 = tq9;
                }
                break;
            }
            if (D_800BB540[w].s12.a & 0x10) {
                sl->h12 = D_800BB540[w].hC;
            } else {
                sl->h12 = D_800BB540[w].hA;
            }
            {
                s16 e = D_800BB540[w].hE;
                s32 x;
                sl->h16 = e;
                x = e;
                if (D_800BACF4_f[D_80031A61].sub[D_800BB540[w].b6].h0 < x) {
                    sl->h14 = D_800BACF4_f[D_80031A61].sub[D_800BB540[w].b6].h0;
                } else {
                    sl->h14 = e;
                }
            }
            {
                u8 t = D_800BB540[w].b6;
                if (t != 4) {
                    sl->h18 = D_800BACF4_f[D_80031A61].sub[t].h2;
                    sl->h1A = D_800BACF4_f[D_80031A61].sub[D_800BB540[w].b6].h4;
                    sl->h1C = D_800BACF4_f[D_80031A61].sub[D_800BB540[w].b6].h6;
                    sl->h1E = D_800BACF4_f[D_80031A61].sub[D_800BB540[w].b6].h8;
                } else {
                    sl->h18 = 0;
                    sl->h1A = 0;
                    sl->h1C = 0;
                    sl->h1E = 0;
                }
            }
            if (D_800BB540[w].b10 < sl->b22) sl->b22 = D_800BB540[w].b10;
            sl->b23 = D_800BB540[w].b11;
            sl->s24 = D_800BB540[w].s12;
            if (D_800BB540[w].s12.a & 0x20) {
                if (D_80039CA0 & 0x200) {
                    sl->b23 = (double)sl->b23 * 100.0 / 120.0;
                }
                if (D_80039CA0 & 0x400) {
                    sl->s24.b = (double)(u16)sl->s24.b * 1.15;
                }
            }
            sl->v2A = D_800BD338[D_800BB540[w].b7];
            func_800A2D04(sl);

        } else if (w < 31) {
            if (RH(0x1C) < T5E(w, 6)) {
                RH(0x1C) = T5E(w, 6);
                rec[0x23] = T5E8(w, 8);
                rec[0x25] = T5E8(w, 9);
            }
        } else {
            rec[0x2F] = D_80031A5F;
            sl->b2 = D_800BD73C[(u16)rec_i].p[0x1A];
            sl->b3 = D_800BD73C[(u16)rec_i].p[0x1B];
            sl->b4 = D_800BD73C[(u16)rec_i].p[0x1E];
            sl->b5 = D_800BD73C[(u16)rec_i].p[0x1F];
            sl->b0 = D_800BB540[w].b0;
            sl->b1 = D_800BB540[w].b1;
            sl->bA = -1;
            sl->bB = 0xFF;
            sl->bC = D_800BB540[w].b5;
            sl->hE = sl->h10 = D_800BB540[w].h8;
            switch (sl->b0) {
            case 0:
                sl->b9 = D_800BB540[w].b2;
                break;
            case 1:
                if (D_80039CA0 & 4) sl->bC |= 0x80;
                sl->b9 = (0x100 - D_800BACF4_f[D_80031A61].sub[D_800BB540[w].b6].hA) / 8;
                if (sl->b9 == 0) sl->b9 = 1;
                if (sl->b9 > 64) sl->b9 = 64;
                break;
            default:
                {
                    s32 m;
                    if (D_80039CA0 & 0x10) {
                        m = TL(D_80031A63, 0x18); m += 800;
                    } else {
                        m = TL(D_80031A63, 0x18);
                    }
                    do { dd41 = 1018.0 / (double)m; } while (0);
                    dd42 = 8000.0 / (double)weight;
                    tq10 = D_800BB540[w].b2 * dd41 * dd42;
                    if (tq10 < 1.0) tq10 = 1.0;
                    if (tq10 > 64.0) tq10 = 64.0;
                    sl->b9 = tq10;
                }
                break;
            }
            if (D_800BB540[w].s12.a & 0x10) {
                sl->h12 = D_800BB540[w].hC;
            } else {
                sl->h12 = D_800BB540[w].hA;
            }
            {
                s16 e = D_800BB540[w].hE;
                s32 x;
                sl->h16 = e;
                x = e;
                if (D_800BACF4_f[D_80031A61].sub[D_800BB540[w].b6].h0 < x) {
                    sl->h14 = D_800BACF4_f[D_80031A61].sub[D_800BB540[w].b6].h0;
                } else {
                    sl->h14 = e;
                }
            }
            {
                u8 t = D_800BB540[w].b6;
                if (t != 4) {
                    sl->h18 = D_800BACF4_f[D_80031A61].sub[t].h2;
                    sl->h1A = D_800BACF4_f[D_80031A61].sub[D_800BB540[w].b6].h4;
                    sl->h1C = D_800BACF4_f[D_80031A61].sub[D_800BB540[w].b6].h6;
                    sl->h1E = D_800BACF4_f[D_80031A61].sub[D_800BB540[w].b6].h8;
                } else {
                    sl->h18 = 0;
                    sl->h1A = 0;
                    sl->h1C = 0;
                    sl->h1E = 0;
                }
            }
            if (D_800BB540[w].b10 < sl->b22) sl->b22 = D_800BB540[w].b10;
            sl->b23 = D_800BB540[w].b11;
            sl->s24 = D_800BB540[w].s12;
            if (D_800BB540[w].s12.a & 0x20) {
                if (D_80039CA0 & 0x200) {
                    sl->b23 = (double)sl->b23 * 100.0 / 120.0;
                }
                if (D_80039CA0 & 0x400) {
                    sl->s24.b = (double)(u16)sl->s24.b * 1.15;
                }
            }
            sl->v2A = D_800BD338[D_800BB540[w].b7];
                func_800A2D04(sl);

        }
    }
    if (D_80039CA0 & 1) rec[0x23] |= 1;
    if (D_80039CA0 & 2) RH(0x32) |= 8;
    if (D_80039CA0 & 0x20) RH(0x10) += 12000;
    if (TH8(D_80031A5A, 0xC) == 1) RH(0x32) &= ~1;
    if (TH8(D_80031A5A, 0xD) == 1) RH(0x32) &= ~2;
    if (D_80031A63 == 21) {
        rec[9] = 26;
        rec[10] = 26;
    } else if (D_80031A63 == 22) {
        rec[9] = 23;
        rec[10] = 23;
    } else if (D_80031A63 == 23) {
        rec[9] = 19;
        rec[10] = 19;
    } else if (D_80031A63 == 26) {
        rec[9] = 24;
        rec[10] = 24;
    } else if (D_80031A63 == 27) {
        rec[9] = 6;
        rec[10] = 6;
    } else if (D_80031A63 == 28) {
        rec[9] = 7;
        rec[10] = 7;
    } else if (D_80031A63 == 30) {
        rec[9] = 13;
        rec[10] = 13;
    } else if (D_80031A63 == 31) {
        rec[9] = 11;
        rec[10] = 11;
    }
    if (D_80039D20 != 0) {
        if (RH(0x1C) == 0) RH(0x1C) = 0x1400;
        rec[0x23] = 1;
        rec[0x25] = 3;
    }
    rec[3] = D_80039D20;
}
