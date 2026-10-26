#include "ac1/ac1.h"
/* ovl201 0x80078D84: per-frame update of the AC assembly screen, installed as
   o->update by func_80078AAC. Pad L/R steps the part category (skipping empty
   tables and the arm/back-weapon conflicts), up/down (0x2000/0x8000) selects a
   part, 0x40 confirms (with the frame-compatibility messages), 0x20 cancels. */

typedef struct AsmWork {
    void *src;          /* +0x00 (+0x7C) */
    UiObj *owner;       /* +0x04 */
    u8 b84, b85;        /* +0x08 */
    u8 b86, b87;        /* +0x0A */
    UiObj *sub;         /* +0x0C */
    u8 pad8c[4];
    u16 h90;            /* +0x14 table index */
    u16 h92;            /* +0x16 part category */
    s16 h94;            /* +0x18 */
    u8 b96;             /* +0x1A */
    u8 b97;
    u8 b98, b99, b9a;
    u8 pad9b;
    UiObj *bar;         /* +0x20 */
    u8 bA0, bA1;        /* +0x24 */
    s16 hA2, hA4;       /* +0x26 */
    u16 hA6;            /* +0x2A part index, 255 = none */
    u16 hA8;            /* +0x2C */
    u8 padAA;
    u8 bAB;             /* +0x2F */
    u16 hAC;            /* +0x30 */
    u8 bAE;             /* +0x32 */
    u8 padAF;
    u16 hB0;            /* +0x34 */
} AsmWork;

typedef struct AsmObj {
    u8 head[0x7C];
    AsmWork w;
} AsmObj;

typedef struct PartTab {
    u16 h0;
    u16 base;
    s32 *list;
    u8 pad[8];
} PartTab;

typedef struct Rec56 { u8 b[56]; } Rec56;
typedef struct Rec44 { u8 b[44]; } Rec44;

extern u32 D_801A2568;
extern u32 D_801A256C;
extern u32 D_801A2550;
extern s16 D_801A2318;
extern u8 D_80031A58[];
extern u16 D_80039CA0;
extern u16 D_800B8228[];
extern char *D_800B824C[];
extern PartTab D_800BCAB8[];
extern Rec56 D_800B9482[];
extern Rec44 D_800BB144[];
extern UiObj *D_8017EF80[7];
extern u8 D_8004C060[], D_8004C084[], D_8004C0A8[], D_8004C0CC[], D_8004C0EC[];
extern u8 D_8004C100[], D_8004C11C[], D_8004C140[], D_8004C164[], D_8004C180[];
extern u8 D_8004C19C[], D_8004C1B4[], D_8004C1D8[], D_8004C1F8[], D_8004C20C[];

extern void SsUtKeyOn(s16 a, s32 b, s32 c, s32 d, s32 e, s32 f, s32 g);
extern void func_800506EC(s32);
extern void func_80050AF4(s32, s16);
extern void func_80051450(s32, s32);
extern void func_8005C2D0(s32, s32, s32, s32, s32);
extern void func_8005D15C(char *s, s32 x, s32 y);
extern void func_8005DED8(UiObj *, void (*)(UiObj *), void *, s32);
extern void func_800740AC(UiObj *);
extern void func_8007437C(u8 *, u8 *);
extern void func_80075520(void);
extern void func_80075898(UiObj *);
extern void func_800788C8(s16, u8);
extern void func_80078A38(UiObj *);
extern u8 func_8007AA04(u8);
extern void func_8007AAF0(UiObj *);
extern void func_8007AC74(UiObj *);
extern void func_8007BA10(UiObj *);
extern void func_8007BB48(UiObj *);
extern void func_8007C22C(UiObj *);
extern void func_8007C240(UiObj *);
extern u16 func_8007C2F0(UiObj *, s32);
extern u8 func_80083C00(s32, u16);
void func_80078D84(UiObj *o);

void func_80078D84(UiObj *o)
{
    AsmWork *w;
    AsmWork *w2;
    s16 idx;
    s32 *lst;
    u8 c;
    u16 n1, k1, n2, k2;
    u8 kA, kB;
    u8 m;
    u8 r;
    u16 q;
    char *nm;
    u16 mask;
    s16 i;
    u8 t;
    u16 j;
    u8 fa, fb;
    s32 e;
    u32 *pad;

    func_800740AC(o);
    w = &((AsmObj *)o)->w;
    func_80075520();
    func_80075898(o);
    if (w->bar->update != func_8007C22C)
        return;

    if (((D_801A2568 & 0x1000) && !(D_801A256C & 0x1000)) ||
        ((D_801A2568 & 0x4000) && !(D_801A256C & 0x4000))) {
        w2 = &((AsmObj *)o)->w;
        if (((AsmObj *)o)->w.b84) {
            ((AsmObj *)o)->w.b84 = 0;
            ((AsmObj *)o)->w.b85 = 0;
            obj_find_by_handle(func_8007437C)->kill = 1;
        }
        idx = ((AsmObj *)o)->w.b96;
        if (D_801A2568 & 0x1000) {
            SsUtKeyOn(D_801A2318, 0, 12, 60, 0, 127, 127);
            if (idx == 9 && D_80031A58[7] >= 31 && D_80031A58[7] != 255)
                idx = 8;
            else if (idx == 10 && D_80031A58[6] >= 31 && D_80031A58[6] != 255)
                idx = 9;
            for (;;) {
                idx--;
                if (idx < 0)
                    idx = 11;
                lst = D_800BCAB8[D_800B8228[idx]].list;
                if (lst[0] == 0 && lst[1] == 0)
                    continue;
                if (idx == 8 && D_80031A58[7] >= 31 && D_80031A58[7] != 255)
                    continue;
                if (idx == 9 && D_80031A58[6] >= 31 && D_80031A58[6] != 255)
                    idx = 8;
                break;
            }
            nm = D_800B824C[idx];
        } else {
            SsUtKeyOn(D_801A2318, 0, 12, 60, 0, 127, 127);
            if (idx == 8 && D_80031A58[6] >= 31 && D_80031A58[6] != 255)
                idx = 9;
            else if (idx == 7 && D_80031A58[7] >= 31 && D_80031A58[7] != 255)
                idx = 8;
            for (;;) {
                if ((u16)++idx >= 12)
                    idx = 0;
                lst = D_800BCAB8[D_800B8228[idx]].list;
                if (lst[0] == 0 && lst[1] == 0)
                    continue;
                if (idx == 8 && D_80031A58[7] >= 31 && D_80031A58[7] != 255)
                    idx = 9;
                if (idx == 9 && D_80031A58[6] >= 31 && D_80031A58[6] != 255)
                    continue;
                break;
            }
            nm = D_800B824C[idx];
        }
        func_8005D15C(nm, 960, 0);
        c = D_80031A58[w2->h92];
        if (c != 255 && w2->hA6 != c)
            func_80051450(w2->bAB, (u16)(w2->hAC + c));
        if (c == 255 || D_80031A58[w2->h92] == 255) {
            if (w2->hA6 != 255) {
                switch (w2->h92) {
                case 0:
                case 1:
                case 2:
                case 3:
                case 8:
                case 9:
                case 11:
                    func_80051450(w2->bAB, (u16)(w2->hAC + func_8007AA04(w2->h92)));
                    break;
                }
            }
            e = D_80031A58[w2->h92];
            if (e == 255 && w2->hA6 != e) {
                switch (w2->h92) {
                case 5:
                    if (w2->b86 != e)
                        func_80051450(w2->bAB, (u16)(w2->hAC + w2->b86));
                    break;
                case 4:
                    if (w2->b87 != e)
                        func_80051450(w2->bAB, (u16)(w2->hAC + w2->b87));
                    break;
                }
            }
        }
        w2->bA0 = 67;
        w2->hA2 = w2->bar->rot.vy;
        w2->hA4 = w2->bar->w.panel.rank;
        w2->bar->kill = 1;
        w2->bar = &g_ui_none;
        w2->b96 = idx;
        o->update = func_8007AAF0;
        return;
    }

    pad = &D_801A2568;
    if (*pad & 0x2000) {
        k1 = w->hA6;
        k1 = (u16)k1;
        SsUtKeyOn(D_801A2318, 0, 12, 60, 0, 127, 127);
        if (w->b84) {
            w->b84 = 0;
            w->b85 = 0;
            obj_find_by_handle(func_8007437C)->kill = 1;
        }
        if (*pad & 0xA) {
            switch (w->h92) {
            case 1:
            case 2:
            case 4:
            case 5:
            case 8:
            case 9:
                if (w->hA6 == 255)
                    goto sel_l;
                n1 = w->hA6;
                break;
            case 6:
            case 7:
                if (w->hA6 == 255) {
                    n1 = func_8007C2F0(o, 78);
                    break;
                }
                if (w->hA6 < 12) {
                    while (w->hA6 < (n1 = func_8007C2F0(o, 78)) && n1 < 12)
                        w->hA6 = n1;
                } else if (w->hA6 < 17) {
                    while (w->hA6 < (n1 = func_8007C2F0(o, 78)) && n1 < 17)
                        w->hA6 = n1;
                } else if (w->hA6 < 24) {
                    while (w->hA6 < (n1 = func_8007C2F0(o, 78)) && n1 < 24)
                        w->hA6 = n1;
                } else if (w->hA6 < 31) {
                    while (w->hA6 < (n1 = func_8007C2F0(o, 78)) && n1 < 31)
                        w->hA6 = n1;
                } else {
                    n1 = w->hA6;
                }
                break;
            case 3:
                if (w->hA6 == 255) {
                    n1 = func_8007C2F0(o, 78);
                    break;
                }
                if (w->hA6 < 8) {
                    while (w->hA6 < (n1 = func_8007C2F0(o, 78)) && n1 < 8)
                        w->hA6 = n1;
                } else {
                    n1 = w->hA6;
                }
                break;
            case 11:
                if (w->hA6 == 255) {
                    n1 = func_8007C2F0(o, 78);
                    break;
                }
                if (w->hA6 < 14) {
                    while (w->hA6 < (n1 = func_8007C2F0(o, 78)) && n1 < 14)
                        w->hA6 = n1;
                } else if (w->hA6 < 21) {
                    while (w->hA6 < (n1 = func_8007C2F0(o, 78)) && n1 < 21)
                        w->hA6 = n1;
                } else if (w->hA6 < 27) {
                    while (w->hA6 < (n1 = func_8007C2F0(o, 78)) && n1 < 27)
                        w->hA6 = n1;
                } else {
                    n1 = w->hA6;
                }
                break;
            case 0:
                n1 = func_8007C2F0(o, 78);
                break;
            }
        } else {
        sel_l:
            n1 = func_8007C2F0(o, 78);
        }
        w->hA6 = k1;
        if (w->bA1 == 2)
            w->bA1 = 1;
        else
            w->bA1 = 2;
        switch (w->h92) {
        default:
            if (w->hA6 != 255 && w->hA6 >= n1) {
                func_80050AF4(w->bA1, 2);
                w->hA8 = 255;
            } else {
                func_80050AF4(w->bA1, (s16)(D_800BCAB8[w->h90].base + n1));
                func_80051450(w->bAB, (u16)(w->hAC + n1));
                w->hA8 = n1;
            }
            break;
        case 1:
        case 2:
        case 3:
        case 8:
        case 9:
        case 11:
            if (w->hA6 != 255 && w->hA6 >= n1) {
                kA = func_8007AA04(w->h92);
                func_80050AF4(w->bA1, 2);
                if (kA != 255) {
                    func_80050AF4(17, (s16)(D_800BCAB8[w->h90].base + kA));
                    func_80051450(w->bAB, (u16)(w->hAC + kA));
                }
                w->hA8 = 255;
            } else {
                func_80050AF4(w->bA1, (s16)(D_800BCAB8[w->h90].base + n1));
                func_80051450(w->bAB, (u16)(w->hAC + n1));
                w->hA8 = n1;
            }
            break;
        case 0:
            func_80050AF4(w->bA1, (s16)(D_800BCAB8[w->h90].base + n1));
            func_80051450(w->bAB, (u16)(w->hAC + n1));
            w->hA8 = n1;
            break;
        }
        if (D_801A2568 & 1)
            w->bar->w.panel.rank = -540;
        else if (D_801A2568 & 4)
            w->bar->w.panel.rank = -360;
        else
            w->bar->w.panel.rank = -180;
        w->bar->update = func_8007C240;
        w->bA0 = 82;
        o->update = func_80078A38;
        return;
    }

    if (*pad & 0x8000) {
        k2 = w->hA6;
        SsUtKeyOn(D_801A2318, 0, 12, 60, 0, 127, 127);
        if (w->b84) {
            w->b84 = 0;
            w->b85 = 0;
            obj_find_by_handle(func_8007437C)->kill = 1;
        }
        if (*pad & 0xA) {
            switch (w->h92) {
            case 1:
            case 2:
            case 4:
            case 5:
            case 8:
            case 9:
                if (w->hA6 == 255)
                    goto sel_r;
                n2 = w->hA6;
                break;
            case 6:
            case 7:
                if (w->hA6 == 255) {
                    n2 = func_8007C2F0(o, 80);
                    break;
                }
                if (w->hA6 >= 31) {
                    while ((n2 = func_8007C2F0(o, 80)) < w->hA6 && n2 >= 31)
                        w->hA6 = n2;
                } else if (w->hA6 >= 24) {
                    while ((n2 = func_8007C2F0(o, 80)) < w->hA6 && n2 >= 24)
                        w->hA6 = n2;
                } else if (w->hA6 >= 17) {
                    while ((n2 = func_8007C2F0(o, 80)) < w->hA6 && n2 >= 17)
                        w->hA6 = n2;
                } else if (w->hA6 >= 12) {
                    while ((n2 = func_8007C2F0(o, 80)) < w->hA6 && n2 >= 12)
                        w->hA6 = n2;
                } else {
                    n2 = w->hA6;
                }
                break;
            case 3:
                if (w->hA6 == 255) {
                    n2 = func_8007C2F0(o, 80);
                    break;
                }
                if (w->hA6 >= 8) {
                    while ((n2 = func_8007C2F0(o, 80)) < w->hA6 && n2 >= 8)
                        w->hA6 = n2;
                } else {
                    n2 = w->hA6;
                }
                break;
            case 11:
                if (w->hA6 == 255) {
                    n2 = func_8007C2F0(o, 80);
                    break;
                }
                if (w->hA6 >= 27) {
                    while ((n2 = func_8007C2F0(o, 80)) < w->hA6 && n2 >= 27)
                        w->hA6 = n2;
                } else if (w->hA6 >= 21) {
                    while ((n2 = func_8007C2F0(o, 80)) < w->hA6 && n2 >= 21)
                        w->hA6 = n2;
                } else if (w->hA6 >= 14) {
                    while ((n2 = func_8007C2F0(o, 80)) < w->hA6 && n2 >= 14)
                        w->hA6 = n2;
                } else {
                    n2 = w->hA6;
                }
                break;
            case 0:
                n2 = func_8007C2F0(o, 80);
                break;
            }
        } else {
        sel_r:
            n2 = func_8007C2F0(o, 80);
        }
        w->hA6 = k2;
        if (w->bA1 == 2)
            w->bA1 = 1;
        else
            w->bA1 = 2;
        switch (w->h92) {
        default:
            if (w->hA6 != 255 && n2 >= w->hA6) {
                func_80050AF4(w->bA1, 2);
                w->hA8 = 255;
            } else {
                func_80050AF4(w->bA1, (s16)(D_800BCAB8[w->h90].base + n2));
                func_80051450(w->bAB, (u16)(w->hAC + n2));
                w->hA8 = n2;
            }
            break;
        case 1:
        case 2:
        case 3:
        case 8:
        case 9:
        case 11:
            if (w->hA6 != 255 && n2 >= w->hA6) {
                kB = func_8007AA04(w->h92);
                func_80050AF4(w->bA1, 2);
                if (kB != 255) {
                    func_80050AF4(17, (s16)(D_800BCAB8[w->h90].base + kB));
                    func_80051450(w->bAB, (u16)(w->hAC + kB));
                }
                w->hA8 = 255;
            } else {
                func_80050AF4(w->bA1, (s16)(D_800BCAB8[w->h90].base + n2));
                func_80051450(w->bAB, (u16)(w->hAC + n2));
                w->hA8 = n2;
            }
            break;
        case 0:
            func_80050AF4(w->bA1, (s16)(D_800BCAB8[w->h90].base + n2));
            func_80051450(w->bAB, (u16)(w->hAC + n2));
            w->hA8 = n2;
            break;
        }
        if (D_801A2568 & 1)
            w->bar->w.panel.rank = 540;
        else if (D_801A2568 & 4)
            w->bar->w.panel.rank = 360;
        else
            w->bar->w.panel.rank = 180;
        w->bar->update = func_8007C240;
        w->bA0 = 76;
        o->update = func_80078A38;
        return;
    }

    if (*pad & 0x40) {
        pad[-6] |= 0x40;
        if (w->hA6 != D_80031A58[w->h92]) {
        if (w->h92 == 4 || w->h92 == 5) {
            if (D_80031A58[3] == 255) {
                func_8005DED8(o, func_80078D84, D_8004C060, 12);
                o->update = ui_obj_nop2;
                return;
            }
            if (D_80031A58[3] >= 8) {
                func_8005DED8(o, func_80078D84, D_8004C084, 12);
                o->update = ui_obj_nop2;
                return;
            }
        }
        if (D_80031A58[11] != 255 && D_80031A58[11] >= 27) {
            if (w->h92 == 1) {
                func_8005DED8(o, func_80078D84, D_8004C0A8, 12);
                o->update = ui_obj_nop2;
                return;
            }
            if ((w->h92 == 6 || w->h92 == 7) &&
                (w->hA6 == 24 || w->hA6 == 25 || w->hA6 == 26 || w->hA6 == 28 || w->hA6 == 30)) {
                func_8005DED8(o, func_80078D84, D_8004C0A8, 12);
                o->update = ui_obj_nop2;
                return;
            }
        }
        if (D_8017EF80[6] != &g_ui_none) {
            o->update = ui_obj_nop2;
            if (w->h92 == 5)
                func_8005DED8(o, func_80078D84, D_8004C0CC, 12);
            else if (D_80031A58[3] == 5 && (w->h92 == 6 || w->h92 == 7))
                func_8005DED8(o, func_80078D84, D_8004C0EC, 12);
            return;
        }
        if (w->b84)
            obj_find_by_handle(func_8007437C)->kill = 1;
        func_8007437C(&w->b84, &w->b85);
        w->b84 = 1;
        w->b85 = 20;
        if (w->hA6 == 255) {
            switch (w->h92) {
            case 1:
            case 2:
            case 3:
            case 8:
            case 9:
            case 11:
                func_800788C8(w->h94, 17);
                break;
            }
        }
        if (w->h92 == 3 && w->hA6 < 8 && (D_80031A58[3] == 255 || D_80031A58[3] >= 8)) {
            D_80031A58[4] = w->b87;
            switch (D_80031A58[11]) {
            case 5:
            case 21:
            case 22:
            case 23:
            case 26:
            case 27:
            case 28:
            case 30:
            case 31:
                if (w->b86 != 15)
                    D_80031A58[5] = w->b86;
                break;
            default:
                D_80031A58[5] = w->b86;
                break;
            }
        }
        if (w->h92 == 11 && (w->hA6 < 27 || w->hA6 == 255)) {
            if (D_80031A58[11] >= 27 && D_80031A58[11] != 255)
                D_80031A58[1] = func_8007AA04(1);
        }
        if (w->hA6 == 255) {
            D_80031A58[w->h92] = 255;
        } else {
            D_80031A58[w->h92] = w->hA6;
            func_800788C8(w->h94, w->bA1);
        }
        if (w->h92 == 0) {
            mask = w->hB0;
            r = D_800B9482[w->hA6].b[0];
            D_80039CA0 = mask;
            for (i = 0; i < 11; i++) {
                if (mask & 1) {
                    t = D_800BB144[i].b[0];
                    if (r < t) {
                        D_80039CA0 &= ~(-1 << i);
                        break;
                    }
                    r -= t;
                }
                mask >>= 1;
            }
        }
        if (w->hA6 == 255) {
            if (w->h92 == 3 && (D_80031A58[4] != 255 || D_80031A58[5] != D_80031A58[4])) {
                D_80031A58[4] = 255;
                D_80031A58[5] = 255;
                func_8005DED8(o, func_80078D84, D_8004C100, 12);
                o->update = ui_obj_nop2;
                return;
            }
        }
        if (w->h92 == 3 && w->hA6 >= 8 && w->hA6 != 255 &&
            (D_80031A58[4] != 255 || D_80031A58[5] != D_80031A58[4])) {
            D_80031A58[4] = 255;
            D_80031A58[5] = 255;
            func_8005DED8(o, func_80078D84, D_8004C100, 12);
            o->update = ui_obj_nop2;
            return;
        }
        if (w->h90 == 6) {
            if (w->hA6 >= 31 && w->hA6 != 255) {
                q = 6;
                if (w->h92 == 6)
                    q = 7;
                if (D_80031A58[q] != 255) {
                    D_80031A58[q] = 255;
                    if (q == 6)
                        func_8005DED8(o, func_80078D84, D_8004C11C, 12);
                    else
                        func_8005DED8(o, func_80078D84, D_8004C140, 12);
                    o->update = ui_obj_nop2;
                    return;
                }
            }
        }
        if (w->h90 == 6 && w->hA6 != 255) {
            j = 6;
            if (w->h92 == 6)
                j = 7;
            if (func_80083C00(6, w->hA6) == 1 && D_80031A58[j] == w->hA6) {
                D_80031A58[j] = 255;
                func_8005DED8(o, func_80078D84, D_8004C164, 12);
                o->update = ui_obj_nop2;
                return;
            }
        }
        if (w->h92 == 11 && w->hA6 != 255 && w->hA6 >= 27) {
            if (D_80031A58[1] != 255) {
                D_80031A58[1] = 255;
                m = func_8007AA04(1);
                if (m != 255) {
                    func_80050AF4(14, m + 64);
                    func_80051450(11, m);
                }
                func_8005DED8(o, func_8007BA10, D_8004C180, 12);
                o->update = ui_obj_nop2;
                return;
            }
            if ((D_80031A58[6] != 255 && D_80031A58[6] - 24 < 7U) ||
                (D_80031A58[7] != 255 && D_80031A58[7] - 24 < 7U)) {
                if (D_80031A58[6] >= 24)
                    D_80031A58[6] = 255;
                if (D_80031A58[7] >= 24)
                    D_80031A58[7] = 255;
                func_8005DED8(o, func_8007BB48, D_8004C19C, 12);
                o->update = ui_obj_nop2;
                return;
            }
            if (D_80031A58[5] == 15) {
                D_80031A58[5] = 255;
                func_8005DED8(o, func_80078D84, D_8004C1B4, 12);
                o->update = ui_obj_nop2;
                return;
            }
        } else {
            if (D_80031A58[5] == 15 && w->h92 == 11) {
            switch (w->hA6) {
            case 5:
            case 21:
            case 22:
            case 23:
            case 26:
            case 27:
            case 28:
            case 30:
            case 31:
                D_80031A58[5] = 255;
                func_8005DED8(o, func_80078D84, D_8004C1B4, 12);
                o->update = ui_obj_nop2;
                break;
            }
            }
        }
        fb = 0;
        fa = 0;
        if (w->h92 == 3 && w->hA6 == 5) {
            if (D_80031A58[6] != 255 && (D_80031A58[6] < 24 || D_80031A58[6] == 32))
                fa = 1;
            if (D_80031A58[7] != 255 && (D_80031A58[7] < 24 || D_80031A58[7] == 32))
                fb = 1;
            if (fa && D_80031A58[6] == 32)
                fb = 1;
            else if (fb && D_80031A58[7] == 32)
                fa = 1;
            if (fb) {
                if (fa) {
                    D_80031A58[6] = 255;
                    D_80031A58[7] = 255;
                    func_8005DED8(o, func_80078D84, D_8004C1D8, 12);
                    o->update = ui_obj_nop2;
                    return;
                } else {
                    D_80031A58[7] = 255;
                    func_8005DED8(o, func_80078D84, D_8004C140, 12);
                    o->update = ui_obj_nop2;
                    return;
                }
            }
            if (fa) {
                D_80031A58[6] = 255;
                func_8005DED8(o, func_80078D84, D_8004C11C, 12);
                o->update = ui_obj_nop2;
                return;
            }
        }
        if (w->h92 == 5)
            w->b86 = D_80031A58[5];
        else if (w->h92 == 4)
            w->b87 = D_80031A58[4];
        if (w->hA6 == 255)
            SsUtKeyOn(D_801A2318, 0, 11, 60, 0, 127, 127);
        else
            SsUtKeyOn(D_801A2318, 0, 10, 60, 0, 127, 127);
        return;
        } else {
            if (w->hA6 == 255)
                func_8005DED8(o, func_80078D84, D_8004C1F8, 12);
            else
                func_8005DED8(o, func_80078D84, D_8004C20C, 12);
            o->update = ui_obj_nop2;
            return;
        }
    }

    if (*pad & 0x20) {
        D_801A2550 |= 0x20;
        SsUtKeyOn(D_801A2318, 0, 0, 60, 0, 127, 127);
        c = D_80031A58[w->h92];
        if (c != 255 && w->hA6 != c)
            func_80051450(w->bAB, (u16)(w->hAC + c));
        if (c == 255 || D_80031A58[w->h92] == 255) {
            if (w->hA6 != 255) {
                switch (w->h92) {
                case 0:
                case 1:
                case 2:
                case 3:
                case 8:
                case 9:
                case 11:
                    func_80051450(w->bAB, (u16)(w->hAC + func_8007AA04(w->h92)));
                    break;
                }
            }
        }
        if (w->bAE) {
            func_800506EC(10);
            obj_find_by_id(9)->kill = 1;
            w->bAE = 0;
        }
        func_800506EC(4);
        func_800506EC(8);
        obj_find_by_id(3)->kill = 1;
        obj_find_by_id(7)->kill = 1;
        func_8005C2D0(32, 40, 128, 168, 3);
        func_8005C2D0(176, 162, 128, 62, 7);
        o->update = func_8007AC74;
    }
}
