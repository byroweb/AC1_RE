#include "ac1/ac1.h"
/* matched (wave4 a5): weapon list k/count split, cur+1 before n-1, index pre-scaled onto nl->w base, pu set before the compass call */
/* ovl202 0x80050468 - mission HUD, revealed step by step by g_mission_end_seq:
 * target gauge (>= 6), AP readout with low-AP warning (>= 11), mission timer
 * (>= 16), blinking alerts, area-limit countdown, radar with range box, entity
 * blips and area boxes (>= 21), weapon / ammo list (>= 26) and compass tape (>= 31). */

typedef struct {                /* 0x3C weapon entry */
    u8 pad00[0x0E];
    u16 max;                    /* +0x0E */
    u16 ammo;                   /* +0x10 */
    u8 pad12[0x2A - 0x12];
    u8 name[0x12];              /* +0x2A */
} HudWpn;

typedef struct {
    u8 n;
    u8 pad;
    HudWpn w[1];
} HudWpnList;

typedef struct {                /* 0x80040FB0 */
    u16 flags;                  /* +0x000 bit0 area boxes, bits0-1 == 1 area countdown */
    u8 pad002[7];
    u8 cur;                     /* +0x009 selected weapon */
    u8 pad00a[4];
    u16 gauge;                  /* +0x00E */
    u16 flags2;                 /* +0x010 bit0 alert */
    u8 pad012[0x216 - 0x12];
    SVECTOR box1[2];            /* +0x216 (0x800411C6) */
    SVECTOR box2[2];            /* +0x226 (0x800411D6) */
    u16 blink;                  /* +0x236 */
    s16 last;                   /* +0x238 */
    s16 cnt;                    /* +0x23A */
    s16 cntmin;                 /* +0x23C */
    u16 alert1;                 /* +0x23E */
    u16 alert2;                 /* +0x240 */
    u8 pad242[0x254 - 0x242];
    u16 gauge_max;              /* +0x254 */
    u8 pad256[4];
    u16 gauge2;                 /* +0x25A */
    u8 pad25c[4];
    u16 range;                  /* +0x260 radar range */
    u8 pad262[5];
    u8 rflags;                  /* +0x267 */
    u8 pad268;
    u8 icon;                    /* +0x269 */
    u8 pad26a[0x2C8 - 0x26A];
    HudWpnList wl;              /* +0x2C8 */
} HudState;

typedef struct {
    u8 c[12];
} HudSpr;

extern HudState D_80040FB0;
extern u8 D_8004ADA8[];
extern u8 D_8004ADB0[];
extern HudSpr D_8008D938[];
extern HudSpr D_8008D968;
extern HudSpr D_8008D974;
extern DVECTOR D_8008D980;
extern u32 D_8008D984;
extern HudSpr D_8008D988;
extern DVECTOR D_8008D994;
extern HudSpr D_8008D998;
extern DVECTOR D_8008D9A4;
extern HudSpr D_8008D9A8;
extern DVECTOR D_8008D9B4;
extern HudSpr D_8008D9B8;
extern DVECTOR D_8008D9C4;
extern HudSpr D_8008D9C8;
extern DVECTOR D_8008D9D4;
extern DVECTOR D_8008D9D8;
extern u32 D_8008D9DC;
extern u32 D_8008D9E0;
extern u32 D_8008D9E4;
extern u32 D_8008D9E8;
extern u32 D_8008D9EC;
extern u32 D_8008D9F0;
extern u32 D_8008D9F4;
extern u32 D_8008D9F8;
extern u32 D_8008D9FC;
extern u32 D_8008DA00;
extern SprInfo D_8008DA04;
extern DVECTOR D_8008DA10;
extern DVECTOR D_8008DA14;
extern u32 D_8008DA1C[];
extern u32 D_8008DA20;
extern u32 D_8008DA44;
extern s16 D_800908D0;
extern s32 D_801AC81C;
extern void func_80060B54(DVECTOR *xy, void *spr, s32 rot, u32 *rgb, s32 z, s32 abr);
extern s32 func_80051150(s32 x, s32 y, s32 z, s32 font, s32 color, u8 *s);
extern void func_8005E7D8(s32 x0, s32 y0, s32 x1, s32 y1, u32 *rgb, s32 z, s32 abr);
extern void func_8004CFC8(s32 a, s32 b);
extern void func_80012670(u8 *buf, u32 v, s32 n);
extern s32 func_800125D0(u8 *buf, u16 v);
extern s32 rsin(s32 a);
extern s32 rcos(s32 a);
extern void func_800610A4(DVECTOR *pts, u32 *rgb, s32 n, s32 z, s32 abr);
extern void func_800501DC(AcPos *pos, u32 *a, u32 *b, u32 *c, s32 sn, s32 cs);
extern void func_8004FD9C(s32 sn, s32 cs);
extern void func_80050088(SVECTOR *box, u32 *rgb, s32 sn, s32 cs);
extern s16 func_800132CC(s16 a, s16 b, s32 c);

void func_80050468(void)
{
    u8 ap[16];
    u8 tm[16];
    u8 num[16];
    DVECTOR pts[5];
    s32 warn;
    s32 g1;
    s32 g2;
    s32 e;
    s32 t;
    s32 m;
    u8 c;
    s32 d;
    s32 k;
    s32 sn;
    s32 cs;
    s32 r;
    s32 i;
    s32 n;
    s32 y;
    s32 col;
    AcEntity *ent;
    HudWpn *w;

    warn = 0;
    if (g_mission_end_seq >= 6) {
        func_80060B54(&D_8008D980, &D_8008D974, 0, D_8008DA1C, 10, 0);
        func_80051150(8, 38, 10, 56, 0, D_8004ADA8);
        g1 = 157 - D_80040FB0.gauge * 157 / D_80040FB0.gauge_max;
        g2 = 157 - D_80040FB0.gauge2 * 157 / D_80040FB0.gauge_max;
        if (g1 != 0) {
            func_8005E7D8(8, 47, 15, g1 + 47, &D_8008D984, 10, 2);
        }
        if (g1 != 157) {
            if (D_80040FB0.flags2 & 1) {
                func_8005E7D8(8, g1 + 47, 15, 204, &D_8008DA1C[10], 10, -1);
            } else {
                if (g1 < g2) {
                    e = g2 + 47;
                    func_8005E7D8(8, g1 + 47, 15, e, &D_8008DA1C[9], 10, -1);
                    func_8005E7D8(8, e, 15, 204, &D_8008DA1C[10], 10, -1);
                    D_80040FB0.blink = 0;
                } else {
                    if (D_80040FB0.last < g1) {
                        D_80040FB0.blink -= g1 - g2;
                        if ((s16)D_80040FB0.blink <= 0) {
                            func_8004CFC8(41, 100);
                            D_80040FB0.blink = (157 - g2) * 2;
                        }
                    }
                    func_8005E7D8(8, g1 + 47, 15, 204, &D_8008DA44, 10, -1);
                }
                D_80040FB0.last = g1;
            }
        }
    }
    if (g_mission_end_seq >= 11) {
        if (g_ac_entities[0].ap * 10 <= g_ac_entities[0].ap_max) {
            warn = 1;
        }
        func_80012670(ap, (u32)(g_ac_entities[0].ap + 3) >> 2, 4);
        func_80051150(32, 22, 10, 220, warn, ap);
    }
    if (g_mission_end_seq >= 16 && g_mission_timer != 0) {
        func_80051150(238, 16, 10, 56, 0, D_8004ADB0);
        t = g_mission_timer * 100 / 22;
        m = t / 6000;
        func_80012670(tm, m, 2);
        func_80012670(tm + 3, t - m * 6000, 4);
        c = tm[5];
        tm[8] = 0;
        tm[7] = tm[6];
        tm[6] = c;
        tm[5] = 0;
        tm[2] = 'A';
        func_80051150(224, 24, 10, 220, 0, tm);
        func_80051150(296, 40, 10, 128, 0, tm + 6);
    }
    {
        u16 *a = &D_80040FB0.alert1;
        s32 v = *a << 16;
        if (v != 0) {
            if ((v >> 18) & 3) {
                func_80060B54(&D_8008D994, &D_8008D988, 0, D_8008DA1C, 10, 2);
            }
            (*a)--;
        }
    }
    {
        u16 *a = &D_80040FB0.alert2;
        s32 v = *a << 16;
        if (v != 0) {
            if ((v >> 18) & 3) {
                func_80060B54(&D_8008D9A4, &D_8008D998, 0, &D_8008DA20, 10, 2);
            }
            (*a)--;
        }
    }
    {
        HudState *hs = &D_80040FB0;
        if ((hs->flags & 3) == 1) {
            s16 *cp = &hs->cnt;
            if (hs->cnt >= D_80040FB0.cntmin) {
                func_80060B54(&D_8008D9B4, &D_8008D9A8, 0, &D_8008DA20, 10, 2);
            }
            if (--hs->cnt == -1) {
                s32 dd;
                s32 kk;
                func_8004CFC8(8, 90);
                dd = D_80040FB0.box2[0].vx - g_ac_entities[0].pos.x;
                if (dd < 0) {
                    dd = g_ac_entities[0].pos.x - D_80040FB0.box2[0].vx;
                }
                kk = D_80040FB0.box2[0].vz - g_ac_entities[0].pos.z;
                if (kk < 0) {
                    kk = g_ac_entities[0].pos.z - D_80040FB0.box2[0].vz;
                }
                if (kk < dd) {
                    dd = kk;
                }
                kk = D_80040FB0.box2[1].vx - g_ac_entities[0].pos.x;
                if (kk < 0) {
                    kk = g_ac_entities[0].pos.x - D_80040FB0.box2[1].vx;
                }
                if (kk < dd) {
                    dd = kk;
                }
                kk = D_80040FB0.box2[1].vz - g_ac_entities[0].pos.z;
                if (kk < 0) {
                    kk = g_ac_entities[0].pos.z - D_80040FB0.box2[1].vz;
                }
                if (kk < dd) {
                    dd = kk;
                }
                dd >>= 8;
                if (dd < 3) {
                    dd = 3;
                }
                cp[0] = dd;
                cp[1] = (dd >> 1) - (dd >> 3);
            }
        } else {
            D_80040FB0.cntmin = 0;
            D_80040FB0.cnt = 0;
        }
    }
    if (warn) {
        k = D_801AC81C & 7;
        if (k == 0) {
            func_8004CFC8(9, 90);
        }
        if (k < 5) {
            func_80060B54(&D_8008D9C4, &D_8008D9B8, 0, &D_8008DA20, 10, 2);
        }
    }
    if (D_80040FB0.flags2 & 1) {
        k = (D_801AC81C + 3) & 7;
        if (k < 5) {
            func_80060B54(&D_8008D9D4, &D_8008D9C8, 0, &D_8008DA20, 10, 2);
            if (k == 0) {
                func_8004CFC8(40, 90);
            }
        }
    }
    if (g_mission_end_seq >= 21) {
        sn = rsin(g_ac_entities->yaw);
        cs = rcos(g_ac_entities->yaw);
        {
            u16 *rng = &D_80040FB0.range;
            if (*rng != 0) {
                r = 80000 / *rng;
                pts[0].vx = 282 - r;
                pts[0].vy = 79 - r;
                pts[1].vx = r + 283;
                pts[1].vy = 79 - r;
                pts[2].vx = r + 283;
                pts[2].vy = r + 79;
                pts[3].vx = 282 - r;
                pts[3].vy = r + 79;
                pts[4] = pts[0];
                func_800610A4(pts, &D_8008D9EC, 4, 3, 0);
                func_80060B54(&D_8008D9D8, &D_8008D938[D_80040FB0.icon], 0, &D_8008D9DC, 15, 2);
                ent = &g_ac_entities[1];
                for (i = 15; i != 0; i--) {
                    if (ent->def != 0 && !(ent->ignore_mask & 0x12)) {
                        func_800501DC(&ent->pos, &D_8008D9E0, &D_8008D9E4, &D_8008D9E8, sn, cs);
                    }
                    ent++;
                }
                {
                    u8 *rf = &D_80040FB0.rflags;
                    if (*rf & 1) {
                        func_8004FD9C(sn, cs);
                    }
                    if (D_80040FB0.flags & 1) {
                        func_80050088(D_80040FB0.box1, &D_8008D9F0, sn, cs);
                        func_80050088(D_80040FB0.box2, &D_8008D9F4, sn, cs);
                    }
                }
            } else {
                func_80060B54(&D_8008D9D8, &D_8008D968, 0, &D_8008D9DC, 15, 2);
                if (D_80040FB0.flags & 1) {
                    *rng = 5000;
                    func_80050088(D_80040FB0.box1, &D_8008D9F8, sn, cs);
                    func_80050088(D_80040FB0.box2, &D_8008D9FC, sn, cs);
                    *rng = 0;
                }
            }
        }
    }
    if (g_mission_end_seq >= 26) {
        HudWpnList *nl = &D_80040FB0.wl;
        if (nl->n != 0) {
            { s32 c = D_80040FB0.cur * sizeof(HudWpn); HudWpn *b = nl->w; w = (HudWpn *)((u8 *)b + c); }
            col = 8;
            if (w->ammo == 0) {
                col = 1;
            }
            func_80051150(214, 112, 10, 128, col, w->name);
            n = func_800125D0(num, w->ammo);
            y = 130;
            num[n] = '/';
            num[n + 1] = 0;
            func_80051150((4 - n) * 6 + 256, 120, 10, 128, col, num);
            n = func_800125D0(num, w->max);
            w++;
            func_80051150((4 - n) * 6 + 286, 120, 10, 128, col, num);
            g1 = D_80040FB0.cur + 1;
            g2 = nl->n;
            g2--;
            while (g2 > 0) {
                if (g1 >= nl->n) {
                    w = nl->w;
                    g1 = 0;
                }
                col = w->ammo == 0;
                func_80051150(226, y, 10, 56, col, w->name);
                n = func_800125D0(num, w->ammo);
                w++;
                func_80051150(310 - n * 4, y, 10, 56, col, num);
                y += 8;
                g1++;
                g2--;
            }
        }
    }
    if (g_mission_end_seq >= 31) {
        func_8005E7D8(96, 16, 224, 30, &D_8008DA00, 10, 2);
        {
            u8 *pu = &D_8008DA04.u;
            D_800908D0 = func_800132CC(D_800908D0, g_ac_entities[0].yaw, 32);
            d = ((1024 - D_800908D0) & 0xFFF) >> 4;
            *pu = d;
            if (d < 129) {
                D_8008DA04.w = 127;
                func_80060B54(&D_8008DA10, &D_8008DA04, 0, D_8008DA1C, 10, 0);
            } else {
                D_8008DA04.w = ~d;
                func_80060B54(&D_8008DA10, &D_8008DA04, 0, D_8008DA1C, 10, 0);
                *pu = 0;
                c = D_8008DA04.w;
                D_8008DA14.vx = c + 96;
                D_8008DA04.w = d & 0x7F;
                func_80060B54(&D_8008DA14, &D_8008DA04, 0, D_8008DA1C, 10, 0);
            }
        }
    } else {
        D_800908D0 = g_ac_entities[0].yaw;
    }
}
