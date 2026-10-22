/* ranking list input: up/down scroll, confirm opens the entry, cancel closes. */
#include "ac1/ac1.h"

typedef struct RankEnt {
    char *name;                 /* +0x00 */
    char *sub;                  /* +0x04 */
    u8 kind;                    /* +0x08 0xF = the player's row */
    u8 pad09[3];
    u32 points;                 /* +0x0C sort key */
} RankEnt;

typedef struct RankWork {
    UiObj *src;                 /* +0x7C */
    UiObj *item[5];             /* +0x80 */
    u8 pad94[4];
    u8 page;                    /* +0x98 */
    u8 row;                     /* +0x99 */
    u8 pad9a[2];
    UiObj *next;                /* +0x9C */
} RankWork;

extern RankEnt D_8017F584[16];
extern s16 D_801A2318;
typedef struct FePad {
    u32 now;                    /* +0x00 0x801A2550 */
    s16 rep_first;              /* +0x04 */
    s16 rep_next;               /* +0x06 */
    u8 pad08[0x10];
    u32 trig;                   /* +0x18 0x801A2568 */
    u32 held;                   /* +0x1C 0x801A256C */
} FePad;
extern FePad D_801A2550;
extern s32 D_801A2574;
extern void func_800506EC(s32);
extern void func_80051450(s32, s32, s32, s32);
extern void func_8005C2D0(s32, s32, s32, s32, s32);
extern void func_8005CC48();
extern void func_800B27A0(UiObj *);
extern void func_800B2EC4(UiObj *);
extern UiObj *func_800B2F18(s32);
extern void func_800B2F74(UiObj *);

void func_800B2AC4(UiObj *o)
{
    RankWork *w = (RankWork *)&o->w;
    s16 k;
    s16 msg;
    u8 kind;
    u8 page;
    u8 row;

    func_800B27A0(o);
    if (D_801A2550.trig & 0x1000) {
        D_801A2550.now |= 0x1000;
        if (D_801A2574 != 0) {
            D_801A2550.rep_first = 6 / D_801A2574;
            if (!(D_801A2550.held & 0x1000)) {
                D_801A2550.rep_next = 12 / D_801A2574;
            }
        } else {
            D_801A2550.rep_first = 6;
            if (!(D_801A2550.held & 0x1000)) {
                D_801A2550.rep_next = 12;
            }
        }
        SsUtKeyOn(D_801A2318, 0, 1, 0x3C, 0, 0x7F, 0x7F);
        if (w->page != 0) {
            w->page--;
        } else if (w->row != 0) {
            w->row--;
        }
        return;
    }
    if (D_801A2550.trig & 0x4000) {
        SsUtKeyOn(D_801A2318, 0, 1, 0x3C, 0, 0x7F, 0x7F);
        D_801A2550.now |= 0x4000;
        if (D_801A2574 != 0) {
            D_801A2550.rep_first = 6 / D_801A2574;
            if (!(D_801A2550.held & 0x4000)) {
                D_801A2550.rep_next = 12 / D_801A2574;
            }
        } else {
            D_801A2550.rep_first = 6;
            if (!(D_801A2550.held & 0x4000)) {
                D_801A2550.rep_next = 12;
            }
        }
        if (w->page != 5) {
            w->page++;
        } else if (w->row != 4) {
            w->row++;
        }
    }
    if (D_801A2550.trig & 0x40) {
        D_801A2550.now |= 0x40;
        SsUtKeyOn(D_801A2318, 0, 2, 0x3C, 0, 0x7F, 0x7F);
        func_800506EC(4);
        obj_find_by_id(3)->kill = 1;
        func_8005C2D0(0x1C, 0x50, 0x100, 0x90, 3);
        ((UiObj *)o->w.list.src)->kill = 1;
        o->w.ring.item[0]->kill = 1;
        o->w.ring.item[1]->kill = 1;
        o->w.ring.item[2]->kill = 1;
        o->w.ring.item[3]->kill = 1;
        row = w->row;
        page = w->page;
        kind = D_8017F584[row + page].kind;
        msg = kind + 0xB;
        if (kind != 0xF) {
            k = kind + 0x1B;
        } else {
            k = row + page + 0x2A;
        }
        func_80051450(4, msg, page, row);
        w->next = func_800B2F18(k);
        o->update = func_800B2F74;
    } else if (D_801A2550.trig & 0x20) {
        D_801A2550.now |= 0x20;
        SsUtKeyOn(D_801A2318, 0, 0, 0x3C, 0, 0x7F, 0x7F);
        func_800506EC(4);
        obj_find_by_id(3)->kill = 1;
        func_8005C2D0(0x1C, 0x50, 0x100, 0x90, 3);
        obj_find_by_handle(func_8005CC48)->kill = 1;
        ((UiObj *)o->w.list.src)->kill = 1;
        o->w.ring.item[0]->kill = 1;
        o->w.ring.item[1]->kill = 1;
        o->w.ring.item[4]->kill = 1;
        o->w.ring.item[2]->kill = 1;
        o->w.ring.item[3]->kill = 1;
        o->update = func_800B2EC4;
    }
}
