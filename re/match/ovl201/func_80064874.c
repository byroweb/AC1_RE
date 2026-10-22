#include "ac1/ac1.h"
/* ovl201 0x80064874 - outline the view rectangle of the object's id
 * (w x h centred, z 256) in the object's frame: a 4-point polyline
 * 0-1-3-2 plus the closing line 0-2, both lit with NCCS. */

#define gte_ldv0(r0) __asm__ volatile ("lwc2 $0, 0( %0 );lwc2 $1, 4( %0 )" : : "r"(r0))
#define gte_ldv3(r0, r1, r2) __asm__ volatile ("lwc2 $0, 0( %0 );lwc2 $1, 4( %0 );lwc2 $2, 0( %1 );lwc2 $3, 4( %1 );lwc2 $4, 0( %2 );lwc2 $5, 4( %2 )" : : "r"(r0), "r"(r1), "r"(r2))
#define gte_ldrgb(r0) __asm__ volatile ("lwc2 $6, 0( %0 )" : : "r"(r0))
#define gte_rtps() __asm__ volatile ("nop;nop;.word 0x4A180001")
#define gte_rtpt() __asm__ volatile ("nop;nop;.word 0x4A280030")
#define gte_nccs() __asm__ volatile ("nop;nop;.word 0x4B08041B")
#define gte_stsxy(r0) __asm__ volatile ("swc2 $14, 0( %0 )" : : "r"(r0) : "memory")
#define gte_stsxy3(r0, r1, r2) __asm__ volatile ("swc2 $12, 0( %0 );swc2 $13, 0( %1 );swc2 $14, 0( %2 )" : : "r"(r0), "r"(r1), "r"(r2) : "memory")
#define gte_strgb(r0) __asm__ volatile ("swc2 $22, 0( %0 )" : : "r"(r0) : "memory")

typedef struct {
    u32 tag;
    u8 r0, g0, b0, code;
    s32 xy0, xy1;
} LineF2;

typedef struct {
    u32 tag;
    u8 r0, g0, b0, code;
    s32 xy0, xy1, xy2, xy3;
    u32 pad;
} LineF4;

typedef struct {
    LineF2 l2;
    LineF4 l4;
} FramePrim;

typedef struct {
    u8 idx;
    u8 pad[3];
    u8 *prim;
} UiGfx;

typedef struct {                /* 100 bytes: one 2D view */
    s16 x, y;                   /* +0x00 screen origin */
    s16 w, h;                   /* +0x04 size */
    s16 ox, oy;                 /* +0x08 world point at the origin */
    u8 pad0C[0x58];
} UiView2D;

typedef struct {
    UiView2D view[1035];
    u8 pad[12];
} UiViewSet;

extern UiGfx D_801EF6C8;
extern UiViewSet D_801BCE88[2];
extern SVECTOR D_800B8D78;
extern u8 D_800B7598[4];
extern u8 D_800B759C[4];
extern void SetRotMatrix(MATRIX *m);
extern void SetTransMatrix(MATRIX *m);

void func_80064874(UiObj *o)
{
    FramePrim *p;
    LineF4 *q;
    s16 depth;
    s16 k;
    s16 w;
    s16 h;
    SVECTOR v[4];
    s32 xy[4];
    s32 a, b;

    p = (FramePrim *)D_801EF6C8.prim;
    depth = o->unk14;
    D_801EF6C8.prim += sizeof(FramePrim);
    q = &p->l4;
    switch (o->id) {
    case 1:
        k = 0;
        break;
    case 3:
        k = 1;
        break;
    case 5:
        k = 2;
        break;
    case 7:
        k = 3;
        break;
    case 9:
        k = 4;
        break;
    case 11:
        k = 5;
        break;
    case 13:
        k = 6;
        break;
    case 15:
        k = 7;
        break;
    case 17:
        k = 8;
        break;
    case 19:
        k = 9;
        break;
    }
    w = D_801BCE88[0].view[k].w - 1;
    h = D_801BCE88[0].view[k].h - 1;
    v[0].vz = v[1].vz = v[2].vz = v[3].vz = 256;
    v[0].vx = v[2].vx = -w / 2;
    v[0].vy = v[1].vy = -h / 2;
    v[1].vx = v[3].vx = w / 2;
    v[2].vy = v[3].vy = h / 2;
    o->mat.t[2] = 0;
    SetRotMatrix(&o->mat);
    SetTransMatrix(&o->mat);
    gte_ldv3(&v[0], &v[1], &v[2]);
    gte_rtpt();
    gte_stsxy3(&xy[0], &xy[1], &xy[2]);
    gte_ldv0(&v[3]);
    gte_rtps();
    gte_stsxy(&xy[3]);
    setlen(&p->l2, 3);
    setcode(&p->l2, 0x40);
    setlen(q, 6);
    setcode(q, 0x4C);
    q->xy0 = a = xy[0];
    q->xy1 = xy[1];
    q->xy2 = xy[3];
    q->xy3 = b = xy[2];
    q->pad = 0x55555555;
    p->l2.xy0 = a;
    p->l2.xy1 = b;
    gte_ldv0(&D_800B8D78);
    gte_ldrgb(D_800B7598);
    gte_nccs();
    gte_strgb(&p->l2.r0);
    gte_ldv0(&D_800B8D78);
    gte_ldrgb(D_800B759C);
    gte_nccs();
    gte_strgb(&q->r0);
    addPrim(o->ot[D_801EF6C8.idx] + depth, q);
    addPrim(o->ot[D_801EF6C8.idx] + depth, p);
}
