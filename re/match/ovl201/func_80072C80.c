#include "ac1/ac1.h"

typedef struct Vec3u {
    u16 x, y, z, pad;
} Vec3u;

typedef struct CoreJoints {     /* 0x28 per core id */
    Vec3u j[5];
} CoreJoints;

typedef struct PartOfsTbl {     /* 16 bytes per part kind */
    Vec3u *ofs;                 /* indexed by part id */
    u8 pad[12];
} PartOfsTbl;

typedef struct AsmPartW {       /* part UiObj +0x7C */
    u16 slot;                   /* +0x7C index into the equipped-part bytes */
    u16 pad7e;
    UiObj *root;                /* +0x80 */
    UiObj *core;                /* +0x84 */
} AsmPartW;

typedef struct AsmPartObj {
    u8 pad[0xC];
    u16 var;                    /* +0x0C */
    u16 kind;                   /* +0x0E */
} AsmPartObj;

typedef struct AsmOwner {
    u8 hdr[0x84];
    u8 b84;                     /* +0x84 */
    u8 b85;                     /* +0x85 */
    u8 pad86[2];
    UiObj *arms;                /* +0x88 */
    UiObj *sel;                 /* +0x8C */
    u8 pad90[4];
    u16 extra;                  /* +0x94 */
} AsmOwner;

extern u8 D_80031A58[];         /* equipped part ids */
extern u8 D_80031A5B;
extern u8 D_80031A5E;
extern u8 D_80031A5F;
extern u8 D_80031A63;
extern Vec3u D_800BC428[];      /* per core id */
extern CoreJoints D_800BC440[]; /* per core id */
extern Vec3u D_800BC638[];
extern Vec3u D_800BC6B8[];
extern Vec3u D_800BC968[];
extern PartOfsTbl D_800BCAC4[];
extern void func_800744A8(void);
extern void func_800804F0(void);
extern void func_800736F0(UiObj *o);
extern UiObj *func_800500EC(void *ctor, UiObj *from);
extern s16 func_800517C8(void);

#define PW(q) ((AsmPartW *)&(q)->w)
#define CORE D_80031A58[0]

UiObj *func_80072C80(AsmOwner *owner, u16 kind, UiObj *parent, UiObj *parent2, u16 var)
{
    UiObj *o;
    UiObj *p;
    AsmPartW *w;
    s16 x, y, z;
    s32 tx, ty, tz;
    s32 ux, uy, uz;
    u16 slot, tbl;
    u8 v, part;
    Vec3u *e;

    o = ui_obj_alloc(6);
    o->ctor = func_80072C80;
    o->update = func_800736F0;
    o->destroy = ui_obj_nop;
    o->render_type = 0;
    ((AsmPartObj *)o)->kind = kind;
    o->light = 18;
    o->rot.vx = 0;
    o->rot.vy = 0;
    o->rot.vz = 0;
    ((AsmPartObj *)o)->var = var;
    if (owner->b84 && owner->b85 && owner->extra == kind) {
        o->light = 22;
    }
    if (owner != (AsmOwner *)&g_ui_none) {
        if (owner->b84 == 0) {
            owner->sel = o;
            switch (kind) {
            case 11:
            case 12:
            case 13:
                parent2->render_type = 2;
                parent2->light = 11;
                parent2->view = 2;
                ((AsmPartObj *)o)->var = 0;
                ((AsmPartObj *)o)->kind = 15;
                o->light = 15;
                break;
            case 7:
                {
                    UiObj *a = ((AsmOwner *)obj_find_by_handle(func_800744A8))->arms;
                    a->render_type = 2;
                    a->light = 11;
                    a->view = 2;
                }
                ((AsmPartObj *)o)->var = 0;
                ((AsmPartObj *)o)->kind = 15;
                o->light = 15;
                break;
            default:
                if (owner->extra == kind) {
                    o->render_type = 2;
                    o->light = 15;
                    o->view = 2;
                }
                break;
            }
        } else {
            switch (kind) {
            case 7:
            case 11:
            case 12:
            case 13:
                o->visible = 0;
                break;
            }
        }
    }
    if (parent != &g_ui_none) {
        o->parent = parent;
    }
    w = PW(o);
    if (kind == 6) {
        w->root = o;
    } else {
        w->root = parent;
    }
    w->core = parent2;
    z = 0;
    y = 0;
    x = 0;
    switch (kind) {
    case 3:
        tbl = 4;
        x = D_800BC428[CORE].x + D_800BC440[CORE].j[0].x;
        y = D_800BC428[CORE].y + D_800BC440[CORE].j[0].y;
        z = D_800BC428[CORE].z + D_800BC440[CORE].j[0].z;
        slot = 2;
        break;
    case 4:
        tbl = 0;
        slot = 0;
        break;
    case 5:
        tbl = 2;
        slot = 3;
        x = D_800BC428[CORE].x + D_800BC440[CORE].j[1].x;
        y = D_800BC428[CORE].y + D_800BC440[CORE].j[1].y;
        z = D_800BC428[CORE].z + D_800BC440[CORE].j[1].z;
        break;
    case 6:
        tbl = 1;
        slot = 11;
        break;
    case 7:
        tbl = 3;
        slot = 4;
        p = obj_find_by_handle(func_80072C80);
        while (p != &g_ui_none && PW(p)->slot != 3) {
            p = func_800500EC(func_80072C80, p);
        }
        v = D_80031A58[PW(parent)->slot];
        if (v < 21 || v == 255) {
            tx = D_800BC638[D_80031A58[PW(p)->slot]].x;
            ty = D_800BC638[D_80031A58[PW(p)->slot]].y - 80;
            tz = D_800BC638[D_80031A58[PW(p)->slot]].z + 120;
        } else {
            tx = D_800BC6B8[D_80031A58[PW(p)->slot]].x;
            ty = D_800BC6B8[D_80031A58[PW(p)->slot]].y + 80;
            tz = D_800BC6B8[D_80031A58[PW(p)->slot]].z + 130;
        }
        x = tx + (D_800BC428[CORE].x + D_800BC440[CORE].j[1].x);
        y = ty + (D_800BC428[CORE].y + D_800BC440[CORE].j[1].y);
        z = tz + (D_800BC428[CORE].z + D_800BC440[CORE].j[1].z);
        break;
    case 8:
        tbl = 7;
        slot = 5;
        p = obj_find_by_handle(func_80072C80);
        while (p != &g_ui_none && PW(p)->slot != 3) {
            p = func_800500EC(func_80072C80, p);
        }
        v = D_80031A58[PW(parent)->slot];
        if (v < 21 || v == 255) {
            ux = D_800BC638[D_80031A58[PW(p)->slot]].x;
            uy = D_800BC638[D_80031A58[PW(p)->slot]].y;
            uz = D_800BC638[D_80031A58[PW(p)->slot]].z;
            o->rot.vx = 512;
            o->rot.vy = 0;
            o->rot.vz = 0;
        } else {
            ux = D_800BC6B8[D_80031A58[PW(p)->slot]].x + D_800BC968[D_80031A58[slot]].x;
            uy = D_800BC6B8[D_80031A58[PW(p)->slot]].y + D_800BC968[D_80031A58[slot]].y;
            uz = D_800BC6B8[D_80031A58[PW(p)->slot]].z + D_800BC968[D_80031A58[slot]].z;
            o->rot.vx = 0;
            o->rot.vy = 0;
            o->rot.vz = 0;
        }
        x = -(ux + (D_800BC428[CORE].x + D_800BC440[CORE].j[1].x));
        y = uy + (D_800BC428[CORE].y + D_800BC440[CORE].j[1].y);
        z = uz + (D_800BC428[CORE].z + D_800BC440[CORE].j[1].z);
        break;
    case 9:
        tbl = 6;
        slot = 6;
        x = D_800BC428[CORE].x + D_800BC440[CORE].j[2].x;
        y = D_800BC428[CORE].y + D_800BC440[CORE].j[2].y;
        z = D_800BC428[CORE].z + D_800BC440[CORE].j[2].z;
        v = D_80031A58[PW(parent)->slot];
        if (v != 255 && v >= 21 && (u8)(D_80031A5E - 17) < 14) {
            o->rot.vx = 512;
            o->rot.vy = 0;
            o->rot.vz = 0;
        }
        break;
    case 10:
        tbl = 6;
        slot = 7;
        x = D_800BC428[CORE].x + D_800BC440[CORE].j[2].x;
        y = D_800BC428[CORE].y + D_800BC440[CORE].j[2].y;
        z = D_800BC428[CORE].z + D_800BC440[CORE].j[2].z;
        v = D_80031A58[PW(parent)->slot];
        if (v != 255 && v >= 21 && (u8)(D_80031A5F - 17) < 14) {
            o->rot.vx = 512;
            o->rot.vy = 0;
            o->rot.vz = 0;
        }
        break;
    case 14:
        tbl = 5;
        slot = 1;
        x = D_800BC428[CORE].x + D_800BC440[CORE].j[4].x;
        y = D_800BC428[CORE].y + D_800BC440[CORE].j[4].y;
        z = D_800BC428[CORE].z + D_800BC440[CORE].j[4].z;
        o->rot.vx = 512;
        o->rot.vy = 0;
        o->rot.vz = 0;
        break;
    case 11:
        tbl = 10;
        slot = 8;
        break;
    case 12:
        tbl = 8;
        slot = 9;
        break;
    case 13:
        tbl = 9;
        slot = 10;
        break;
    }
    if (slot == 0 && D_80031A58[0] == 255) {
        part = 0;
    } else if (slot == 11 && D_80031A63 == 255) {
        part = 0;
    } else if (slot == 3 && D_80031A5B == 255) {
        part = 0;
    } else {
        part = D_80031A58[slot];
    }
    x += D_800BCAC4[tbl].ofs[part].x;
    y += D_800BCAC4[tbl].ofs[part].y;
    z += D_800BCAC4[tbl].ofs[part].z;
    if (tbl == 6 && D_80031A58[slot] >= 31 && D_80031A58[slot] != 255) {
        x *= -1;
    } else if (var & 1) {
        x *= -1;
    }
    if (obj_find_by_handle(func_800804F0) != &g_ui_none) {
        if (func_800517C8()) {
            o->render_type = 2;
            o->light = 11;
            o->view = 2;
        }
    }
    w->slot = slot;
    if (D_80031A58[slot] == 255) {
        o->visible = 0;
    }
    o->pos.vx = x;
    o->pos.vy = y;
    o->pos.vz = z;
    return o;
}
