#include "ac1/ac1.h"
/* ovl201 0x8004DCC0 - build the invisible root object of the ring menu
 * (update func_8004DEB0): upload its CLUT and texture strip, open a
 * full-screen box, build the ring (func_8004F098) and, when either of the
 * D_80031A5E / D_80031A5F bytes is set and >= 31, copy two VRAM areas. */

typedef struct {                /* UiObj work area of the ring-menu root */
    UiObj *ring;                /* +0x7C */
    u8 pad80[0x2E];
    u8 unkAE;                   /* +0xAE */
    u8 padAF[2];
    u8 flagB1;                  /* +0xB1 read by func_8004FB28 / func_8004FC3C */
} RootWork;

extern u8 D_80031A5E;
extern u8 D_80031A5F;
extern s32 D_801A2574;
extern u32 D_8004AC18[];
extern u32 D_80042B70[];
extern void func_8004DEB0(UiObj *o);
extern UiObj *func_8004F098(s32 a, s32 b, s32 c, s32 d, s32 e, s32 f, s32 g);
extern void func_8004F924(void);
extern int LoadImage(RECT *rect, u32 *p);
extern int MoveImage(RECT *rect, int x, int y);
extern int DrawSync(int mode);
extern void box_init(s32 x, s32 y, s32 w, s32 h, s32 a, s32 b, s32 c);

UiObj *func_8004DCC0(void)
{
    UiObj *o;
    RootWork *w;

    o = ui_obj_alloc(0);
    o->ctor = func_8004DCC0;
    o->update = func_8004DEB0;
    o->destroy = ui_obj_nop;
    o->visible = 0;
    o->attached = 0;
    o->parent = 0;
    o->has_xform = 0;
    w = (RootWork *)&o->w;
    w->flagB1 = 0;
    w->unkAE = 0;
    {
        RECT tex = { 448, 192, 16, 64 };
        RECT clut = { 368, 225, 16, 1 };

        LoadImage(&clut, D_8004AC18);
        LoadImage(&tex, D_80042B70);
    }
    DrawSync(0);
    box_init(0, 0, 320, 240, 1, 0, 0);
    w->ring = func_8004F098(0, 0, -2870, 0, 0, 0, 2);
    if ((D_80031A5E != 255 && D_80031A5E >= 31) || (D_80031A5F != 255 && D_80031A5F >= 31)) {
        RECT a = { 768, 448, 64, 64 };
        RECT b = { 0, 493, 256, 1 };

        MoveImage(&a, 384, 256);
        MoveImage(&b, 320, 482);
        DrawSync(0);
    }
    func_8004F924();
    D_801A2574 = 0;
    return o;
}
