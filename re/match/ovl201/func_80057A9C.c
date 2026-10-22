#include "ac1/ac1.h"

/* PsyQ inline_c.h GTE macros */
#define gte_ldv0(r0) __asm__ volatile ("lwc2 $0, 0(%0)\n\tlwc2 $1, 4(%0)" : : "r"(r0))
#define gte_ldrgb(r0) __asm__ volatile ("lwc2 $6, 0(%0)" : : "r"(r0))
#define gte_nccs() __asm__ volatile ("nop\n\tnop\n\t.word 0x4B08041B")
#define gte_strgb(r0) __asm__ volatile ("swc2 $22, 0(%0)" : : "r"(r0) : "memory")
#define gte_ldsxy3(r0, r1, r2) __asm__ volatile ("mtc2 %0, $12\n\tmtc2 %2, $14\n\tmtc2 %1, $13" : : "r"(r0), "r"(r1), "r"(r2))
#define gte_nclip() __asm__ volatile ("nop\n\tnop\n\t.word 0x4B400006")
#define gte_stopz(r0) __asm__ volatile ("swc2 $24, 0(%0)" : : "r"(r0) : "memory")
#define gte_ldsz3(r0, r1, r2) __asm__ volatile ("mtc2 %0, $17\n\tmtc2 %1, $18\n\tmtc2 %2, $19" : : "r"(r0), "r"(r1), "r"(r2))
#define gte_avsz3() __asm__ volatile ("nop\n\tnop\n\t.word 0x4B58002D")
#define gte_stotz(r0) __asm__ volatile ("swc2 $7, 0(%0)" : : "r"(r0) : "memory")

typedef struct {
    u8 r, g, b, code;
} Rgbc;

/* preprocessed TMD packet: flat textured triangle with one normal (0x18) */
typedef struct {
    u8 type;
    u8 pad1;
    s16 count;
    u16 uv0;    /* +0x04 */
    u16 clut;
    u16 uv1;    /* +0x08 */
    u16 tpage;
    u16 uv2;    /* +0x0C */
    u16 pad0E;
    s16 n0;     /* +0x10 normal index */
    s16 v0;     /* +0x12 byte offsets into the vertex tables */
    s16 v1;
    s16 v2;
} TmdPktFT3;

typedef struct {
    u32 tag;
    Rgbc c;
    s32 xy0;
    u16 uv0, clut;
    s32 xy1;
    u16 uv1, tpage;
    s32 xy2;
    u16 uv2, pad1;
} PolyFT3w;

typedef struct {
    u8 idx;         /* 0x801EF6C8 */
    u8 pad[3];
    u8 *prim;       /* 0x801EF6CC next free primitive */
} GfxBuf;

extern TmdPktFT3 *D_8017E500;
extern GfxBuf D_801EF6C8;
extern u8 D_8019F428[];    /* screen xy per vertex */
extern u8 D_801F07F0[];    /* sz per vertex */
extern u8 D_800B702C[4];   /* base colour */

#define SXY(off) (*(s32 *)(D_8019F428 + (off)))
#define SZ(off) (*(s32 *)(D_801F07F0 + (off)))

void func_80057A9C(u32 *ot, SVECTOR *normals)
{
    TmdPktFT3 *pk;
    PolyFT3w *p;
    s16 n;
    s32 opz;
    s32 otz;

    for (n = D_8017E500->count; n > 0; n--) {
        pk = D_8017E500;
        p = (PolyFT3w *)D_801EF6C8.prim;
        gte_ldsxy3(SXY(pk->v0), SXY(pk->v1), SXY(pk->v2));
        gte_nclip();
        gte_stopz(&opz);
        if (opz > 0) {
            p->xy0 = SXY(pk->v0);
            p->xy1 = SXY(pk->v1);
            p->xy2 = SXY(pk->v2);
            gte_ldv0(&normals[pk->n0]);
            gte_ldrgb(D_800B702C);
            gte_nccs();
            gte_strgb(&p->c);
            p->tag = 0x071FEF10;
            p->uv0 = pk->uv0;
            p->uv1 = pk->uv1;
            p->uv2 = pk->uv2;
            p->tpage = pk->tpage;
            p->clut = pk->clut;
            gte_ldsz3(SZ(pk->v0), SZ(pk->v1), SZ(pk->v2));
            gte_avsz3();
            gte_stotz(&otz);
            if (otz > 1 && otz < 1406) {
                addPrim(ot + otz, p);
                D_801EF6C8.prim += sizeof(PolyFT3w);
            }
        }
        D_8017E500++;
    }
}
