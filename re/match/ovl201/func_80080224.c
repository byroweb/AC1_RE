#include "ac1_types.h"
/* ovl201 0x80080224  size 696: spawn a task object and place it from one of
 * the position tables selected by kind (cases 3..6; no default, like retail). */
typedef struct {
    s16 vx, vy, vz, pad;
} SVECTOR;
typedef struct {
    SVECTOR a;          /* +0x00 */
    SVECTOR b;          /* +0x08 */
    u8 pad[0x28 - 0x10];
} Func80080224Ofs;
typedef struct {
    SVECTOR *tbl;       /* +0x0 */
    u8 pad[0xC];
} Func80080224Set;
typedef struct {
    u8 pad0[0xA];
    s16 f0A;            /* +0x0A */
    s16 flag;           /* +0x0C */
    s16 kind;           /* +0x0E */
    u8 pad10[0x16 - 0x10];
    s16 f16;            /* +0x16 */
    u8 pad18[0x40 - 0x18];
    s16 f40;            /* +0x40 */
    s16 f42;            /* +0x42 */
    s16 f44;            /* +0x44 */
    s16 pad46;
    s32 x;              /* +0x48 */
    s32 y;              /* +0x4C */
    s32 z;              /* +0x50 */
    u8 pad54[0x6C - 0x54];
    s32 arg;            /* +0x6C */
    void (*f70)();      /* +0x70 */
    void (*f74)();      /* +0x74 */
    void (*f78)();      /* +0x78 */
} Func80080224Obj;

extern u8 D_80031A58[];
extern SVECTOR D_800BC428[];
extern Func80080224Ofs D_800BC440[];
extern Func80080224Set D_800BCAC4[];
extern Func80080224Obj *func_800501BC(int);
extern void func_800800A4();
extern void func_800804DC();
extern void func_800507F4();
extern void func_800507FC();

Func80080224Obj *func_80080224(u16 kind, s32 arg, s16 flag)
{
    Func80080224Obj *o;
    s16 x;
    s16 z;
    s16 y;
    s16 k;
    s16 m;
    int i;

    o = func_800501BC(4);
    o->f70 = func_800800A4;
    o->f74 = func_800804DC;
    o->f78 = func_800507F4;
    o->kind = kind;
    o->f0A = 0;
    o->flag = flag;
    o->f16 = 18;
    o->f40 = 0;
    o->f42 = 0;
    o->f44 = 0;
    if (kind != 6) {
        o->arg = arg;
        o->f74 = func_800507FC;
    }
    switch (kind) {
    case 3:
        i = D_80031A58[0];
        m = 2;
        x = D_800BC428[i].vx + D_800BC440[i].a.vx;
        y = D_800BC428[i].vy + D_800BC440[i].a.vy;
        z = D_800BC428[i].vz + D_800BC440[i].a.vz;
        k = 4;
        break;
    case 4:
        k = m = 0;
        x = y = z = 0;
        break;
    case 5:
        i = D_80031A58[0];
        m = 3;
        k = 2;
        x = D_800BC428[i].vx + D_800BC440[i].b.vx;
        y = D_800BC428[i].vy + D_800BC440[i].b.vy;
        z = D_800BC428[i].vz + D_800BC440[i].b.vz;
        break;
    case 6:
        m = 11;
        k = 1;
        x = 0;
        y = 180;
        z = 1000;
        break;
    }
    x += D_800BCAC4[k].tbl[D_80031A58[m]].vx;
    y += D_800BCAC4[k].tbl[D_80031A58[m]].vy;
    z += D_800BCAC4[k].tbl[D_80031A58[m]].vz;
    if (flag & 1) {
        x *= -1;
    }
    o->x = x;
    o->y = y;
    o->z = z;
    return o;
}
