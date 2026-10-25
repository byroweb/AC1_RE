#include "ac1/ac1.h"
#include "m2c_compat.h"

M2C_UNK func_80014728();                 /* extern */
M2C_UNK func_8004CECC(); /* extern */
M2C_UNK func_8004DF88();              /* extern */
M2C_UNK func_8004EB3C();        /* extern */
s32 func_8004EBD0();                          /* extern */
s32 func_8004ECF8();                          /* extern */
M2C_UNK func_8007295C();                     /* extern */
s32 func_80073B0C();                   /* extern */
M2C_UNK func_8007510C();  /* extern */
M2C_UNK func_8007513C(); /* extern */
M2C_UNK func_800751AC(); /* extern */
M2C_UNK func_8007521C(); /* extern */
M2C_UNK func_800752A4(); /* extern */
M2C_UNK func_80075648(); /* extern */
M2C_UNK func_80075D34(); /* extern */
M2C_UNK func_80075EF0(); /* extern */
M2C_UNK func_8007617C(); /* extern */
M2C_UNK func_8007647C(); /* extern */
M2C_UNK func_800764D0(); /* extern */
M2C_UNK func_80077010(); /* extern */
M2C_UNK func_800787BC();                      /* extern */
M2C_UNK func_800789B8();  /* extern */
M2C_UNK func_8007A0F8();             /* extern */
M2C_UNK func_8007A1A8();    /* extern */
M2C_UNK func_8007A240();                      /* extern */
s32 func_8007D6C8();                          /* extern */
s32 ratan2();                               /* extern */
extern SVECTOR D_80039C78;
extern u8 *D_80039D18;
#define GF(T, o) (*(T *)(D_80039D18 + (o)))
extern struct { s32 v; } D_801ADA38;
extern void func_80077FF4();
extern void func_80078070();
extern void func_800780EC();
extern void func_8007853C();
extern void func_80078654();
extern void func_80078688();
extern void func_80078DF4();
extern void func_80079480();
extern void func_80079944();
extern void func_80079D4C();
extern void func_8007A078();


#define B(o) M2C_FIELD(arg0, u8 *, o)
#define H(o) M2C_FIELD(arg0, u16 *, o)
#define SH(o) M2C_FIELD(arg0, s16 *, o)
#define W(o) M2C_FIELD(arg0, s32 *, o)
#define P(o) ((u8 *)arg0 + (o))
#define CLR10() (H(0x36) = H(0x36) & 0xFFEF)
#define SVCOPY() (*(SVECTOR *)P(0xCC) = D_80039C78)

void func_8007D818(void *arg0, s32 arg1) {
    SVECTOR sv;
    s32 mode;
    s32 orig;
    s32 r;

    mode = arg1;
    orig = mode;
    if (H(0x36) & 0x10) {
        mode = B(0x3C);
    }
    if (B(0x64) == 0x16) {
        if (func_8004EBD0(P(0x64)) != 0) {
            func_8004DF88(P(0x64), P(0xCC));
            H(0xCC) = H(0xCC) << 6;
            H(0xCE) = H(0xCE) << 6;
            H(0xD0) = H(0xD0) << 6;
            goto L19;
        }
    } else if (B(0x3D) != 0xC0 && (u32)(mode - 0x60) >= 2) {
        if (B(0x41) != 0x40 || B(0x3D) != 0xB0) {
            if (SH(0x2C) - SH(0xA) < 0x32) goto check2;
            sv.vx = H(8);
            sv.vy = SH(0xA) + 0x32;
            sv.vz = H(0xC);
            D_801ADA38.v = 1;
            r = func_80073B0C(arg0, &sv);
            D_801ADA38.v = 0;
            if (r != 0) goto check2;
            mode = 0x4B;
            H(0xCE) = 0x100;
            H(0xD0) = 0;
            H(0xCC) = 0;
            B(0x3C) = 0x4B;
            B(0x41) = 0x4B;
            CLR10();
        }
        {
            u8 *g = D_80039D18;
            W(0x15C) = 0;
            SH(0x148) = 0;
            *(u16 *)(g + 0x20) &= 0xFFFD;
        }
        switch (B(0x64)) {
        case 0x1: case 0x2: case 0x30:
            func_8007A0F8(arg0, 0x1A);
            return;
        case 0x3: case 0x4: case 0x34:
            func_8007A0F8(arg0, 0x20);
            return;
        case 0x5: case 0x6: case 0x35:
            func_8007A0F8(arg0, 0x21);
            return;
        case 0xE: case 0xF: case 0x31:
            func_8007A0F8(arg0, 0x1B);
            return;
        case 0x10: case 0x11: case 0x36:
            func_8007A0F8(arg0, 0x22);
            return;
        case 0x12: case 0x13: case 0x37:
            func_8007A0F8(arg0, 0x23);
            return;
        case 0x8: case 0x9: case 0x32:
            func_8007A0F8(arg0, 0x1C);
            return;
        case 0xB: case 0xC: case 0x33:
            func_8007A0F8(arg0, 0x1D);
            return;
        case 0x15: {
            u8 v;
            SVECTOR *p;
            p = (SVECTOR *)P(0xCC);
            *p = D_80039C78;
            if (B(0x3D) == 2) {
                func_8007617C(arg0, 0x1E, 1, 0x200, p, &func_80079D4C);
                v = 0x49;
            } else {
                func_8007617C(arg0, 0x1F, 1, 0x200, p, &func_80079D4C);
                v = 0x4A;
            }
            B(0x3C) = v;
            B(0x41) = v;
            return;
        }
        case 0x2D: case 0x2E:
            CLR10();
            func_8007A0F8(arg0, 0x18);
            return;
        }
check2:
        if (GF(s32, 0x8) & GF(u16, 0x29A)) {
            if (B(0x3D) != 0xB0 && GF(u16, 0x210) != 0 && (GF(s32, 0x20) & 3) == 0) {
                GF(s16, 0x12) = 5;
                switch (B(0x64)) {
                case 1: case 2:
                    func_8007A1A8(arg0, 0x30, 0x30);
                    return;
                case 3: case 4:
                    func_8007A1A8(arg0, 0x34, 0x31);
                    return;
                case 5: case 6:
                    func_8007A1A8(arg0, 0x35, 0x32);
                    return;
                case 14: case 15:
                    func_8007A1A8(arg0, 0x31, 0x35);
                    return;
                case 16: case 17:
                    func_8007A1A8(arg0, 0x36, 0x36);
                    return;
                case 18: case 19:
                    func_8007A1A8(arg0, 0x37, 0x37);
                    return;
                case 8: case 9:
                    func_8007A1A8(arg0, 0x32, 0x33);
                    return;
                case 11: case 12:
                    func_8007A1A8(arg0, 0x33, 0x34);
                    return;
                }
            }
        }
    }
    switch (mode) {
    case 0x1:
        switch (B(0x64)) {
        case 2: case 4: case 6: case 9: case 12: case 15: case 17: case 19:
            func_800752A4(arg0, 0x2, 1, 1, 0, 0, &func_800780EC);
            SH(0x72) = 0x200;
            break;
        case 1:
            if (func_8004EBD0(P(0x64)) != 0) {
                func_800752A4(arg0, 0x2, 1, 0, 2, GF(s32, 0x4C), &func_800780EC);
            } else {
                func_800752A4(arg0, 0x1, 1, 0, 0, 0, &func_800780EC);
            }
            break;
        case 3: case 5:
            func_800752A4(arg0, 0x1, 1, 1, 0, 0, &func_800780EC);
            SH(0x72) = 0x200;
            break;
        default:
            if (func_8007D6C8(arg0) != 0) {
                func_800752A4(arg0, 0x1, 1, 0, 0, 0, &func_800780EC);
                SH(0x72) = 0x200;
            }
            break;
        }
        break;
    case 0x2:
        switch (B(0x64)) {
        case 2: case 4: case 6: case 9: case 12: case 15: case 17: case 19:
            func_800752A4(arg0, 0x4, 1, 1, 0, 0, &func_800780EC);
            SH(0x72) = 0x200;
            break;
        case 3:
            if (func_8004EBD0(P(0x64)) != 0) {
                func_800752A4(arg0, 0x4, 1, 0, 2, GF(s32, 0x4C), &func_800780EC);
            } else {
                func_800752A4(arg0, 0x3, 1, 0, 0, 0, &func_800780EC);
            }
            break;
        case 1: case 5:
            func_800752A4(arg0, 0x3, 1, 1, 0, 0, &func_800780EC);
            SH(0x72) = 0x200;
            break;
        default:
            if (func_8007D6C8(arg0) != 0) {
                func_800752A4(arg0, 0x3, 1, 0, 0, 0, &func_800780EC);
                SH(0x72) = 0x200;
            }
            break;
        }
        break;
    case 0x3:
        switch (B(0x64)) {
        case 2: case 4: case 6: case 9: case 12: case 15: case 17: case 19:
            func_800752A4(arg0, 0x6, 1, 1, 0, 0, &func_800780EC);
            SH(0x72) = 0x200;
            break;
        case 5:
            if (func_8004EBD0(P(0x64)) != 0) {
                func_800752A4(arg0, 0x6, 1, 0, 2, GF(s32, 0x4C), &func_800780EC);
            } else {
                func_800752A4(arg0, 0x5, 1, 0, 0, 0, &func_800780EC);
            }
            break;
        case 1: case 3:
            func_800752A4(arg0, 0x5, 1, 1, 0, 0, &func_800780EC);
            SH(0x72) = 0x200;
            break;
        default:
            if (func_8007D6C8(arg0) != 0) {
                func_800752A4(arg0, 0x5, 1, 0, 0, 0, &func_800780EC);
                SH(0x72) = 0x200;
            }
            break;
        }
        break;
    case 0x6:
        switch (B(0x64)) {
        case 2: case 4: case 6: case 9: case 12: case 15: case 17: case 19:
            func_800752A4(arg0, 0xF, 1, 1, 0, 0, &func_800780EC);
            SH(0x72) = 0x200;
            break;
        case 14:
            if (func_8004EBD0(P(0x64)) != 0) {
                func_800752A4(arg0, 0xF, 1, 0, 2, GF(s32, 0x4C), &func_800780EC);
            } else {
                func_800752A4(arg0, 0xE, 1, 0, 0, 0, &func_800780EC);
            }
            break;
        case 16: case 18:
            func_800752A4(arg0, 0xE, 1, 1, 0, 0, &func_800780EC);
            SH(0x72) = 0x200;
            break;
        default:
            if (func_8007D6C8(arg0) != 0) {
                func_800752A4(arg0, 0xE, 1, 0, 0, 0, &func_800780EC);
                SH(0x72) = 0x200;
            }
            break;
        }
        break;
    case 0x7:
        switch (B(0x64)) {
        case 2: case 4: case 6: case 9: case 12: case 15: case 17: case 19:
            func_800752A4(arg0, 0x11, 1, 1, 0, 0, &func_800780EC);
            SH(0x72) = 0x200;
            break;
        case 16:
            if (func_8004EBD0(P(0x64)) != 0) {
                func_800752A4(arg0, 0x11, 1, 0, 2, GF(s32, 0x4C), &func_800780EC);
            } else {
                func_800752A4(arg0, 0x10, 1, 0, 0, 0, &func_800780EC);
            }
            break;
        case 14: case 18:
            func_800752A4(arg0, 0x10, 1, 1, 0, 0, &func_800780EC);
            SH(0x72) = 0x200;
            break;
        default:
            if (func_8007D6C8(arg0) != 0) {
                func_800752A4(arg0, 0x10, 1, 0, 0, 0, &func_800780EC);
                SH(0x72) = 0x200;
            }
            break;
        }
        break;
    case 0x8:
        switch (B(0x64)) {
        case 2: case 4: case 6: case 9: case 12: case 15: case 17: case 19:
            func_800752A4(arg0, 0x13, 1, 1, 0, 0, &func_800780EC);
            SH(0x72) = 0x200;
            break;
        case 18:
            if (func_8004EBD0(P(0x64)) != 0) {
                func_800752A4(arg0, 0x13, 1, 0, 2, GF(s32, 0x4C), &func_800780EC);
            } else {
                func_800752A4(arg0, 0x12, 1, 0, 0, 0, &func_800780EC);
            }
            break;
        case 14: case 16:
            func_800752A4(arg0, 0x12, 1, 1, 0, 0, &func_800780EC);
            SH(0x72) = 0x200;
            break;
        default:
            if (func_8007D6C8(arg0) != 0) {
                func_800752A4(arg0, 0x12, 1, 0, 0, 0, &func_800780EC);
                SH(0x72) = 0x200;
            }
            break;
        }
        break;
    case 0x4:
        switch (B(0x64)) {
        case 2: case 4: case 6: case 9: case 12: case 15: case 17: case 19:
            func_800752A4(arg0, 0x9, 1, 1, 0, 0, &func_800780EC);
            SH(0x72) = 0x200;
            break;
        case 8:
            if (func_8004EBD0(P(0x64)) != 0) {
                func_800752A4(arg0, 0x9, 1, 0, 2, GF(s32, 0x4C), &func_800780EC);
            } else {
                func_800752A4(arg0, 0x8, 1, 0, 0, 0, &func_800780EC);
            }
            break;
        default:
            if (func_8007D6C8(arg0) != 0) {
                func_800752A4(arg0, 0x8, 1, 0, 0, 0, &func_800780EC);
                SH(0x72) = 0x200;
            }
            break;
        }
        break;
    case 0x5:
        switch (B(0x64)) {
        case 2: case 4: case 6: case 9: case 12: case 15: case 17: case 19:
            func_800752A4(arg0, 0xC, 1, 1, 0, 0, &func_800780EC);
            SH(0x72) = 0x200;
            break;
        case 11:
            if (func_8004EBD0(P(0x64)) != 0) {
                func_800752A4(arg0, 0xC, 1, 0, 2, GF(s32, 0x4C), &func_800780EC);
            } else {
                func_800752A4(arg0, 0xB, 1, 0, 0, 0, &func_800780EC);
            }
            break;
        default:
            if (func_8007D6C8(arg0) != 0) {
                func_800752A4(arg0, 0xB, 1, 0, 0, 0, &func_800780EC);
                SH(0x72) = 0x200;
            }
            break;
        }
        break;
    case 0x0:
        if (func_8007D6C8(arg0) != 0) {
            func_8007513C(arg0, 0, 1, &func_80077FF4);
            SH(0x72) = 0x200;
        }
        break;
    case 0x9:
        if (B(0x3D) != 3) {
            if (B(0x64) == 0x15) {
                func_8007510C(arg0, 2, &func_8007853C);
                SH(0x70) = 1;
                M2C_FIELD(arg0, s8 *, 0x66) = 1;
                break;
            }
            if (func_8007D6C8(arg0) == 0) break;
        }
        func_8007521C(arg0, 0x15, 1, 2, GF(u16, 0x214), GF(u16, 0x216), &func_8007853C);
        break;
    case 0xA:
        if (B(0x3D) != 2) {
            if (B(0x64) == 0x15) {
                func_8007510C(arg0, 3, &func_80078654);
                SH(0x70) = 1;
                M2C_FIELD(arg0, s8 *, 0x66) = 1;
                break;
            }
            if (func_8007D6C8(arg0) == 0) break;
        }
        func_8007521C(arg0, 0x15, 1, 3, -GF(u16, 0x214), -GF(u16, 0x216), &func_80078654);
        break;
    case 0x40:
        if (B(0x3D) != 0xC0) {
            if (func_8007D6C8(arg0) != 0) {
                if (B(0x64) == 0x38) {
                    func_80075648(arg0, 0x16, 1, 0xFF, GF(s32, 0x4C) + 0xC, &func_800780EC);
                    SH(0x72) = 0x400;
                } else {
                    func_80075648(arg0, 0x16, 1, 0xFF, GF(s32, 0x4C) + 0xC, &func_800780EC);
                }
            }
        } else {
L19:
            func_8007617C(arg0, 0x19, 1, 0x200, P(0xCC), &func_80079D4C);
        }
        break;
    case 0x41:
        func_8007617C(arg0, 0x1A, 1, 0x200, P(0xCC), &func_80079D4C);
        break;
    case 0x42:
        func_8007617C(arg0, 0x20, 1, 0x200, P(0xCC), &func_80079D4C);
        break;
    case 0x43:
        func_8007617C(arg0, 0x21, 1, 0x200, P(0xCC), &func_80079D4C);
        break;
    case 0x44:
        func_8007617C(arg0, 0x1B, 1, 0x200, P(0xCC), &func_80079D4C);
        break;
    case 0x45:
        func_8007617C(arg0, 0x22, 1, 0x200, P(0xCC), &func_80079D4C);
        break;
    case 0x46:
        func_8007617C(arg0, 0x23, 1, 0x200, P(0xCC), &func_80079D4C);
        break;
    case 0x47:
        func_8007617C(arg0, 0x1C, 1, 0x200, P(0xCC), &func_80079D4C);
        break;
    case 0x48:
        func_8007617C(arg0, 0x1D, 1, 0x200, P(0xCC), &func_80079D4C);
        break;
    case 0x49:
        func_8007617C(arg0, 0x1E, 1, 0x200, P(0xCC), &func_80079D4C);
        break;
    case 0x4A:
        func_8007617C(arg0, 0x1F, 1, 0x200, P(0xCC), &func_80079D4C);
        break;
    case 0x4B:
        func_8007617C(arg0, 0x18, 1, 0x200, P(0xCC), &func_80079D4C);
        break;
    case 0x60:
        func_800787BC(arg0);
        if (B(0x64) != 0x17) {
            W(0x15C) = 0;
            SH(0x148) = 0;
            func_8004CECC(0xE, P(8), 0x3C, 0);
        }
        switch (orig) {
        case 1:
            func_800752A4(arg0, 0x2, 1, 0, 2, GF(s32, 0x4C), &func_800780EC);
            mode = orig;
            SH(0x72) = 0x200;
            CLR10();
            break;
        case 2:
            func_800752A4(arg0, 0x4, 1, 0, 2, GF(s32, 0x4C), &func_800780EC);
            mode = orig;
            SH(0x72) = 0x200;
            CLR10();
            break;
        case 3:
            func_800752A4(arg0, 0x6, 1, 0, 2, GF(s32, 0x4C), &func_800780EC);
            mode = orig;
            SH(0x72) = 0x200;
            CLR10();
            break;
        case 6:
            func_800752A4(arg0, 0xF, 1, 0, 2, GF(s32, 0x4C), &func_800780EC);
            mode = orig;
            SH(0x72) = 0x200;
            CLR10();
            break;
        case 7:
            func_800752A4(arg0, 0x11, 1, 0, 2, GF(s32, 0x4C), &func_800780EC);
            mode = orig;
            SH(0x72) = 0x200;
            CLR10();
            break;
        case 8:
            func_800752A4(arg0, 0x13, 1, 0, 2, GF(s32, 0x4C), &func_800780EC);
            mode = orig;
            SH(0x72) = 0x200;
            CLR10();
            break;
        case 4:
            func_800752A4(arg0, 0x9, 1, 0, 2, GF(s32, 0x4C), &func_800780EC);
            mode = orig;
            SH(0x72) = 0x200;
            CLR10();
            break;
        case 5:
            func_800752A4(arg0, 0xC, 1, 0, 2, GF(s32, 0x4C), &func_800780EC);
            mode = orig;
            SH(0x72) = 0x200;
            CLR10();
            break;
        case 9:
            mode = orig;
            func_8007521C(arg0, 0x15, 1, 2, GF(u16, 0x214), GF(u16, 0x216), &func_8007853C);
            CLR10();
            break;
        case 10:
            mode = orig;
            func_8007521C(arg0, 0x15, 1, 3, -GF(u16, 0x214), -GF(u16, 0x216), &func_80078654);
            CLR10();
            break;
        default:
            if (B(0x64) == 0x17) {
                if (func_8004ECF8(P(0x64)) != 0) {
                    CLR10();
                    func_8007513C(arg0, 0, 1, &func_80077FF4);
                } else {
                    func_800751AC(arg0, 0x17, 1, &func_80078070);
                }
            } else {
                func_800751AC(arg0, 0x17, 1, &func_80078070);
                SVCOPY();
            }
            SH(0x72) = 0x200;
            break;
        }
        break;
    case 0x61:
        if (B(0x64) == 0x24) {
            if (func_8004ECF8(P(0x64)) != 0) {
                CLR10();
                func_8007513C(arg0, 0, 1, &func_80077FF4);
            } else {
                func_800751AC(arg0, 0x24, 1, &func_80078070);
            }
        } else {
            W(0x15C) = 0;
            SH(0x148) = 0;
            func_800751AC(arg0, 0x24, 1, &func_80078070);
            func_8004EB3C(P(0x64), 1, GF(s32, 0x4C) + 8);
            SVCOPY();
            if (SH(0x2E) <= SH(0xA) && SH(0x2E) >= SH(0xA) - H(0x2A)) {
                func_80077010(arg0, 0x64, 5, 0x12C, 0x2BC, 0x64, 0x12C, 0x40);
            }
        }
        break;
    case 0x30:
        func_80075EF0(arg0, 0x30, 1, 0x200, P(0xCC), &func_80079944);
        break;
    case 0x31:
        func_80075EF0(arg0, 0x34, 1, 0x200, P(0xCC), &func_80079944);
        break;
    case 0x32:
        func_80075EF0(arg0, 0x35, 1, 0x200, P(0xCC), &func_80079944);
        break;
    case 0x35:
        func_80075EF0(arg0, 0x31, 1, 0x200, P(0xCC), &func_80079944);
        break;
    case 0x36:
        func_80075EF0(arg0, 0x36, 1, 0x200, P(0xCC), &func_80079944);
        break;
    case 0x37:
        func_80075EF0(arg0, 0x37, 1, 0x200, P(0xCC), &func_80079944);
        break;
    case 0x33:
        func_80075EF0(arg0, 0x32, 1, 0x200, P(0xCC), &func_80079944);
        break;
    case 0x34:
        func_80075EF0(arg0, 0x33, 1, 0x200, P(0xCC), &func_80079944);
        break;
    case 0x38: {
        s16 x;
        x = SH(0xCC);
        if (abs(x) >= 0x15 || abs(SH(0xD0)) >= 0x15) {

        s32 a;
        a = ratan2(x, SH(0xD0));
        r = SH(0x12);
        r = -a - r;
        switch ((r + 0x100) >> 9 & 7) {
        case 0:
            func_80075EF0(arg0, 0x31, 1, 0x200, P(0xCC), &func_80079944);
            break;
        case 1:
            func_80075EF0(arg0, 0x37, 1, 0x200, P(0xCC), &func_80079944);
            break;
        case 2:
            func_80075EF0(arg0, 0x33, 1, 0x200, P(0xCC), &func_80079944);
            break;
        case 3:
            func_80075EF0(arg0, 0x35, 1, 0x200, P(0xCC), &func_80079944);
            break;
        case 4:
            func_80075EF0(arg0, 0x30, 1, 0x200, P(0xCC), &func_80079944);
            break;
        case 5:
            func_80075EF0(arg0, 0x34, 1, 0x200, P(0xCC), &func_80079944);
            break;
        case 6:
            func_80075EF0(arg0, 0x32, 1, 0x200, P(0xCC), &func_80079944);
            break;
        case 7:
            func_80075EF0(arg0, 0x36, 1, 0x200, P(0xCC), &func_80079944);
            break;
        }
        } else {
            func_8007513C(arg0, 0, 1, &func_80077FF4);
            func_800787BC(arg0);
            SH(0x72) = 0x200;
        }
        break;
    }
    case 0x20:
        if (B(0x64) == 0x38 && func_8004ECF8(P(0x64)) != 0) {
            CLR10();
            func_8007A240();
        }
        func_8007295C(1);
        if (B(0x64) != 0x38 || SH(0x6E) < 7) {
            func_800752A4(arg0, 0x38, 1, 0, 0, 0, &func_80078DF4);
        } else {
            if (SH(0x6E) == 7) {
                func_80014728(SH(0x12), P(0xCC));
            }
            func_800789B8(arg0, 0x38, &func_80078DF4);
        }
        func_8007295C(0);
        SH(0x72) = 0x400;
        break;
    case 0x21:
        if (B(0x64) == 0x2F && func_8004ECF8(P(0x64)) != 0) {
            CLR10();
            func_8007A240();
        }
        func_8007617C(arg0, 0x2F, 1, 0x400, P(0xCC), &func_80079480);
        break;
    case 0x22:
        if ((u32)(B(0x64) - 0x2D) >= 2) {
            SVCOPY();
        }
        func_80075D34(arg0, GF(u8, 0x1A), 1, P(0xCC), &func_80078688);
        break;
    case 0x23:
        if (SH(0x70) < 0) {
            if (func_8004ECF8(P(0x64)) != 0) {
                CLR10();
            }
            func_80075D34(arg0, GF(u8, 0x1A), -1, P(0xCC), &func_80078688);
        } else {
            func_80075D34(arg0, GF(u8, 0x1A), 1, P(0xCC), &func_80078688);
            SH(0x70) = -1;
            H(0x6A) = H(0x28);
            H(0x6C) = H(0x2A);
        }
        break;
    case 0x80:
        func_800764D0(arg0, B(0x64), 1, 0x400, P(0xCC), -1, &func_8007A078);
        break;
    case 0x81:
        func_8007647C(arg0, B(0x64), 0x400, P(0xCC), -1, &func_80079D4C);
        break;
    }
end:
    if (!(H(0x36) & 0x10)) {
        B(0x3C) = mode;
    }
}
