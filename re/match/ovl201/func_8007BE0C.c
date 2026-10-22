#include "ac1/ac1.h"

typedef struct {
    u16 x;              /* +0x00 */
    s16 y;              /* +0x02 */
    s16 z;              /* +0x04 */
    u8 pad06[6];
    s32 scale;          /* +0x0C */
} PartPose;

typedef struct {
    u32 *mask;          /* +0x00 owned parts, 64 bits */
    PartPose *pose;     /* +0x04 per-part model placement */
    u8 pad[8];
} PartCat;

extern PartCat D_800BCABC[];

/* part-list work area of the caller (UiObj +0x7C) */
typedef struct {
    u8 pad7c[0x14];
    u16 cat;            /* +0x90 part category */
    u16 kind;           /* +0x92 */
    u8 pad94[0x12];
    u16 cur;            /* +0xA6 selected part, 255 = none */
} PartWork;

/* the model object built here (UiObj +0x7C) */
typedef struct {
    u16 h7c;            /* +0x7C */
    u16 h7e;            /* +0x7E */
    u16 cat;            /* +0x80 */
    u8 pad82[2];
    UiObj *list;        /* +0x84 the part list that owns it */
    u16 cur;            /* +0x88 */
} PartModelWork;

extern void func_8007C15C(UiObj *o);

UiObj *func_8007BE0C(s16 model, UiObj *p, u8 side)
{
    UiObj *o;
    PartWork *pw;
    PartModelWork *ow;
    s32 x;

    pw = (PartWork *)&p->w;
    o = ui_obj_alloc(4);
    o->ctor = func_8007BE0C;
    o->update = func_8007C15C;
    o->destroy = ui_obj_nop;
    o->render_type = 0;
    ((u16 *)&o->res)[0] = 0;
    if (pw->cur != 255) {
        if (pw->kind == 1) {
            ((u16 *)&o->res)[0] = 1;
        } else if (pw->kind == 7 && pw->cur < 31) {
            ((u16 *)&o->res)[0] = 1;
        }
    }
    ((u16 *)&o->res)[1] = model;
    o->light = 18;
    o->scaled = 1;
    o->rot.vx = 0;
    o->rot.vy = 0;
    o->rot.vz = 0;
    ow = (PartModelWork *)&o->w;
    ow->cat = pw->cat;
    ow->list = p;
    ow->cur = pw->cur;
    if (pw->cur == 255) {
        switch (side) {
        case 'C':
            ow->h7e = 0;
            x = 0;
            break;
        case 'L':
            x = -(s16)D_800BCABC[0].pose->x;
            ow->h7e = ow->h7c;
            break;
        case 'R':
            x = D_800BCABC[0].pose->x;
            ow->h7e = ow->h7c;
            break;
        }
        o->scale.vx = 0xE00;
        o->scale.vy = 0xE00;
        o->scale.vz = 0xE00;
        o->pos.vx = (s16)x;
        o->pos.vy = -130;
        o->pos.vz = D_800BCABC[0].pose->z;
    } else {
        switch (side) {
        case 'C':
            ow->h7e = 0;
            x = 0;
            break;
        case 'L':
            x = -(s16)D_800BCABC[ow->cat].pose[pw->cur].x;
            ow->h7e = ow->h7c;
            break;
        case 'R':
            x = D_800BCABC[ow->cat].pose[pw->cur].x;
            ow->h7e = ow->h7c;
            break;
        }
        o->scale.vx = D_800BCABC[ow->cat].pose[pw->cur].scale;
        o->scale.vy = D_800BCABC[ow->cat].pose[pw->cur].scale;
        o->scale.vz = D_800BCABC[ow->cat].pose[pw->cur].scale;
        o->pos.vx = (s16)x;
        o->pos.vy = D_800BCABC[ow->cat].pose[pw->cur].y;
        o->pos.vz = D_800BCABC[ow->cat].pose[pw->cur].z;
    }
    return o;
}
