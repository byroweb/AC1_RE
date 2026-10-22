#include "ac1/ac1.h"

typedef struct {
    union {
        u32 w0;         /* +0x00 packed fields */
        struct {
            u16 h0;
            u16 h2;     /* +0x02 */
        } h;
    } u;
    u8 pad4[4];         /* +0x04 */
    s16 f8;
    s16 fa;
    s16 fc;
    s16 fe;
    s16 f10;
    u8 f12;
    s8 f13;
    s16 f14;
    s16 f16;
    s16 f18;
    s16 f1a;
    u8 f1c;
    s8 f1d;
    s16 f1e;
    s16 f20;
    s16 f22;
} Rec;
typedef struct {
    u8 pad0[8];
    u16 x;              /* +0x08 */
    u16 y;              /* +0x0A */
    u16 z;              /* +0x0C */
    u8 pad0e[0x1C];
    u16 h2a;            /* +0x2A */
    u8 pad2c[2];
    s16 h2e;            /* +0x2E */
    u8 pad30[0x10];
    u8 cnt;             /* +0x40 */
    u8 pad41[0x29];
    u16 r;              /* +0x6A */
    u16 hgt;            /* +0x6C */
    u8 pad6e[0x102];
} Obj;

extern s32 func_80014D78(s32);
extern void func_8004CC30(s32, s32, AcPos *, s32, s32, s32, s32);
extern void func_80086854(s32, AcPos *, void *);
extern void func_800869F0(s32, AcPos *, s32, s32, void *);
extern void func_80086AD8(s32, AcPos *, s32, s32, s32);
extern void func_80086DDC(s32, AcPos *, s32, s32, s32, s32, s32, s32, s32, s32);
extern void func_800873F8(Obj *, AcPos *, s32, s32, s32, s32);
extern void func_801C64E8(Obj *, AcPos *, s32, s32, s32, s32);

s32 func_801C6880(Obj *o, Rec *rec)
{
    AcPos a;
    AcPos b;

    if (o->cnt == 0) {
        a.x = o->x;
        a.y = o->y - (o->hgt >> 1);
        a.z = o->z;
        if (rec->fc != 0) {
            func_80086AD8(0, &a, rec->f8, rec->fa, rec->fc);
        }
        func_800869F0(0, &a, o->r * 2, rec->fe, rec);
        func_801C64E8(o, &a, rec->f14, rec->f16, rec->f18, 0);
        func_8004CC30(rec->f10, 0, &a, rec->f12, 20000, 18000, rec->f13);
        if ((s16)o->y >= o->h2e && o->h2e >= (s16)o->y - o->h2a) {
            a.y = o->h2e;
            func_80086DDC(0, &a, o->r, rec->fe, 40, (rec->u.w0 >> 17) & 0x3FF, (rec->u.w0 >> 15) & 0xFFC,
                          (rec->u.w0 << 5) >> 23, rec->u.h.h2 & 0x7FE, 64);
        }
        return 1;
    }
    if (!(o->cnt & 1)) {
        b.x = o->x + func_80014D78(o->r * 2) - o->r;
        b.z = o->z + func_80014D78(o->r * 2) - o->r;
        b.y = o->y - func_80014D78(o->hgt);
        func_80086854(0, &b, rec->pad4);
        func_800873F8(o, &b, rec->f1e, rec->f20, rec->f22, 1);
        if (!(o->cnt & 3)) {
            func_8004CC30(rec->f1a, 0, &b, rec->f1c, 16000, 18000, rec->f1d);
        }
    }
    o->cnt--;
    return 0;
}
