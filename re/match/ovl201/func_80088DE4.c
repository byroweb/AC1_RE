#include "ac1/ac1.h"

typedef struct {                /* 0x60 */
    u8 c0, c1, c2, c3;
    u8 pad04[0x5C];
} GarageEnt;

typedef struct {
    s8 count;                   /* +0x000 */
    s8 rows;                    /* +0x001 */
    u8 b2, b3, b4, b5, b6;      /* +0x002 */
    s8 b7;                      /* +0x007 */
    s8 b8;                      /* +0x008 */
    u8 b9;                      /* +0x009 */
    u8 bA;                      /* +0x00A */
    u8 padB;
    u8 bC, bD, bE;              /* +0x00C */
    u8 padF[0x26 - 0xF];
    s16 h26;                    /* +0x026 */
    u8 pad28[0x3C - 0x28];
    UiObj *label2[5];           /* +0x03C */
    s32 w50[3];                 /* +0x050 */
    GarageEnt ent[5];           /* +0x05C */
    u8 c23C[3];                 /* +0x23C */
    u8 pad23F[0x2F0 - 0x23F];
    char text[0x100];           /* +0x2F0 */
} NameList;

typedef struct {
    u8 pad0[0x79];
    s8 cursor;                  /* +0x079 0x801F37C9 */
    u8 pad7A[0x84 - 0x7A];
    s8 order[64];               /* +0x084 0x801F37D4 */
    u8 padC4[0xC54 - 0xC4];
    struct { s16 v[16]; } tbl[64]; /* +0xC54 0x801F43A4 text ids per mission/page (row struct: retail index order) */
    u8 pad1454[0x1944 - 0x1454];
    NameList *list;             /* +0x1944 0x801F5094 */
} Garage3750;

extern Garage3750 D_801F3750;
extern s16 D_800B8F44[9];
extern s8 D_801F37D4[64];
extern s8 D_801F37C9;
extern UiObj *func_8005CB94(void *p, s32 x, s32 y, s32 n);
extern void func_80088A18(s32 i);
extern void func_800891BC(UiObj *o);
extern void func_800887B8(char *s);
extern void func_80015538(void *p, s32 v, s32 n);
extern void func_800513F0(s32 a, u16 b, void *cb);

UiObj *func_80088DE4(void)
{
    UiObj *o;
    s32 i;
    s32 j;
    s32 n;

    o = ui_obj_alloc(6);
    o->ctor = func_80088DE4;
    o->update = func_800891BC;
    o->destroy = ui_obj_nop;
    o->visible = 0;
    o->attached = 0;
    o->parent = 0;
    o->has_xform = 0;
    i = 5;
    while (--i != -1) {
        D_801F3750.list->label2[i] = func_8005CB94(&D_801F3750.list->ent[i], 8, (s16)(i * 16 + 5), 6);
        D_801F3750.list->label2[i]->light = 12;
        func_80088A18(i);
    }
    D_800B8F44[0] = 128;
    D_800B8F44[1] = 128;
    D_800B8F44[2] = 128;
    D_800B8F44[3] = 128;
    D_800B8F44[4] = 128;
    D_800B8F44[5] = 128;
    D_800B8F44[6] = 128;
    D_800B8F44[7] = 128;
    D_800B8F44[8] = 128;
    D_801F3750.list->bC = 0;
    D_801F3750.list->h26 = 0;
    D_801F3750.list->b2 = 0;
    D_801F3750.list->b3 = 0;
    D_801F3750.list->b4 = 0;
    D_801F3750.list->b5 = 0;
    D_801F3750.list->b6 = 0;
    D_801F3750.list->b7 = 0;
    D_801F3750.list->b9 = 0;
    D_801F3750.list->bD = 0;
    D_801F3750.list->bE = 0;
    j = 3;
    while (--j != -1) {
        D_801F3750.list->c23C[j] = 0;
        D_801F3750.list->w50[j] = 0;
    }
    D_801F3750.list->b8 = -1;
    D_801F3750.list->bA = 0;
    func_80015538(D_801F3750.list->text, 0, 0x100);
    n = D_801F3750.tbl[D_801F3750.order[D_801F3750.cursor]].v[D_801F3750.list->b7];
    if (n >= 0) {
        D_801F3750.list->b9 = 1;
        func_800513F0(19, n + 1, func_800887B8);
    } else {
        D_801F3750.list->text[0] = '/';
        D_801F3750.list->text[1] = 0;
    }
    {
    RECT r = { 0, 0, 50, 100 };

    r.x = 896;
    r.y = 0;
    ClearImage(&r, 255, 255, 255);
    r.x = 960;
    r.y = 0;
    ClearImage(&r, 255, 255, 255);
    r.x = 896;
    r.y = 128;
    ClearImage(&r, 255, 255, 255);
    r.w = 256;
    r.h = 1;
    r.x = 0;
    r.y = 481;
    ClearImage(&r, 0, 0, 0);
    r.x = 0;
    r.y = 482;
    ClearImage(&r, 0, 0, 0);
    r.x = 0;
    r.y = 483;
    ClearImage(&r, 0, 0, 0);
    }
    DrawSync(0);
    return o;
}
