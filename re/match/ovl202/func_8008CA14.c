#include "ac1/ac1.h"
#include "ac1/ovl202_t42.h"

extern SVECTOR D_80090824[3];
extern SVECTOR D_8009083C[1];
extern u32 D_80090844[3];
extern void func_80013AFC(s16 ang, MATRIX *m);
extern void func_8008C6FC(SVECTOR *v);
extern void func_8008C7BC(SVECTOR *in, ScrPt *out, s32 n);
extern void func_8008C890(ScrPt *r, s32 a1, s32 idx, s32 a3);
extern void func_8008C95C(ScrPt *xy, u32 *rgb, s32 z);

void func_8008CA14(SVECTOR *pos, s32 ang, s32 label) {
    T42Spad *sp = SPAD;

    func_8008C6FC(pos);
    RotTrans(&SPAD->v, &SPAD->out, &SPAD->flag);
    func_80013AFC(-ang, &SPAD->rot);
    MulRotMatrix(&SPAD->rot);
    SetRotMatrix(&SPAD->rot);
    SetTransMatrix((MATRIX *)((u8 *)&SPAD->out - 20));
    func_8008C7BC(D_80090824, SPAD->pts, 3);
    if ((s16)(SPAD->pts[2].clip | (SPAD->pts[0].clip | SPAD->pts[1].clip)) != 255
        && !(SPAD->pts[2].clip & (SPAD->pts[0].clip & SPAD->pts[1].clip))) {
        func_8008C95C(SPAD->pts, D_80090844, 16);
    }
    if (label != 0) {
        func_8008C7BC(D_8009083C, sp->pts, 1);
        if (sp->pts[0].clip != 255) {
            func_8008C890(sp->pts, label, 7, 8);
        }
    }
}
