#include "ac1/ac1.h"
/* ovl201 0x80064EB0 - two mirrored gouraud quads (a bar of length
 * o->+0x7C / 4 to either side of the object's origin), lit with NCCS in
 * two alternating colours, at the object's depth. */

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
    s32 xy0;
    u8 r1, g1, b1, p1;
    s32 xy1;
    u8 r2, g2, b2, p2;
    s32 xy2;
    u8 r3, g3, b3, p3;
    s32 xy3;
} PolyG4;

typedef struct {
    u8 idx;
    u8 pad[3];
    u8 *prim;
} UiGfx;

extern UiGfx D_801EF6C8;
extern SVECTOR D_800B8D78;
extern u8 D_800B75B0[4];
extern u8 D_800B75B4[4];
extern void SetRotMatrix(MATRIX *m);
extern void SetTransMatrix(MATRIX *m);

void func_80064EB0(UiObj *o)
{
    PolyG4 *p[2];
    SVECTOR v[4];
    s32 xy[4];
    s16 len;
    s16 depth;

    len = *(s16 *)&o->w.raw[0] >> 2;
    depth = o->unk14;
    v[0].vx = 10;
    v[0].vy = 50;
    v[2].vx = 0;
    v[2].vy = 55;
    v[1].vy = 50;
    v[3].vy = 55;
    v[0].vz = v[1].vz = v[2].vz = v[3].vz = 256;
    p[0] = (PolyG4 *)D_801EF6C8.prim;
    D_801EF6C8.prim += sizeof(PolyG4);
    p[1] = (PolyG4 *)D_801EF6C8.prim;
    D_801EF6C8.prim += sizeof(PolyG4);
    v[1].vx = len + 10;
    v[3].vx = len;
    o->mat.t[2] = 0;
    SetRotMatrix(&o->mat);
    SetTransMatrix(&o->mat);

    gte_ldv3(&v[0], &v[1], &v[2]);
    gte_rtpt();
    gte_stsxy3(&xy[0], &xy[1], &xy[2]);
    gte_ldv0(&v[3]);
    gte_rtps();
    gte_stsxy(&xy[3]);
    setlen(p[0], 8);
    setcode(p[0], 0x38);
    p[0]->xy0 = xy[0];
    p[0]->xy1 = xy[1];
    p[0]->xy2 = xy[2];
    p[0]->xy3 = xy[3];
    gte_ldv0(&D_800B8D78);
    gte_ldrgb(D_800B75B0);
    gte_nccs();
    gte_strgb(&p[0]->r0);
    gte_ldv0(&D_800B8D78);
    gte_ldrgb(D_800B75B4);
    gte_nccs();
    gte_strgb(&p[0]->r1);
    gte_ldv0(&D_800B8D78);
    gte_ldrgb(D_800B75B0);
    gte_nccs();
    gte_strgb(&p[0]->r2);
    gte_ldv0(&D_800B8D78);
    gte_ldrgb(D_800B75B4);
    gte_nccs();
    gte_strgb(&p[0]->r3);
    addPrim(o->ot[D_801EF6C8.idx] + depth, p[0]);

    v[0].vx = -10;
    v[0].vy = 50;
    v[2].vx = 0;
    v[2].vy = 55;
    v[1].vx = -10 - len;
    v[1].vy = 50;
    v[3].vx = -len;
    v[3].vy = 55;
    v[0].vz = v[1].vz = v[2].vz = v[3].vz = 256;
    gte_ldv3(&v[0], &v[1], &v[2]);
    gte_rtpt();
    gte_stsxy3(&xy[0], &xy[1], &xy[2]);
    gte_ldv0(&v[3]);
    gte_rtps();
    gte_stsxy(&xy[3]);
    setlen(p[1], 8);
    setcode(p[1], 0x38);
    p[1]->xy0 = xy[0];
    p[1]->xy1 = xy[1];
    p[1]->xy2 = xy[2];
    p[1]->xy3 = xy[3];
    gte_ldv0(&D_800B8D78);
    gte_ldrgb(D_800B75B0);
    gte_nccs();
    gte_strgb(&p[1]->r0);
    gte_ldv0(&D_800B8D78);
    gte_ldrgb(D_800B75B4);
    gte_nccs();
    gte_strgb(&p[1]->r1);
    gte_ldv0(&D_800B8D78);
    gte_ldrgb(D_800B75B0);
    gte_nccs();
    gte_strgb(&p[1]->r2);
    gte_ldv0(&D_800B8D78);
    gte_ldrgb(D_800B75B4);
    gte_nccs();
    gte_strgb(&p[1]->r3);
    addPrim(o->ot[D_801EF6C8.idx] + depth, p[1]);
}
