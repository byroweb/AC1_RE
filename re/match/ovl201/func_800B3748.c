#include "ac1/ac1.h"

typedef struct {
    u16 x, y, z, pad;
} PartPt;

typedef struct {
    PartPt pt[5];               /* attach points of a core: 0 head, 1 arms, 2 weapons, 4 back */
} CorePts;

typedef struct {
    PartPt *pts;                /* per-part offsets of one model kind */
    u8 pad04[12];
} PartKind;

typedef struct {
    u16 slot;                   /* +0x7C equipped-part slot this object shows */
    u8 pad7e[2];
    UiObj *root;                /* +0x80 */
} PartWork;

typedef struct {
    u8 pad00[0xC];
    u16 flag;                   /* +0x0C bit 0: mirror */
    u16 kind;                   /* +0x0E */
} PartHdr;

extern u8 D_80031A58[12];       /* equipped part per slot, 0xFF = none */
extern PartPt D_800BC428[];     /* per core */
extern CorePts D_800BC440[];    /* per core */
extern PartPt D_800BC638[];     /* per arms */
extern PartPt D_800BC6B8[];     /* per arms */
extern PartPt D_800BC968[];     /* per legs */
extern PartKind D_800BCAC4[];
extern UiObj *func_800500EC(void *ctor, UiObj *from);
extern void func_800B3E7C(UiObj *o);

#define SLOT(o) (((PartWork *)&(o)->w)->slot)

/* AC preview model part: places a part object at its attach point on the core. */
UiObj *func_800B3748(u16 kind, UiObj *parent, UiObj *unused, u16 flag, s16 n)
{
    UiObj *o;
    PartWork *w;
    UiObj *p;
    s16 x, y, z;
    s32 px, py, pz;
    u16 slot;
    u16 tbl;
    u8 t;
    u8 id;

    o = ui_obj_alloc(n);
    o->ctor = func_800B3748;
    o->update = func_800B3E7C;
    o->destroy = ui_obj_nop;
    o->light = 18;
    o->render_type = 0;
    ((PartHdr *)o)->flag = flag;
    ((PartHdr *)o)->kind = kind;
    o->rot.vx = 0;
    o->rot.vy = 0;
    o->rot.vz = 0;
    if (parent != &g_ui_none) {
        o->parent = parent;
    }
    w = (PartWork *)&o->w;
    if (kind == 6) {
        w->root = o;
    } else {
        w->root = parent;
    }
    z = 0;
    y = 0;
    x = 0;
    switch (kind) {
    case 3:
        tbl = 4;
        x = D_800BC428[D_80031A58[0]].x + D_800BC440[D_80031A58[0]].pt[0].x;
        y = D_800BC428[D_80031A58[0]].y + D_800BC440[D_80031A58[0]].pt[0].y;
        z = D_800BC428[D_80031A58[0]].z + D_800BC440[D_80031A58[0]].pt[0].z;
        slot = 2;
        break;
    case 4:
        tbl = 0;
        slot = 0;
        break;
    case 5:
        tbl = 2;
        x = D_800BC428[D_80031A58[0]].x + D_800BC440[D_80031A58[0]].pt[1].x;
        slot = 3;
        y = D_800BC428[D_80031A58[0]].y + D_800BC440[D_80031A58[0]].pt[1].y;
        z = D_800BC428[D_80031A58[0]].z + D_800BC440[D_80031A58[0]].pt[1].z;
        break;
    case 6:
        tbl = 1;
        slot = 11;
        break;
    case 8:
        tbl = 7;
        slot = 5;
        p = obj_find_by_handle(func_800B3748);
        while (p != &g_ui_none && SLOT(p) != 3) {
            p = func_800500EC(func_800B3748, p);
        }
        t = D_80031A58[SLOT(parent)];
        if (t < 21 || t == 0xFF) {
            px = D_800BC638[D_80031A58[SLOT(p)]].x;
            py = D_800BC638[D_80031A58[SLOT(p)]].y;
            pz = D_800BC638[D_80031A58[SLOT(p)]].z;
            o->rot.vx = 0x200;
            o->rot.vy = 0;
            o->rot.vz = 0;
        } else {
            px = D_800BC6B8[D_80031A58[SLOT(p)]].x + D_800BC968[D_80031A58[slot]].x;
            py = D_800BC6B8[D_80031A58[SLOT(p)]].y + D_800BC968[D_80031A58[slot]].y;
            pz = D_800BC6B8[D_80031A58[SLOT(p)]].z + D_800BC968[D_80031A58[slot]].z;
            o->rot.vx = 0;
            o->rot.vy = 0;
            o->rot.vz = 0;
        }
        x = -(px + (D_800BC428[D_80031A58[0]].x + D_800BC440[D_80031A58[0]].pt[1].x));
        y = py + (D_800BC428[D_80031A58[0]].y + D_800BC440[D_80031A58[0]].pt[1].y);
        z = pz + (D_800BC428[D_80031A58[0]].z + D_800BC440[D_80031A58[0]].pt[1].z);
        break;
    case 9:
        tbl = 6;
        x = D_800BC428[D_80031A58[0]].x + D_800BC440[D_80031A58[0]].pt[2].x;
        y = D_800BC428[D_80031A58[0]].y + D_800BC440[D_80031A58[0]].pt[2].y;
        z = D_800BC428[D_80031A58[0]].z + D_800BC440[D_80031A58[0]].pt[2].z;
        t = D_80031A58[SLOT(parent)];
        slot = 6;
        if (t != 0xFF && t >= 21 && D_80031A58[6] >= 17 && D_80031A58[6] < 31) {
            o->rot.vx = 0x200;
            o->rot.vy = 0;
            o->rot.vz = 0;
        }
        break;
    case 10:
        tbl = 6;
        x = D_800BC428[D_80031A58[0]].x + D_800BC440[D_80031A58[0]].pt[2].x;
        y = D_800BC428[D_80031A58[0]].y + D_800BC440[D_80031A58[0]].pt[2].y;
        z = D_800BC428[D_80031A58[0]].z + D_800BC440[D_80031A58[0]].pt[2].z;
        t = D_80031A58[SLOT(parent)];
        slot = 7;
        if (t != 0xFF && t >= 21 && D_80031A58[7] >= 17 && D_80031A58[7] < 31) {
            o->rot.vx = 0x200;
            o->rot.vy = 0;
            o->rot.vz = 0;
        }
        break;
    case 14:
        tbl = 5;
        slot = 1;
        x = D_800BC428[D_80031A58[0]].x + D_800BC440[D_80031A58[0]].pt[4].x;
        y = D_800BC428[D_80031A58[0]].y + D_800BC440[D_80031A58[0]].pt[4].y;
        z = D_800BC428[D_80031A58[0]].z + D_800BC440[D_80031A58[0]].pt[4].z;
        o->rot.vx = 0x200;
        o->rot.vy = 0;
        o->rot.vz = 0;
        break;
    }
    if (slot == 0 && D_80031A58[0] == 0xFF) {
        id = 0;
    } else if (slot == 11 && D_80031A58[11] == 0xFF) {
        id = 0;
    } else if (slot == 3 && D_80031A58[3] == 0xFF) {
        id = 0;
    } else {
        id = D_80031A58[slot];
    }
    x += D_800BCAC4[tbl].pts[id].x;
    y += D_800BCAC4[tbl].pts[id].y;
    z += D_800BCAC4[tbl].pts[id].z;
    if (tbl == 6 && D_80031A58[slot] >= 31 && D_80031A58[slot] != 0xFF) {
        x *= -1;
    } else if (flag & 1) {
        x *= -1;
    }
    w->slot = slot;
    if (D_80031A58[slot] == 0xFF) {
        o->visible = 0;
    }
    o->pos.vx = x;
    o->pos.vy = y;
    o->pos.vz = z;
    return o;
}
