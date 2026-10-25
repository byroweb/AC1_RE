/* ovl203 0x80050D78 - 2-player HUD: energy bar and damage warning, heat, timer,
 * radar frame + blips, weapon list, compass strip.  Hand rewrite of the m2c draft. */
#include "ac1/ac1.h"

typedef struct Pt16 {
    s16 x, y;
} Pt16;

typedef struct Body {
    u8 unk000[8];
    s16 x;                      /* +0x08 */
    u8 unk00A[2];
    s16 z;                      /* +0x0C */
    u8 unk00E[4];
    s16 angle;                  /* +0x12 */
    u8 unk014[0x160 - 0x14];
    u16 heat;                   /* +0x160 */
    u16 heat2;                  /* +0x162 */
} Body;

typedef struct Weap {
    u8 unk00[0xE];
    u16 max;                    /* +0x0E */
    u16 ammo;                   /* +0x10 */
    u8 unk12[0x2A - 0x12];
    u8 name[0x3C - 0x2A];       /* +0x2A */
} Weap;

typedef struct Pilot {
    u8 unk000[0x10];
    u16 mode;                   /* +0x10 */
    u8 unk012[0x19 - 0x12];
    u8 cur;                     /* +0x19 */
    u8 unk01A[4];
    u16 hp;                     /* +0x1E */
    u16 flags;                  /* +0x20 */
    u8 unk022[0x226 - 0x22];
    SVECTOR box1[2];            /* +0x226 */
    SVECTOR box2[2];            /* +0x236 */
    s16 warn;                   /* +0x246 */
    s16 last_bar;               /* +0x248 */
    s16 beep;                   /* +0x24A */
    s16 beep_on;                /* +0x24C */
    u16 blink1;                 /* +0x24E */
    u16 blink2;                 /* +0x250 */
    u8 unk252[0x264 - 0x252];
    u16 hp_max;                 /* +0x264 */
    u8 unk266[4];
    u16 hp2;                    /* +0x26A */
    u8 unk26C[4];
    u16 range;                  /* +0x270 */
    u8 unk272[5];
    u8 radar_flags;             /* +0x277 */
    u8 unk278;
    u8 radar_mode;              /* +0x279 */
    u8 unk27A[0x2D8 - 0x27A];
    u8 nweap;                   /* +0x2D8 */
    u8 unk2D9;
    Weap weap[3];               /* +0x2DA */
    u8 unk38E[6];
    Body *body;                 /* +0x394 */
} Pilot;

extern Pilot *D_80039D18;
extern Pilot *D_80039D30;
extern Pilot *D_80039D34;
extern u8 D_8004ADA8[];
extern u8 D_8004ADB0[];
extern u8 D_8004ADC0[];
extern u8 D_800895E0[];
extern u8 D_80089610[];
extern u8 D_8008961C[];
extern Pt16 D_80089628;
extern u8 D_8008962C[];
extern u8 D_80089630[];
extern Pt16 D_8008963C;
extern u8 D_80089640[];
extern Pt16 D_8008964C;
extern u8 D_80089650[];
extern Pt16 D_8008965C;
extern u8 D_80089660[];
extern Pt16 D_8008966C;
extern u8 D_80089670[];
extern Pt16 D_8008967C;
extern u8 D_80089680[];
extern Pt16 D_8008968C;
extern Pt16 D_80089690;
extern u8 D_80089694[];
extern u8 D_80089698[];
extern u8 D_8008969C[];
extern u8 D_800896A0[];
extern u8 D_800896A4[];
extern u8 D_800896A8[];
extern u8 D_800896AC[];
extern u8 D_800896B0[];
extern u8 D_800896B4[];
extern u8 D_800896B8[];
extern u8 D_800896C0[];
extern u8 D_800896C2;
extern Pt16 D_800896C8;
extern Pt16 D_800896CC;
extern u8 D_80089744[];
extern u8 D_80089748[];
extern u8 D_8008976C[];
extern s16 D_8008C840;
extern u16 D_8019B492;
extern u16 D_8019B494;
extern s32 D_801A3DA4;

s32 func_800125D0(u8 *buf, u32 n);
void func_80012670(u8 *buf, u32 n, s32 digits);
s16 func_800132CC(s32 a, s32 b, s32 c);
void func_8004D1D0(s32 a, s32 b);
void func_800506A4(s32 s, s32 c);
void func_80050998(SVECTOR *box, u8 *color, s32 s, s32 c);
void func_80050AF0(void *p, u8 *a, u8 *b, u8 *c, s32 s, s32 co);
void func_80051E1C(s32 x, s32 y, s32 w, s32 h, s32 n, u8 *s);
void func_8005DFF8(s32 x, s32 y, s32 w, s32 h, u8 *col, s32 a, s32 b);
void func_80060374(Pt16 *p, u8 *a, s32 b, u8 *c, s32 d, s32 e);
void func_800608C4(DVECTOR *p, u8 *col, s32 n, s32 a, s32 b);

void func_80050D78(void)
{
    u8 heat_str[16];
    u8 time_str[16];
    u8 num_str[16];
    union {
        DVECTOR pt[5];
        u8 str[20];
    } u;
    s32 overheat;
    s32 bar;
    s32 bar2;
    s32 max;
    s32 n;
    s32 t;
    s32 m;
    s32 d;
    s32 e;
    s32 r;
    s32 sn;
    s32 cs;
    s32 k;
    s32 color;
    s32 y;
    s32 i;
    s32 j;
    s32 c;
    s16 hd;
    s32 len;
    u8 *str;
    Pilot *p;
    Body *b;
    Weap *w;

    overheat = 0;
    if (D_8019B492 >= 6) {
        func_80060374(&D_80089628, D_8008961C, 0, D_80089744, 10, 0);
        func_80051E1C(8, 28, 10, 56, 0, D_8004ADA8);
        max = D_80039D30->hp_max;
        bar = 157 - D_80039D30->hp * 157 / max;
        bar2 = 157 - D_80039D30->hp2 * 157 / max;
        if (bar != 0) {
            func_8005DFF8(8, 37, 15, bar + 37, D_8008962C, 10, 2);
        }
        if (bar != 157) {
            if (D_80039D18->flags & 1) {
                func_8005DFF8(8, bar + 37, 15, 194, D_80089744 + 40, 10, -1);
            } else {
                if (bar < bar2) {
                    func_8005DFF8(8, bar + 37, 15, bar2 + 37, D_80089744 + 36, 10, -1);
                    func_8005DFF8(8, bar2 + 37, 15, 194, D_80089744 + 40, 10, -1);
                    D_80039D30->warn = 0;
                } else {
                    if (D_80039D30->last_bar < bar) {
                        if ((s16)(D_80039D30->warn -= bar - bar2) <= 0) {
                            func_8004D1D0(41, 100);
                            D_80039D30->warn = (157 - bar2) * 2;
                        }
                    }
                    func_8005DFF8(8, bar + 37, 15, 194, D_8008976C, 10, -1);
                }
                D_80039D30->last_bar = bar;
            }
        }
        func_80060374(&D_8008963C, D_80089630, 0, D_80089630 + 8, 4, 2);
    }
    if (D_8019B492 >= 11) {
        if (D_80039D18->body->heat2 >= D_80039D18->body->heat * 10) {
            overheat = 1;
        }
        func_80012670(heat_str, (u32)(D_80039D18->body->heat + 3) >> 2, 4);
        func_80051E1C(32, 22, 10, 220, overheat, heat_str);
    }
    if (D_8019B492 >= 16) {
        if (D_8019B494 != 0) {
            func_80051E1C(238, 16, 10, 56, 0, D_8004ADB0);
            t = D_8019B494 * 100 / 22;
            m = t / 6000;
            func_80012670(time_str, m, 2);
            func_80012670(time_str + 3, t - m * 6000, 4);
            c = time_str[5];
            time_str[8] = 0;
            time_str[5] = 0;
            time_str[2] = 'A';
            time_str[7] = time_str[6];
            time_str[6] = c;
            func_80051E1C(224, 24, 10, 220, 0, time_str);
            func_80051E1C(296, 40, 10, 128, 0, time_str + 6);
        }
    }
    func_80051E1C(40, 50, 10, 56, 0, D_8004ADC0);
    n = func_800125D0(num_str, (u32)(D_80039D34->body->heat + 3) >> 2);
    func_80051E1C(n * 8 + 40, 56, 10, 128, 0, num_str);
    if ((s16)D_80039D30->blink1 != 0) {
        if (((s16)D_80039D30->blink1 >> 2) & 3) {
            func_80060374(&D_8008964C, D_80089640, 0, D_80089744, 10, 2);
        }
        D_80039D30->blink1--;
    }
    if ((s16)D_80039D30->blink2 != 0) {
        if (((s16)D_80039D30->blink2 >> 2) & 3) {
            func_80060374(&D_8008965C, D_80089650, 0, D_80089748, 10, 2);
        }
        D_80039D30->blink2--;
    }
    if ((D_80039D30->mode & 3) == 1) {
        if (D_80039D30->beep >= D_80039D30->beep_on) {
            func_80060374(&D_8008966C, D_80089660, 0, D_80089748, 10, 2);
        }
        if (--D_80039D30->beep == -1) {
            func_8004D1D0(8, 90);
            b = D_80039D18->body;
            p = D_80039D30;
            d = p->box2[0].vx - b->x;
            if (d < 0) {
                d = b->x - p->box2[0].vx;
            }
            e = p->box2[0].vz - b->z;
            if (e < 0) {
                e = b->z - p->box2[0].vz;
            }
            if (e < d) {
                d = e;
            }
            e = p->box2[1].vx - b->x;
            if (e < 0) {
                e = b->x - p->box2[1].vx;
            }
            if (e < d) {
                d = e;
            }
            e = p->box2[1].vz - b->z;
            if (e < 0) {
                e = b->z - p->box2[1].vz;
            }
            if (e < d) {
                d = e;
            }
            d >>= 8;
            if (d < 3) {
                d = 3;
            }
            p->beep = d;
            p->beep_on = (d >> 1) - (d >> 3);
        }
    } else {
        D_80039D30->beep_on = 0;
        D_80039D30->beep = 0;
    }
    if (overheat) {
        n = D_801A3DA4 & 7;
        if (n == 0) {
            func_8004D1D0(9, 90);
        }
        if (n < 5) {
            func_80060374(&D_8008967C, D_80089670, 0, D_80089748, 10, 2);
        }
    }
    if (D_80039D18->flags & 1) {
        n = (D_801A3DA4 + 3) & 7;
        if (n < 5) {
            func_80060374(&D_8008968C, D_80089680, 0, D_80089748, 10, 2);
            if (n == 0) {
                func_8004D1D0(40, 90);
            }
        }
    }
    if (D_8019B492 >= 21) {
        sn = rsin(D_80039D18->body->angle);
        cs = rcos(D_80039D18->body->angle);
        if (D_80039D18->range != 0) {
            k = 80000 / D_80039D18->range;
            u.pt[0].vx = 282 - k;
            u.pt[0].vy = 79 - k;
            u.pt[1].vx = k + 283;
            u.pt[1].vy = 79 - k;
            u.pt[2].vx = k + 283;
            u.pt[2].vy = k + 79;
            u.pt[3].vx = 282 - k;
            u.pt[3].vy = k + 79;
            u.pt[4] = u.pt[0];
            func_800608C4(u.pt, D_800896A4, 4, 3, 0);
            func_80060374(&D_80089690, D_800895E0 + D_80039D18->radar_mode * 12, 0, D_80089694, 15, 2);
            func_80050AF0(&D_80039D34->body->x, D_80089698, D_8008969C, D_800896A0, sn, cs);
            if (D_80039D18->radar_flags & 1) {
                func_800506A4(sn, cs);
            }
            if (D_80039D30->mode & 1) {
                func_80050998(D_80039D30->box1, D_800896A8, sn, cs);
                func_80050998(D_80039D30->box2, D_800896AC, sn, cs);
            }
        } else {
            func_80060374(&D_80089690, D_80089610, 0, D_80089694, 15, 2);
            if (D_80039D30->mode & 1) {
                D_80039D18->range = 5000;
                func_80050998(D_80039D30->box1, D_800896B0, sn, cs);
                func_80050998(D_80039D30->box2, D_800896B4, sn, cs);
                D_80039D18->range = 0;
            }
        }
    }
    if (D_8019B492 >= 26 && D_80039D18->nweap != 0) {
        w = &D_80039D18->weap[D_80039D18->cur];
        color = 8;
        if (w->ammo == 0) {
            color = 1;
        }
        func_80051E1C(214, 112, 10, 128, color, w->name);
        c = func_800125D0(u.str, w->ammo);
        y = 130;
        u.str[c] = '/';
        u.str[c + 1] = 0;
        func_80051E1C((4 - c) * 6 + 256, 120, 10, 128, color, u.str);
        func_80051E1C((4 - func_800125D0(u.str, w->max)) * 6 + 286, 120, 10, 128, color, u.str);
        w++;
        j = D_80039D18->cur + 1;
        i = D_80039D18->nweap;
        i--;
        while (i > 0) {
            if (j >= D_80039D18->nweap) {
                w = D_80039D18->weap;
                j = 0;
            }
            color = w->ammo == 0;
            func_80051E1C(226, y, 10, 56, color, w->name);
            j++;
            i--;
            len = func_800125D0(u.str, w->ammo);
            w++;
            func_80051E1C(310 - len * 4, y, 10, 56, color, u.str);
            y += 8;
        }
    }
    if (D_8019B492 >= 31) {
        func_8005DFF8(96, 16, 224, 30, D_800896B8, 10, 2);
        hd = func_800132CC(D_8008C840, D_80039D18->body->angle, 32);
        str = D_800896C0;
        D_8008C840 = hd;
        r = ((0x400 - hd) & 0xFFF) >> 4;
        str[0] = r;
        if (r < 129) {
            D_800896C2 = 127;
            func_80060374(&D_800896C8, str - 4, 0, D_80089744, 10, 0);
        } else {
            u8 old;

            D_800896C2 = ~r;
            func_80060374(&D_800896C8, str - 4, 0, D_80089744, 10, 0);
            str[0] = 0;
            old = D_800896C2;
            D_800896C2 = r & 0x7F;
            D_800896CC.x = old + 96;
            func_80060374(&D_800896CC, str - 4, 0, D_80089744, 10, 0);
        }
    } else {
        D_8008C840 = D_80039D18->body->angle;
    }
}
