#include "ac1/ac1.h"

typedef struct PadRepeat {      /* 0x801A2550 */
    s32 latch;                  /* +0x00 */
    s16 delay;                  /* +0x04 */
    s16 delay0;                 /* +0x06 */
    u8 pad08[0x10];
    s32 now;                    /* +0x18 */
    s32 prev;                   /* +0x1C */
} PadRepeat;
extern PadRepeat D_801A2550;
extern s32 D_801A2574;
extern s16 D_801A2318;

typedef struct AcData {         /* 0xA0 bytes, byte aligned */
    u8 b[0xA0];
} AcData;

typedef struct MenuW {          /* this UiObj +0x7C */
    UiObj *src;                 /* +0x7C object whose work byte +0x7C is the cursor */
    UiObj *box;                 /* +0x80 */
    UiObj *box2;                /* +0x84 */
    u8 pad88;
    char name[12];              /* +0x89 */
} MenuW;

typedef struct MenuObj {
    u8 hdr[0x7C];
    UiObj *src;                 /* +0x7C */
    UiObj *box;                 /* +0x80 */
    UiObj *box2;                /* +0x84 */
} MenuObj;

extern AcData D_80031A94;       /* AC being edited */
extern AcData D_80042AA8;       /* copy */
extern u8 D_80031A59;
extern u8 D_80031A5A;
extern u8 D_80031A5B;
extern u8 D_80031A60;
extern u8 D_80031A61;
extern u8 D_80031A63;
extern s32 D_800485F0;
extern u8 D_8004BD00[];
extern u8 D_8004BD20[];
extern u8 D_8004BD40[];
extern u8 D_8004BD5C[];
extern RECT D_800B858C;
extern RECT D_800B8594;

short SsUtKeyOn(short, short, short, short, short, short, short);
int MoveImage(RECT *, int, int);
int DrawSync(int);
extern void func_80083A58(void);
extern u8 func_8007AA04(s32);
extern void func_8005DED8(UiObj *o, void (*fn)(UiObj *), void *msg, s32 n);
extern void func_800506EC(s32);
extern void func_8005C2D0(s32, s32, s32, s32, s32);
extern void func_80051450(s32, s32);
extern void func_80050AF4(s32, s32);
extern u8 func_80084290(void);
extern void func_8005D8D4(void *, char *, s32, s32);
extern void func_80072448(UiObj *o);
extern void func_800854A8(UiObj *o);
extern void func_80072134(UiObj *o);
extern void func_80072640(UiObj *o);
extern void func_800722D4(UiObj *o);
extern void func_80071328(UiObj *o);
extern void func_80083C58(UiObj *o);
extern void func_80072730(UiObj *o);
extern void func_80072838(void);
extern void func_8005CC48(void);

void func_80070A44(UiObj *o)
{
    MenuW *w = (MenuW *)&o->w;
    s8 *cur = (s8 *)&w->src->w;

    if (D_801A2550.now & 0x1000) {
        D_801A2550.latch |= 0x1000;
        if (D_801A2574 != 0) {
            D_801A2550.delay = 6 / D_801A2574;
            if (!(D_801A2550.prev & 0x1000))
                D_801A2550.delay0 = 12 / D_801A2574;
        } else {
            D_801A2550.delay = 6;
            if (!(D_801A2550.prev & 0x1000))
                D_801A2550.delay0 = 12;
        }
        SsUtKeyOn(D_801A2318, 0, 1, 60, 0, 127, 127);
        *cur = *cur - 1;
    } else if (D_801A2550.now & 0x4000) {
        D_801A2550.latch |= 0x4000;
        if (D_801A2574 != 0) {
            D_801A2550.delay = 6 / D_801A2574;
            if (!(D_801A2550.prev & 0x4000))
                D_801A2550.delay0 = 12 / D_801A2574;
        } else {
            D_801A2550.delay = 6;
            if (!(D_801A2550.prev & 0x4000))
                D_801A2550.delay0 = 12;
        }
        SsUtKeyOn(D_801A2318, 0, 1, 60, 0, 127, 127);
        *cur = *cur + 1;
    }
    if (*cur < 0) {
        *cur = 6;
    } else if ((u32)*cur >= 7) {
        *cur = 0;
    }
    if (D_801A2550.now & 0x40) {
        D_801A2550.latch |= 0x40;
        SsUtKeyOn(D_801A2318, 0, 2, 60, 0, 127, 127);
        switch ((u8)*cur) {
        case 0:
            D_80042AA8 = D_80031A94;
            func_80083A58();
            if (func_8007AA04(0) == 255) {
                func_8005DED8(o, func_80070A44, D_8004BD00, 12);
                o->update = ui_obj_nop2;
            } else {
                func_800506EC(4);
                obj_find_by_id(3)->kill = 1;
                func_8005C2D0(16, 40, 144, 184, 3);
                o->update = func_80072448;
            }
            break;
        case 2:
            if (D_80031A5A == 255 || D_80031A5B == 255 || D_80031A63 == 255
                || (D_80031A63 < 27 && D_80031A59 == 255)
                || D_80031A61 == 255 || D_80031A60 == 255) {
                func_8005DED8(o, func_80070A44, D_8004BD20, 12);
                o->update = ui_obj_nop2;
            } else {
                func_800506EC(4);
                obj_find_by_id(3)->kill = 1;
                func_8005C2D0(16, 40, 144, 184, 3);
                o->update = func_800854A8;
            }
            break;
        case 4:
            func_80051450(4, 8);
            func_800506EC(4);
            obj_find_by_id(3)->kill = 1;
            func_800506EC(6);
            obj_find_by_id(5)->kill = 1;
            func_8005C2D0(176, 24, 128, 144, 5);
            func_8005C2D0(16, 40, 144, 184, 3);
            o->update = func_80072134;
            obj_find_by_handle(func_80072838)->kill = 1;
            break;
        case 3:
            if (D_80031A5A == 255 || D_80031A5B == 255 || D_80031A63 == 255) {
                func_8005DED8(o, func_80070A44, D_8004BD20, 12);
                o->update = ui_obj_nop2;
            } else {
                func_800506EC(4);
                obj_find_by_id(3)->kill = 1;
                func_8005C2D0(16, 40, 144, 184, 3);
                o->update = func_80072640;
            }
            break;
        case 5:
            func_800506EC(4);
            obj_find_by_id(3)->kill = 1;
            func_800506EC(6);
            obj_find_by_id(5)->kill = 1;
            func_8005C2D0(176, 24, 128, 144, 5);
            func_8005C2D0(16, 40, 144, 184, 3);
            w->box2->kill = 1;
            o->update = func_800722D4;
            break;
        case 6:
            func_8005D8D4(D_8004BD40, w->name, 12, 0);
            o->update = func_80071328;
            break;
        case 1:
            D_80042AA8 = D_80031A94;
            func_80083A58();
            if (D_800485F0 == 0) {
                func_8005DED8(o, func_80070A44, D_8004BD5C, 12);
                o->update = ui_obj_nop2;
            } else {
                MoveImage(&D_800B858C, 512, 256);
                MoveImage(&D_800B8594, 320, 511);
                DrawSync(0);
                func_80050AF4(13, func_80084290() + 132);
                func_80051450(17, func_80084290());
                func_800506EC(4);
                obj_find_by_id(3)->kill = 1;
                func_800506EC(6);
                obj_find_by_id(5)->kill = 1;
                func_8005C2D0(176, 24, 128, 144, 5);
                func_8005C2D0(16, 40, 144, 184, 3);
                obj_find_by_handle(func_80072838)->kill = 1;
                o->update = func_80083C58;
            }
            break;
        }
    } else if (D_801A2550.now & 0x20) {
        SsUtKeyOn(D_801A2318, 0, 0, 60, 0, 127, 127);
        ((MenuObj *)o)->box2->kill = 1;
        ((MenuObj *)o)->box->kill = 1;
        func_800506EC(4);
        func_800506EC(6);
        obj_find_by_id(3)->kill = 1;
        obj_find_by_id(5)->kill = 1;
        obj_find_by_handle(func_8005CC48)->kill = 1;
        func_8005C2D0(32, 40, 128, 168, 3);
        func_8005C2D0(176, 24, 128, 144, 5);
        o->update = func_80072730;
    }
}
