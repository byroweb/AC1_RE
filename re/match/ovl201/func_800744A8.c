#include "ac1/ac1.h"
/* local view of the assembly-screen object's work area (UiObj +0x7C) */
typedef struct AsmWork {
    void *src;          /* +0x7C */
    UiObj *owner;       /* +0x80 */
    u8 b84, b85;        /* +0x84 */
    u8 b86, b87;        /* +0x86 */
    UiObj *sub;         /* +0x88 */
    u8 pad8c[4];
    u16 h90;            /* +0x90 part category */
    u16 h92;            /* +0x92 selector mode */
    s16 h94;            /* +0x94 */
    u8 b96;             /* +0x96 */
    u8 b97;             /* +0x97 */
    u8 b98, b99, b9a;   /* +0x98 */
    u8 pad9b;
    UiObj *bar;         /* +0x9C */
    u8 bA0, bA1;        /* +0xA0 */
    s16 hA2, hA4;       /* +0xA2 */
    u16 hA6;            /* +0xA6 part index in the category, 255 = none */
    s16 hA8;            /* +0xA8 */
    u8 padAA;
    u8 bAB;             /* +0xAB */
    s16 hAC;            /* +0xAC */
    u8 bAE;             /* +0xAE */
    u8 padAF;
    u16 hB0;            /* +0xB0 */
} AsmWork;

typedef struct AsmObj {
    u8 head[0x7C];
    AsmWork w;
} AsmObj;

extern u8 D_80031A58[];
extern u16 D_80039CA0;
extern u8 D_80039CA2;
extern char D_8004BEA8[];
extern char D_8004BEAC[];
extern char D_8004BEB4[];
extern char D_8004BEBC[];
extern char D_8004BEC0[];
extern char D_8004BEC8[];
extern char D_8004BECC[];
extern char D_8004BED4[];
extern s32 D_800B824C;
extern s32 D_800B8250;
extern char D_800B83D0[];
extern char D_800B843C[];
extern UiObj *D_8017EF80[7];
extern s32 D_801BC480;
extern s32 D_801F36D8[];

void func_80051078(s32);
void func_8005CAE0(void *, s32, s32, s32);
void func_8005D15C(s32, s32, s32);
void func_8005D754(s32, void *, s32, s32, s32);
UiObj *func_8005D80C(s32 layer, char *s, s32 x, s32 w, s32 y, s32 kind);
void func_8005DED8(UiObj *, void (*)(UiObj *), void *, s32);
void func_800740AC(UiObj *);
void func_80074A18(void);
void func_800788C8(s32, s32);
void func_80078A38(UiObj *o);
void func_8007BCE8(s32);

UiObj *func_800744A8(void *src, UiObj *owner, u8 mode)
{
    UiObj *o;
    AsmWork *w;
    u8 p0, p2, p3, p1, p8, p9;

    o = ui_obj_alloc(0);
    o->ctor = func_800744A8;
    o->update = func_80078A38;
    o->destroy = ui_obj_nop;
    o->visible = 0;
    o->attached = 0;
    o->parent = 0;
    o->has_xform = 0;
    func_8005CAE0(D_800B83D0, 0, 0, 4);
    func_8007BCE8(0x4C);
    func_8007BCE8(0x52);
    func_8007BCE8(0x55);
    func_8007BCE8(0x44);
    func_8005D754(2, D_800B843C, 8, 8, 4);
    func_8005D80C(1, D_8004BEA8, 6, 8, 8, 8);
    func_8005D80C(1, D_8004BEAC, 6, 8, 16, 8);
    func_8005D80C(1, D_8004BEB4, 6, 8, 24, 8);
    func_8005D80C(1, D_8004BEBC, 6, 8, 40, 8);
    func_8005D80C(1, D_8004BEC0, 6, 8, 48, 8);
    func_8005D80C(1, D_8004BEA8, 6, 8, 132, 4);
    func_8005D80C(1, D_8004BEC8, 6, 8, 140, 4);
    func_8005D80C(1, D_8004BECC, 6, 8, 172, 4);
    ((AsmObj *)o)->w.src = src;
    ((AsmObj *)o)->w.owner = owner;
    ((AsmObj *)o)->w.b84 = 0;
    ((AsmObj *)o)->w.b85 = 0;
    D_8017EF80[0] = &g_ui_none;
    D_8017EF80[1] = &g_ui_none;
    D_8017EF80[2] = &g_ui_none;
    D_8017EF80[3] = &g_ui_none;
    D_8017EF80[4] = &g_ui_none;
    D_8017EF80[5] = &g_ui_none;
    D_8017EF80[6] = &g_ui_none;
    ((AsmObj *)o)->w.sub = &g_ui_none;
    ((AsmObj *)o)->w.b97 = mode;
    ((AsmObj *)o)->w.b98 = D_80031A58[1];
    ((AsmObj *)o)->w.b99 = D_80031A58[4];
    ((AsmObj *)o)->w.b9a = D_80031A58[5];
    ((AsmObj *)o)->w.bAE = 0;
    ((AsmObj *)o)->w.hB0 = D_80039CA0;
    {
    u8 p11;
    p0 = D_80031A58[0];
    p2 = D_80031A58[2];
    p3 = D_80031A58[3];
    p11 = D_80031A58[11];
    p1 = D_80031A58[1];
    p8 = D_80031A58[8];
    p9 = D_80031A58[9];
    func_80074A18();
    w = &((AsmObj *)o)->w;
    if (p0 != D_80031A58[0] || p2 != D_80031A58[2] || p3 != D_80031A58[3] || p11 != D_80031A58[11]
        || p1 != D_80031A58[1] || p8 != D_80031A58[8] || p9 != D_80031A58[9]) {
        func_8005DED8(o, func_80078A38, D_8004BED4, 12);
        o->update = ui_obj_nop2;
    }
    }
    if (D_80031A58[2] != 255) {
        func_8005D15C(D_800B824C, 0x3C0, 0);
        w->h92 = 2;
        w->h94 = 3;
        w->b96 = 0;
        w->bA0 = 0x43;
        w->hA6 = w->hA8 = D_80031A58[2];
        w->bAB = 10;
        w->hAC = D_80039CA2 * 10;
        w->h90 = 4;
        w->bar = &g_ui_none;
        w->bA1 = 1;
        w->hA2 = 0;
        w->hA4 = 0;
        w->b86 = D_80031A58[5];
        w->b87 = D_80031A58[4];
        if (D_801F36D8[0] != 0) {
            func_80051078(D_801F36D8[0]);
            D_801F36D8[0] = 0;
            D_801BC480 = 0;
        }
        func_800788C8(1, 3);
    } else {
        func_8005D15C(D_800B8250, 0x3C0, 0);
        w->h94 = 4;
        w->h92 = 0;
        w->b96 = 1;
        w->bA0 = 0x43;
        w->hA6 = w->hA8 = D_80031A58[0];
        w->bAB = 7;
        w->hAC = D_80039CA2 * 3;
        w->h90 = 0;
        w->bar = &g_ui_none;
        w->bA1 = 1;
        w->hA2 = 0;
        w->hA4 = 0;
        w->b86 = D_80031A58[5];
        w->b87 = D_80031A58[4];
        if (D_801F36D8[0] != 0) {
            func_80051078(D_801F36D8[0]);
            D_801F36D8[0] = 0;
            D_801BC480 = 0;
        }
        func_800788C8(1, 4);
    }
    func_800740AC(o);
    return o;
}
