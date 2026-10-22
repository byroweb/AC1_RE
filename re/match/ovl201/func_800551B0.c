#include "ac1/ac1.h"

/* save/load menu work area (UiObj +0x7C) */
typedef struct {
    UiObj *owner;       /* +0x7C */
    UiObj *arg;         /* +0x80 */
    UiObj *box;         /* +0x84 */
    u8 pad88[0x10];
    s32 status[2];      /* +0x98 per card slot */
    u8 padA0[2];
    s8 cursor;          /* +0xA2 row on screen */
    u8 mode;            /* +0xA3 */
    u8 slot;            /* +0xA4 */
    u8 kind;            /* +0xA5 */
    u8 top;             /* +0xA6 first entry shown */
} SaveWork;

typedef struct { u8 b[0xA0]; } Blob160;
extern Blob160 D_80031A94;
extern Blob160 D_80042AA8;
extern u8 D_80039CAB;
extern u8 D_80039CAD;
extern u8 D_80031A5E;
extern u8 D_80031A5F;
extern u8 D_800B6FCC[];
extern u8 D_800B6FD4[];
extern u8 D_8004B1E8[];
extern u8 D_8004B258[];
extern u8 D_8004B26C[];
extern s16 D_801A2318;

extern void SsUtKeyOn(s16 a, s32 b, s32 c, s32 d, s32 e, s32 f, s32 g);
extern int MoveImage(void *rect, s32 x, s32 y);
extern int DrawSync(int mode);
extern void func_800506EC(s32 id);
extern void func_8005C2D0(s32 x, s32 y, s32 w, s32 h, s32 id);
extern void func_8005DED8(UiObj *o, void *fn, void *msg, s32 n);
extern void func_8005E23C(UiObj *o, void *fn, void *msg, s32 n);
extern s32 func_800684A0(u8 slot);
extern s32 func_80067CF0(u8 slot, s16 n);
extern void func_80083A58(void);
extern void func_80074A18(void);
extern void func_8005EB80(s32 a);
extern void func_80073718(void);
extern void func_80073B7C(void);
extern s16 func_800517C8(void);
extern void func_800A2D0C(UiObj *o);
extern void func_800537E0(UiObj *o);
extern void func_80054258(UiObj *o);
extern void func_8005552C(UiObj *o);

void func_800551B0(UiObj *o)
{
    SaveWork *w = (SaveWork *)&o->w;
    u8 save_cad;
    u8 save_cab;

    w->status[o->w.raw[0x28]] = func_800684A0(o->w.raw[0x28]);
    if (w->status[o->w.raw[0x28]] != 0) {
        func_800506EC(10);
        obj_find_by_id(9)->kill = 1;
        func_8005C2D0(0xC, 0x68, 0x128, 0x20, 9);
        func_8005DED8(o, func_80054258, D_8004B1E8, 0xC);
        SsUtKeyOn(D_801A2318, 0, 0, 0x3C, 0, 0x7F, 0x7F);
    } else {
        if ((u8)func_80067CF0(o->w.raw[0x28], o->w.raw[0x2A] + (s8)o->w.raw[0x26])) {
            save_cad = D_80039CAD;
            save_cab = D_80039CAB;
            D_80042AA8 = D_80031A94;
            func_80083A58();
            func_80074A18();
            if (w->owner->ctor != func_800A2D0C) {
                D_80039CAB = 0;
                D_80039CAD = 0;
                func_8005EB80(1);
                func_80073718();
                func_80073B7C();
                D_80039CAD = save_cad;
                D_80039CAB = save_cab;
            } else {
                func_8005EB80(1);
                func_80073B7C();
            }
            while (func_800517C8() != 0) {
            }
            if ((D_80031A5E != 0xFF && D_80031A5E >= 0x1F) || (D_80031A5F != 0xFF && D_80031A5F >= 0x1F)) {
                MoveImage(D_800B6FCC, 0x300, 0x1C0);
                MoveImage(D_800B6FD4, 0, 0x1ED);
                MoveImage(D_800B6FD4, 0, 0x1EE);
                DrawSync(0);
            }
            if (w->arg == 0) {
                func_8005DED8(o, func_800537E0, D_8004B258, 0xC);
            } else {
                func_8005DED8(o, func_8005552C, D_8004B258, 0xC);
            }
        } else {
            func_8005E23C(o, func_80054258, D_8004B26C, 0xC);
        }
        func_800506EC(10);
        obj_find_by_id(9)->kill = 1;
        func_8005C2D0(0xC, 0x68, 0x128, 0x20, 9);
    }
    o->update = ui_obj_nop2;
}
