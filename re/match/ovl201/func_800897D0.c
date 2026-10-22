#include "ac1/ac1.h"

typedef struct LetterLine {
    u8 text[0x50];              /* +0x00 control codes + text */
    s32 pos;                    /* +0x50 read position, 0xFFFF = line done */
    s32 next;                   /* +0x54 */
    s32 len;                    /* +0x58 */
    s32 ended;                  /* +0x5C */
} LetterLine;

typedef struct {
    s8 count;                   /* +0x000 */
    s8 rows;                    /* +0x001 */
    s8 more;                    /* +0x002 next page waiting */
    s8 last;                    /* +0x003 */
    s8 down;                    /* +0x004 scroll-down steps left */
    s8 up;                      /* +0x005 scroll-up steps left */
    s8 page;                    /* +0x006 */
    s8 pages;                   /* +0x007 */
    u8 pad8;
    u8 loading;                 /* +0x009 */
    u8 padA;
    u8 tick;                    /* +0x00B */
    s8 reset;                   /* +0x00C */
    u8 hist_n;                  /* +0x00D */
    u8 hist_max;                /* +0x00E */
    u8 padF;
    s16 hist[11];               /* +0x010 */
    s16 scroll;                 /* +0x026 */
    u8 pad28[0x50 - 0x28];
    UiObj *win[3];              /* +0x050 */
    LetterLine line[5];         /* +0x05C */
    u8 pad23C[0x2F0 - 0x23C];
    u8 text[1];                 /* +0x2F0 */
} NameList;

typedef struct {
    u8 pad0;
    s8 b1;                      /* +0x001 0x801F3751 */
    u8 pad2[0x79 - 2];
    s8 cursor;                  /* +0x079 0x801F37C9 */
    u8 pad7A[0x84 - 0x7A];
    s8 order[64];               /* +0x084 0x801F37D4 */
    u8 padC4[0xC54 - 0xC4];
    struct { s16 v[16]; } tbl[64]; /* +0xC54 0x801F43A4 */
    u8 pad1454[0x1944 - 0x1454];
    NameList *list;             /* +0x1944 0x801F5094 */
} Garage3750;

extern Garage3750 D_801F3750;
extern u32 D_801A2568;
extern u32 D_801A256C;
extern s8 D_801A2309;
extern s8 D_801A230A;
extern s8 D_801A230B;
extern s8 D_801A230C;
extern s8 D_801A230D;
extern s8 D_801A230E;
extern void *D_801A2310;
extern void *D_801A2314;
extern u8 D_800B8AB0[];
extern void func_8005BA2C();
extern void func_8008B574();
extern void func_8005CC48();
extern UiObj *func_8008729C(void);
extern void func_80086E04(void);
extern void func_800887B8(char *s);
extern void func_80088A18(s32 i);
extern void func_80088AC4(LetterLine *l, u8 *src, s32 more);
extern void func_800513F0(s32 a, u16 b, void *cb);
extern s32 func_800892AC(void);
extern s32 func_800867C4(void);
extern s32 func_8008935C(void);
extern s32 func_800895D4(void);
extern void func_800AE30C(void);
extern void func_8008A338(void);
extern void func_8008B604(UiObj *o);
extern void func_8008A2F0(UiObj *o);
extern void func_800885DC(UiObj *o);
extern void func_80087AEC(UiObj *o);
extern void func_8008B8D0(UiObj *o);
extern void func_8008BA70(UiObj *o);
extern void func_8008BC10(UiObj *o);
extern void func_800506EC(s32 id);
extern void func_8005C2D0(s32 x, s32 y, s32 w, s32 h, s32 id);

void func_800897D0(UiObj *o)
{
    s16 a;
    s16 b;
    UiObj *mark;
    s32 i;
    u8 *src;
    s32 fast;
    s32 n;
    s8 v;
    s8 msg;

    a = 0;
    b = 0;
    mark = obj_find_by_handle(func_8008B574);
    for (i = 4; i != -1; i--) {
        a += D_801F3750.list->line[i].next;
    }
    src = &D_801F3750.list->text[D_801F3750.list->scroll] + a;
    if (obj_find_by_handle(func_8005BA2C) != &g_ui_none) {
        return;
    }
    if (D_801F3750.list->down > 0) {
        D_801F3750.list->down--;
        D_801F3750.list->scroll += D_801F3750.list->line[0].next;
        for (i = 0; i < 4; i++) {
            D_801F3750.list->line[i] = D_801F3750.list->line[i + 1];
        }
        func_80088A18(4);
        if (D_801F3750.list->down == 0 && D_801F3750.list->reset != 0) {
            D_801F3750.list->scroll = 0;
            D_801F3750.list->reset = 0;
        }
        return;
    }
    if (D_801F3750.list->up > 0) {
        D_801F3750.list->up--;
        for (i = 3; i != -1; i--) {
            D_801F3750.list->line[i + 1] = D_801F3750.list->line[i];
        }
        func_80088A18(0);
        if (D_801F3750.list->up == 0 && D_801F3750.list->reset != 0) {
            D_801F3750.list->scroll = 0;
            D_801F3750.list->reset = 0;
        }
        return;
    }
    if (D_801F3750.list->loading == 0) {
        fast = 0;
        if (D_801F3750.list->last != 0) {
            fast = 1;
        } else if ((D_801A2568 & 0x40) || (D_801A256C & 0x40) || (D_801A2568 & 0x1000) ||
            (D_801A256C & 0x1000) || (D_801A2568 & 0x4000) || (D_801A256C & 0x4000)) {
            fast = 1;
        }
        for (i = 0; i < 5; i++) {
            if (D_801F3750.list->line[i].pos != 0xFFFF) {
                func_80088AC4(&D_801F3750.list->line[i], src, fast);
                break;
            }
        }
    }
    D_801F3750.list->tick++;
    for (i = 4; i != -1; i--) {
        b += D_801F3750.list->line[i].next;
    }
    if ((D_801F3750.list->line[4].ended != 0 || D_801F3750.list->more != 0)
        && (D_801F3750.list->last == 0 || D_801F3750.list->page < D_801F3750.list->pages)
        && mark->visible == 0) {
        mark->visible = 1;
    }
    if ((D_801A2568 & 0x1000) && !(D_801A256C & 0x1000)) {
        if (D_801F3750.list->more != 0) {
            if (D_801F3750.list->page > 0) {
                D_801F3750.list->page--;
                n = D_801F3750.tbl[D_801F3750.order[D_801F3750.cursor]].v[D_801F3750.list->page];
                if (n >= 0) {
                    D_801F3750.list->loading = 1;
                    func_800513F0(19, n + 1, func_800887B8);
                }
                D_801F3750.list->more = 0;
                D_801F3750.list->reset = 1;
                goto set_up;
            }
            if (D_801F3750.list->scroll != 0 && a == b) {
                D_801F3750.list->hist_n--;
                D_801F3750.list->more = 0;
                goto back;
            }
        } else if (D_801F3750.list->line[4].ended != 0 && D_801F3750.list->scroll != 0 && a == b) {
            D_801F3750.list->hist_n--;
back:
            D_801F3750.list->scroll -= D_801F3750.list->hist[D_801F3750.list->hist_n];
set_up:
            D_801F3750.list->up = 5;
            mark->visible = 0;
        }
    } else if ((D_801A2568 & 0x4000) && !(D_801A256C & 0x4000) && D_801F3750.list->last != 0) {
        if (D_801F3750.list->more != 0) {
            if (D_801F3750.list->page < D_801F3750.list->pages) {
                D_801F3750.list->page++;
                n = D_801F3750.tbl[D_801F3750.order[D_801F3750.cursor]].v[D_801F3750.list->page];
                if (n >= 0) {
                    D_801F3750.list->loading = 1;
                    func_800513F0(19, n + 1, func_800887B8);
                }
                D_801F3750.list->more = 0;
                D_801F3750.list->reset = 1;
                D_801F3750.list->down = 5;
                mark->visible = 0;
            }
        } else if (D_801F3750.list->line[4].ended != 0 && a == b) {
            D_801F3750.list->hist[D_801F3750.list->hist_n] = a;
            D_801F3750.list->hist_n++;
            if (D_801F3750.list->hist_max < D_801F3750.list->hist_n) {
                D_801F3750.list->hist_max = D_801F3750.list->hist_n;
            }
            D_801F3750.list->down = 5;
            mark->visible = 0;
        }
    } else if (((D_801A2568 & 0x40) && !(D_801A256C & 0x40)) || ((D_801A2568 & 0x4000) && !(D_801A256C & 0x4000))) {
        if (D_801F3750.list->last == 0) {
            if (D_801F3750.list->more != 0) {
                if (D_801F3750.list->page < D_801F3750.list->pages) {
                    D_801F3750.list->page++;
                } else {
                    D_801F3750.list->page++;
                    D_801F3750.list->pages++;
                }
                n = D_801F3750.tbl[D_801F3750.order[D_801F3750.cursor]].v[D_801F3750.list->page];
                if (n >= 0) {
                    D_801F3750.list->loading = 1;
                    func_800513F0(19, n + 1, func_800887B8);
                }
                D_801F3750.list->more = 0;
                D_801F3750.list->reset = 1;
                D_801F3750.list->down = 5;
                mark->visible = 0;
            } else if (D_801F3750.list->line[4].ended != 0 && a == b) {
                D_801F3750.list->hist[D_801F3750.list->hist_n] = a;
                D_801F3750.list->hist_n++;
                if (D_801F3750.list->hist_max < D_801F3750.list->hist_n) {
                    D_801F3750.list->hist_max = D_801F3750.list->hist_n;
                }
                D_801F3750.list->down = 5;
                mark->visible = 0;
            }
        } else {
            mark->update = func_8008B604;
            o->update = ui_obj_nop2;
            if (func_800892AC() & 0xFF) {
                D_801A2309 = 1;
                D_801A230A = 8;
                D_801A2314 = func_80086E04;
                D_801A230B = 6;
                D_801A230C = 14;
                D_801A230D = 13;
                D_801A230E = 6;
                D_801A2310 = D_800B8AB0;
                func_800AE30C();
            } else if (func_800867C4() != 0) {
                func_8008A338();
            } else if (func_8008935C() & 0xFF) {
                D_801A2309 = 1;
                D_801A230A = 9;
                D_801A2314 = func_80086E04;
                D_801A230B = 6;
                D_801A230C = 14;
                D_801A230D = 13;
                D_801A230E = 6;
                D_801A2310 = D_800B8AB0;
                func_800AE30C();
            } else if (func_800895D4() & 0xFF) {
                D_801A2309 = 1;
                D_801A230A = 10;
                D_801A2314 = func_80086E04;
                D_801A230B = 6;
                D_801A230C = 14;
                D_801A230D = 13;
                D_801A230E = 6;
                D_801A2310 = D_800B8AB0;
                func_800AE30C();
            } else {
                func_8008A338();
            }
        }
    } else if ((D_801A2568 & 0x20) && !(D_801A256C & 0x20)) {
        mark->update = func_8008B604;
        func_800506EC(6);
        obj_find_by_id(5)->kill = 1;
        func_8005C2D0(8, 128, 304, 96, 5);
        o->update = func_8008A2F0;
        if (D_801F3750.b1 >= 0) {
            obj_find_by_handle(func_8005CC48)->kill = 1;
            obj_find_by_handle(func_8008729C)->update = func_800885DC;
        } else {
            obj_find_by_handle(func_8008729C)->update = func_80087AEC;
        }
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
    }
}
