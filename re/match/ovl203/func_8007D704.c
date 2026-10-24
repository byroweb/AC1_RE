#include "ac1/ac1.h"
/* ovl203: (re)initialise the AC of the side in D_80039D18 from its assembled
 * part stats, then spawn its body o. */
typedef struct {
    u8 pad0[0x0E];
    s16 h0E;            /* +0x0E */
    u8 pad10[0x36 - 0x10];
    u16 h36;            /* +0x36 */
    u16 h38;            /* +0x38 */
    u8 b3A;             /* +0x3A */
    u8 pad3b[0x60 - 0x3B];
    u8 mover[4];        /* +0x60 */
    u8 b64;             /* +0x64 */
    u8 pad65[3];
    u8 b68;             /* +0x68 */
    u8 pad69[0x6E - 0x69];
    s16 h6E;            /* +0x6E */
    u8 pad70[0x76 - 0x70];
    u8 s76[0x110 - 0x76]; /* +0x76 */
    s32 w110;           /* +0x110 */
    u8 pad114[0x160 - 0x114];
    u16 h160;           /* +0x160 */
    u16 h162;           /* +0x162 */
    u16 h164;           /* +0x164 */
} IObj3;
typedef struct {
    u8 pad0[0x0E];
    u16 a;              /* +0x0E max */
    u16 b;              /* +0x10 ammo */
    u8 pad12[0x3C - 0x12];
} Ent3C;
typedef struct {
    u8 pad0[0x18];
    u8 b18;             /* +0x18 */
    u8 b19;             /* +0x19 */
    u8 pad1a[4];
    u16 h1E;            /* +0x1E */
    u16 h20;            /* +0x20 */
    u8 pad22[0x4C - 0x22];
    u8 *p4C;            /* +0x4C */
    u8 pad50[0x204 - 0x50];
    s32 w204;           /* +0x204 */
    u16 h208;           /* +0x208 */
    u16 h20A;           /* +0x20A */
    u16 h20C;           /* +0x20C */
    u16 h20E;           /* +0x20E */
    u16 h210;           /* +0x210 */
    u16 h212;           /* +0x212 */
    u16 h214;           /* +0x214 */
    u16 h216;           /* +0x216 */
    u8 pad218[0x21E - 0x218];
    u16 h21E;           /* +0x21E */
    u8 pad220[0x257 - 0x220];
    u8 b257;            /* +0x257 */
    u8 pad258;
    u8 b259;            /* +0x259 */
    u8 b25A;            /* +0x25A */
    u8 b25B;            /* +0x25B */
    u8 b25C;            /* +0x25C */
    u8 b25D;            /* +0x25D */
    u8 b25E;            /* +0x25E */
    u8 b25F;            /* +0x25F */
    u8 b260;            /* +0x260 */
    u8 b261;            /* +0x261 */
    u8 pad262[2];
    u16 h264;           /* +0x264 */
    u16 h266;           /* +0x266 */
    u16 h268;           /* +0x268 */
    u8 pad26a[2];
    u16 h26C;           /* +0x26C */
    u8 pad26e[2];
    u16 h270;           /* +0x270 */
    u16 h272;           /* +0x272 */
    u8 pad274[2];
    u8 b276;            /* +0x276 */
    u8 pad277[3];
    u8 b27A;            /* +0x27A */
    u8 b27B;            /* +0x27B */
    u8 pad27c[0x286 - 0x27C];
    u16 h286;           /* +0x286 */
    u8 pad288[0x2D8 - 0x288];
    u8 b2D8;            /* +0x2D8 */
    u8 pad2d9;
    Ent3C ent[3];       /* +0x2DA */
} Side3;
extern Side3 *D_80039D18;
extern u8 D_8008A374[][20];
extern void func_8007CB08();
extern void func_800781F0(void);
extern void func_8007A590(void);
extern void func_8007F958(IObj3 *o);
extern void func_8008182C(IObj3 *o);
extern void func_8005414C(void *m, s32 a, s32 b, s32 c, s32 d, s32 e, s32 f, void *g);

void func_8007D704(IObj3 *o)
{
    Side3 *p = D_80039D18;
    Ent3C *e;
    s32 i;

    p->b19 = 0;
    p->b18 = 0;
    func_8007CB08();
    func_800781F0();
    D_80039D18->h1E = D_80039D18->h264;
    D_80039D18->h20 = 0;
    D_80039D18->h208 = D_80039D18->b259 * 2;
    D_80039D18->h20A = D_80039D18->b25A * 200;
    D_80039D18->h20C = D_80039D18->b25B * 14;
    D_80039D18->h20E = D_80039D18->b25C * 140;
    D_80039D18->h210 = D_80039D18->b25D * 14;
    D_80039D18->h212 = D_80039D18->b25E * 140;
    D_80039D18->h214 = (u32)(D_80039D18->b25F * 5) >> 4;
    D_80039D18->h216 = (D_80039D18->b260 * 15) >> 4;
    D_80039D18->w204 = D_80039D18->h270 * D_80039D18->h270;
    D_80039D18->h21E = D_80039D18->b261 * 4;
    D_80039D18->p4C = D_8008A374[D_80039D18->b27B];
    if (D_80039D18->b257 >= 6) {
        D_80039D18->h266 >>= 1;
        D_80039D18->h268 <<= 1;
        D_80039D18->h26C <<= 1;
        if (D_80039D18->h266 < D_80039D18->h268) {
            D_80039D18->h268 = D_80039D18->h266;
        }
        if (D_80039D18->h266 < D_80039D18->h26C) {
            D_80039D18->h26C = D_80039D18->h266;
        }
    }
    i = D_80039D18->b2D8;
    if (--i != -1) {
        u16 *q = &D_80039D18->ent[0].b;
        s32 j = 0;
        do {
            q[j * 30] = q[j * 30 - 1];
            j++;
        } while (--i != -1);
    }
    D_80039D18->b18 = 0;
    func_8007A590();
    o->b3A = 7;
    o->h36 = 1;
    o->h160 = o->h162 = D_80039D18->h272;
    o->h164 = (u32)(D_80039D18->b276 * 25) >> 2;
    o->h38 = D_80039D18->h286 & ~3;
    D_80039D18->h286 &= ~8;
    o->b68 = 2;
    o->w110 = 0;
    switch (D_80039D18->b27A) {
    case 0:
    case 3:
        if (D_80039D18->b257 >= 4) {
            D_80039D18->b27A = 3;
        }
        func_8007F958(o);
        break;
    case 1:
    case 2:
        func_8008182C(o);
        break;
    }
    func_8005414C(o->mover, o->h0E + 128, o->b64, o->h6E, 0, 255, 0, o->s76);
}
