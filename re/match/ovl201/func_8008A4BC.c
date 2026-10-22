#include "ac1/ac1.h"
/* ovl201 0x8008A4BC - mission briefing confirm (update of the briefing
 * window): O on "accept" (cursor obj pos.vx 4) with no pending mission takes
 * the highlighted one: marks it seen, pays its fee, starts it; with a pending
 * mission: commits it (random rewards, counters, rank, fixed part costs into
 * D_80048608) and opens the result windows. X backs out to the list. */

typedef struct {
    u8 pad0[0x50];
    UiObj *win[3];      /* +0x50 */
} GarageList;

typedef struct {
    s8 sel;             /* +0x000 */
    s8 pending;         /* +0x001 */
    s8 b2;              /* +0x002 */
    u8 pad3;
    s8 b4;              /* +0x004 */
    u8 b5;              /* +0x005 */
    u16 h6;             /* +0x006 */
    u16 h8;             /* +0x008 */
    s16 add[15];        /* +0x00A */
    u8 pad28[4];
    u32 counter;        /* +0x02C */
    u8 pad30[8];
    u8 flags[64];       /* +0x038 */
    u8 pad78;
    s8 cursor;          /* +0x079 */
    u8 pad7A[0x84 - 0x7A];
    u8 order[64];       /* +0x084 */
    u8 padC4[0x9C4 - 0xC4];
    s32 fee[64];        /* +0x9C4 */
    s32 reward[64];     /* +0xAC4 */
    u8 tblBC4[64];      /* +0xBC4 */
    u8 tblC04[64];      /* +0xC04 */
    u8 padC44[0x1444 - 0xC44];
    u8 kind[64];        /* +0x1444 */
    u16 h6tbl[64];      /* +0x1484 */
    u16 h8tbl[64];      /* +0x1504 */
    u8 chance[15][64];  /* +0x1584 */
    u8 pad[0x1944 - 0x1584 - 15 * 64];
    GarageList *list;   /* +0x1944 */
} Garage;

extern Garage D_801F3750;
extern u32 D_801A2568;
extern u32 D_801A256C;
extern s16 D_801A2318;
extern s32 D_80039CA4;
extern u8 D_800411F8_a[]; /* array: may alias, keeps retail store order */
#define D_800411F8 D_800411F8_a[0]
extern u8 D_80039C5D;
extern s32 D_8004860C;
extern s32 D_80048608;
extern u8 D_80031A58;
extern u8 D_80031A59;
extern u8 D_80031A5A;
extern u8 D_80031A5B;
extern u8 D_80031A5C;
extern u8 D_80031A5D;
extern u8 D_80031A5E;
extern u8 D_80031A5F;
extern u8 D_80031A60;
extern u8 D_80031A61;
extern u8 D_80031A63;
extern void func_8008B610(UiObj *);
extern void func_8008B76C(UiObj *);
extern void func_8008AEC4(UiObj *);
extern void func_8008B8D0(UiObj *);
extern void func_8008BA70(UiObj *);
extern void func_8008BC10(UiObj *);
extern void func_8005CC48(UiObj *);
extern void func_8008729C(UiObj *);
extern void func_80088664(UiObj *);
extern void func_800885DC(UiObj *);
extern void func_80087AEC(UiObj *);
extern void func_80088DE4(UiObj *);
extern void func_80089250(UiObj *);
extern void func_800506EC(s32);
extern void func_8005C2D0(s32, s32, s32, s32, s32);
extern void func_8008A338(void);
extern void func_800866FC(void);
extern void func_80096230(void);
extern s32 func_8008F08C(s32 kind, s32 part);

void func_8008A4BC(UiObj *o)
{
    UiObj *c;
    s32 v;
    s32 i;
    s32 s;
    s32 sum;
    s32 t;
    u8 m;

    c = obj_find_by_handle(func_8008B610);
    if ((D_801A2568 & 0x40) && !(D_801A256C & 0x40)) {
        SsUtKeyOn(D_801A2318, 0, 1, 60, 0, 127, 127);
        if (c->pos.vx == 4) {
            if (D_801F3750.pending < 0) {
                m = D_801F3750.pending = D_801F3750.order[D_801F3750.cursor];
                if (!(D_801F3750.flags[D_801F3750.pending] & 1)) {
                    if ((u8)(m - 48) < 2) {
                        D_801F3750.flags[48] |= 1;
                        D_801F3750.flags[49] |= 1;
                    } else {
                        D_801F3750.flags[D_801F3750.pending] |= 1;
                    }
                    D_80039CA4 += D_801F3750.fee[D_801F3750.pending];
                    if (D_80039CA4 > 99999999) {
                        D_80039CA4 = 99999999;
                    }
                }
                c->update = func_8008B76C;
                func_800506EC(14);
                obj_find_by_id(13)->kill = 1;
                func_8005C2D0(96, 100, 144, 48, 13);
                o->update = func_8008AEC4;
                func_8008A338();
                return;
            }
            D_801F3750.sel = D_801F3750.pending;
            D_801F3750.pending = -1;
            D_801F3750.counter++;
            if (D_801F3750.counter >= 1000) {
                D_801F3750.counter = 999;
            }
            D_801F3750.h6 = D_801F3750.h6tbl[D_801F3750.sel];
            D_801F3750.h8 = D_801F3750.h8tbl[D_801F3750.sel];
            for (i = 14; i != -1; i--) {
                s = D_801F3750.h6 * ((0x4000 - rand()) / 4 + 0x8000);
                t = s / 0x8000;
                v = D_801F3750.chance[i][D_801F3750.b2] * 0x7FFF / 256;
                if (rand() < v) {
                    D_801F3750.add[i] = t;
                } else {
                    D_801F3750.add[i] = t / 10;
                }
            }
            D_801F3750.b5 = D_801F3750.kind[D_801F3750.sel];
            if (D_801F3750.counter > 99998) {
                D_801F3750.counter = 99999;
            }
            if (D_801F3750.b4 == 1 && (u8)(D_801F3750.sel - 21) >= 2 && (u8)(D_801F3750.sel - 47) >= 2 && D_801F3750.sel != 49) {
                D_801F3750.b2++;
            }
            if (D_801F3750.b2 >= 64) {
                D_801F3750.b2 &= 0x3F;
            }
            D_800411F8 = D_801F3750.tblBC4[D_801F3750.sel];
            D_80039C5D = D_801F3750.tblC04[D_801F3750.sel];
            D_8004860C = D_801F3750.reward[D_801F3750.sel];
            sum = func_8008F08C(0, D_80031A5A);
            sum += func_8008F08C(1, D_80031A58);
            sum += func_8008F08C(2, D_80031A5B);
            sum += func_8008F08C(3, D_80031A63);
            sum += func_8008F08C(4, D_80031A60);
            sum += func_8008F08C(5, D_80031A61);
            if (D_80031A59 != 0xFF) {
                sum += func_8008F08C(7, D_80031A59);
            }
            if (D_80031A5E != 0xFF) {
                sum += func_8008F08C(8, D_80031A5E);
            }
            if (D_80031A5F != 0xFF) {
                sum += func_8008F08C(8, D_80031A5F);
            }
            if (D_80031A5C != 0xFF) {
                sum += func_8008F08C(9, (u8)(D_80031A5C + 16));
            }
            if (D_80031A5D != 0xFF) {
                sum += func_8008F08C(9, D_80031A5D);
            }
            D_80048608 = sum;
            c->update = func_8008B76C;
            func_800506EC(14);
            obj_find_by_id(13)->kill = 1;
            func_800506EC(6);
            obj_find_by_id(5)->kill = 1;
            func_8005C2D0(96, 100, 144, 48, 13);
            func_8005C2D0(8, 128, 304, 96, 5);
            if (D_801F3750.list->win[0] != 0) {
                func_800506EC(8);
                obj_find_by_id(7)->kill = 1;
                func_8005C2D0(4, 22, 104, 100, 7);
                D_801F3750.list->win[0]->update = func_8008B8D0;
            }
            if (D_801F3750.list->win[1] != 0) {
                func_800506EC(10);
                obj_find_by_id(9)->kill = 1;
                func_8005C2D0(108, 22, 104, 100, 9);
                D_801F3750.list->win[1]->update = func_8008BA70;
            }
            if (D_801F3750.list->win[2] != 0) {
                func_800506EC(12);
                obj_find_by_id(11)->kill = 1;
                func_8005C2D0(212, 22, 104, 100, 11);
                D_801F3750.list->win[2]->update = func_8008BC10;
            }
            o->update = func_8008AEC4;
            obj_find_by_handle(func_8005CC48)->kill = 1;
            obj_find_by_handle(func_8008729C)->update = func_80088664;
            func_800866FC();
            func_80096230();
            return;
        }
        c->update = func_8008B76C;
        func_800506EC(14);
        obj_find_by_id(13)->kill = 1;
        func_800506EC(6);
        obj_find_by_id(5)->kill = 1;
        func_8005C2D0(96, 100, 144, 48, 13);
        func_8005C2D0(8, 128, 304, 96, 5);
        if (D_801F3750.list->win[0] != 0) {
            func_800506EC(8);
            obj_find_by_id(7)->kill = 1;
            func_8005C2D0(4, 22, 104, 100, 7);
            D_801F3750.list->win[0]->update = func_8008B8D0;
        }
        if (D_801F3750.list->win[1] != 0) {
            func_800506EC(10);
            obj_find_by_id(9)->kill = 1;
            func_8005C2D0(108, 22, 104, 100, 9);
            D_801F3750.list->win[1]->update = func_8008BA70;
        }
        if (D_801F3750.list->win[2] != 0) {
            func_800506EC(12);
            obj_find_by_id(11)->kill = 1;
            func_8005C2D0(212, 22, 104, 100, 11);
            D_801F3750.list->win[2]->update = func_8008BC10;
        }
        o->update = func_8008AEC4;
        if (D_801F3750.pending >= 0) {
            obj_find_by_handle(func_8005CC48)->kill = 1;
            obj_find_by_handle(func_8008729C)->update = func_800885DC;
        } else {
            obj_find_by_handle(func_8008729C)->update = func_80087AEC;
        }
        return;
    }
    if ((D_801A2568 & 0x20) && !(D_801A256C & 0x20)) {
        SsUtKeyOn(D_801A2318, 0, 1, 60, 0, 127, 127);
        c->update = func_8008B76C;
        func_800506EC(14);
        obj_find_by_id(13)->kill = 1;
        func_8005C2D0(96, 100, 144, 48, 13);
        obj_find_by_handle(func_80088DE4)->update = func_80089250;
        o->update = func_8008AEC4;
    }
}
