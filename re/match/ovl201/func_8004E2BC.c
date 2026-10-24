#include "ac1/ac1.h"
/* ovl201 0x8004E2BC: matched (--rodata 0x8004ADFC); *flags value pinned to v0 with a register asm local */
/* ovl201 0x8004E2BC - garage main menu ring, idle step: confirm opens the item
 * at the front (per-item setup, then the panels fall into rank order behind
 * it), left/right start a ring turn (func_8004E894). */

typedef struct {
    u8 b[0xA0];
} Blob160;

extern s16 D_801A2318;
extern s32 D_801A2550;
extern s32 D_801A2568;
extern s32 D_801A256C;
extern u8 D_80039CAA;
extern Blob160 D_80031A94;
extern Blob160 D_80042AA8;
extern s32 D_800485A0;
extern s32 D_800485A4;
extern u8 D_8004ADD8[];
extern u8 D_800B91A8;
extern u8 D_80031A58[];
extern MATRIX D_800B8E44[5];

extern void SsUtKeyOn(s16 vab, s32 a, s32 b, s32 c, s32 d, s32 e, s32 f);
extern void func_80083A58(void);
extern void func_8005DED8(UiObj *o, void (*fn)(UiObj *), void *p, s32 n);
extern void func_8005EE90(s32 a);
extern void func_8005F054(s32 a);
extern void func_8005F1DC(s32 a);
extern u8 func_8007AA04(s32 slot);
extern void func_8005B024(s32 *p);
extern void func_8008C394(void);
extern void func_80096424(s32 *p);
extern void func_80051450(s32 a, s32 b);
extern void func_80086E80(s32 *p);
extern UiObj *func_8004FC3C(s32 front);
extern void func_8004FB28(s32 front);
extern void func_8004F810(UiObj *o, s32 n);
extern void func_8004FD3C(UiObj *o);
extern void func_8004F588(UiObj *o);
extern void func_8004F7E8(UiObj *o);
extern void func_8004EC24(UiObj *o);
extern void func_8004E894(UiObj *o);
extern UiObj *func_8005CC48(void);

void func_8004E2BC(UiObj *o)
{
    s16 i;
    s16 rank;
    s8 prev;
    s16 j;
    s32 *flags;

    o->w.ring.arrow[0]->visible = 1;
    o->w.ring.arrow[1]->visible = 1;
    if (((D_801A2568 & 0x40) && !(D_801A256C & 0x40)) || D_80039CAA) {
        flags = &D_801A2550;
        i = o->w.ring.front;
        {
            register s32 fv asm("$2");
            fv = *flags;
            *flags = fv | 0x40;
        }
        rank = 1;
        switch (i) {
        case 0:
            D_80042AA8 = D_80031A94;
            func_80083A58();
            if (D_800485A0 == 0 && D_800485A4 == 0) {
                func_8005DED8(o, func_8004E2BC, D_8004ADD8, 12);
                o->update = ui_obj_nop2;
                return;
            }
            if (D_800B91A8) {
                func_8005EE90(0);
                func_8005F054(0);
                if (D_80031A58[0] == 0xFF) {
                    D_80031A58[0] = func_8007AA04(0);
                }
            } else {
                func_8005F1DC(0);
            }
            break;
        case 1:
            func_8005B024(flags);
            func_8008C394();
            break;
        case 4:
            func_80096424(flags);
            break;
        case 5:
            func_80051450(4, 10);
            break;
        case 2:
            func_80086E80(flags);
            break;
        }
        SsUtKeyOn(D_801A2318, 0, 2, 60, 0, 127, 127);
        o->w.ring.lead = func_8004FC3C(o->w.ring.front);
        o->w.ring.item[i++]->kill = 1;
        obj_find_by_handle(func_8004FB28)->kill = 1;
        func_8004F810(o->w.ring.lead, 1);
        func_8004F810(o->w.ring.lead, 2);
        func_8004F810(o->w.ring.lead, 3);
        o->w.ring.lead->update = func_8004FD3C;
        while (1) {
            if (i == 6) {
                i = 0;
            }
            if (i == o->w.ring.front) {
                break;
            }
            o->w.ring.item[i]->w.panel.rank = rank++;
            o->w.ring.item[i]->update = func_8004F588;
            o->w.ring.item[i]->w.panel.label->update = func_8004F7E8;
            i++;
        }
        o->update = func_8004EC24;
        obj_find_by_handle(func_8005CC48)->kill = 1;
        o->w.ring.arrow[0]->kill = 1;
        o->w.ring.arrow[1]->kill = 1;
    } else if (D_801A2568 & 0xA000) {
        prev = o->w.ring.front;
        if (D_801A2568 & 0x8000) {
            SsUtKeyOn(D_801A2318, 0, 1, 60, 0, 127, 127);
            if (++o->w.ring.front == 6) {
                o->w.ring.front = 0;
            }
            o->w.ring.dir = -32;
            o->w.ring.step = 0;
        } else {
            SsUtKeyOn(D_801A2318, 0, 1, 60, 0, 127, 127);
            if (--o->w.ring.front == -1) {
                o->w.ring.front = 5;
            }
            o->w.ring.dir = 32;
            o->w.ring.step = 0;
        }
        o->update = func_8004E894;
        o->w.ring.lead->kill = 1;
        o->w.ring.arrow[0]->visible = 0;
        o->w.ring.arrow[1]->visible = 0;
        obj_find_by_handle(func_8004FB28)->kill = 1;
        func_8004FB28(o->w.ring.front);
        for (j = 0; j < 6; j++) {
            o->w.ring.item[j]->w.panel.label->light = 6;
        }
        o->w.ring.item[o->w.ring.front]->w.panel.label->light = 7;
        o->w.ring.item[prev]->w.panel.label->light = 8;
        D_800B8E44[3] = D_800B8E44[2];
        D_800B8E44[4] = D_800B8E44[0];
    }
}
