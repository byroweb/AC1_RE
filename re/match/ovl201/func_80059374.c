#include "ac1/ac1.h"

/* PsyQ inline_c.h GTE macros */
#define gte_ldv3(r0, r1, r2) __asm__ volatile ("lwc2 $0, 0(%0)\n\tlwc2 $1, 4(%0)\n\tlwc2 $2, 0(%1)\n\tlwc2 $3, 4(%1)\n\tlwc2 $4, 0(%2)\n\tlwc2 $5, 4(%2)" : : "r"(r0), "r"(r1), "r"(r2))
#define gte_ldv0(r0) __asm__ volatile ("lwc2 $0, 0(%0)\n\tlwc2 $1, 4(%0)" : : "r"(r0))
#define gte_nccs() __asm__ volatile ("nop\n\tnop\n\t.word 0x4B08041B")
#define gte_strgb(r0) __asm__ volatile ("swc2 $22, 0(%0)" : : "r"(r0) : "memory")
#define gte_ldrgb(r0) __asm__ volatile ("lwc2 $6, 0(%0)" : : "r"(r0))
#define gte_ncct() __asm__ volatile ("nop\n\tnop\n\t.word 0x4B18043F")
#define gte_strgb3(r0, r1, r2) __asm__ volatile ("swc2 $20, 0(%0)\n\tswc2 $21, 0(%1)\n\tswc2 $22, 0(%2)" : : "r"(r0), "r"(r1), "r"(r2) : "memory")
#define gte_ldsxy3(r0, r1, r2) __asm__ volatile ("mtc2 %0, $12\n\tmtc2 %2, $14\n\tmtc2 %1, $13" : : "r"(r0), "r"(r1), "r"(r2))
#define gte_nclip() __asm__ volatile ("nop\n\tnop\n\t.word 0x4B400006")
#define gte_stopz(r0) __asm__ volatile ("swc2 $24, 0(%0)" : : "r"(r0) : "memory")
#define gte_ldsz3(r0, r1, r2) __asm__ volatile ("mtc2 %0, $17\n\tmtc2 %1, $18\n\tmtc2 %2, $19" : : "r"(r0), "r"(r1), "r"(r2))
#define gte_avsz3() __asm__ volatile ("nop\n\tnop\n\t.word 0x4B58002D")
#define gte_stotz(r0) __asm__ volatile ("swc2 $7, 0(%0)" : : "r"(r0) : "memory")

typedef struct {
    u8 r, g, b, code;
} Rgbc;

/* preprocessed TMD packet: gouraud textured quad (0x24) */
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
    u16 uv3;    /* +0x10 */
    u16 pad12;
    s16 n0;     /* +0x14 normal index */
    s16 v0;     /* +0x16 byte offset into the vertex tables */
    s16 n1;
    s16 v1;
    s16 n2;
    s16 v2;
    s16 n3;
    s16 v3;
} TmdPktGT4;

typedef struct {
    u32 tag;
    Rgbc c0;
    s32 xy0;
    u16 uv0, clut;
    Rgbc c1;
    s32 xy1;
    u16 uv1, tpage;
    Rgbc c2;
    s32 xy2;
    u16 uv2, pad1;
    Rgbc c3;
    s32 xy3;
    u16 uv3, pad2;
} PolyGT4w;

typedef struct {
    u8 idx;         /* 0x801EF6C8 */
    u8 pad[3];
    u8 *prim;       /* 0x801EF6CC next free primitive */
} GfxBuf;

extern TmdPktGT4 *D_8017E500;
extern GfxBuf D_801EF6C8;
extern u8 D_8019F428[];    /* screen xy per vertex */
extern u8 D_801F07F0[];    /* sz per vertex */
extern u8 D_800B703C[4];   /* base colour */

#define SXY(off) (*(s32 *)(D_8019F428 + (off)))
#define SZ(off) (*(s32 *)(D_801F07F0 + (off)))

void func_80059374(u32 *ot, SVECTOR *normals)
{
    TmdPktGT4 *pk;
    PolyGT4w *p;
    s16 n;
    s32 opz;
    s32 otz;

    for (n = D_8017E500->count; n > 0; n--) {
        pk = D_8017E500;
        p = (PolyGT4w *)D_801EF6C8.prim;
        gte_ldsxy3(SXY(pk->v0), SXY(pk->v1), SXY(pk->v2));
        gte_nclip();
        gte_stopz(&opz);
        if (opz > 0) {
            p->tag = 0x0C7FFFD8;
            gte_ldv3(&normals[pk->n0], &normals[pk->n1], &normals[pk->n2]);
            gte_ldrgb(D_800B703C);
            gte_ncct();
            gte_strgb3(&p->c0, &p->c1, &p->c2);
            gte_ldv0(&normals[pk->n3]);
            gte_ldrgb(D_800B703C);
            gte_nccs();
            gte_strgb(&p->c3);
            p->xy0 = SXY(pk->v0);
            p->xy1 = SXY(pk->v1);
            p->xy2 = SXY(pk->v2);
            p->xy3 = SXY(pk->v3);
            p->uv0 = pk->uv0;
            p->uv1 = pk->uv1;
            p->uv2 = pk->uv2;
            p->uv3 = pk->uv3;
            p->tpage = pk->tpage;
            p->clut = pk->clut;
            gte_ldsz3(SZ(pk->v0), SZ(pk->v1), SZ(pk->v2));
            gte_avsz3();
            gte_stotz(&otz);
            if (otz > 1 && otz < 1406) {
                addPrim(ot + otz, p);
                D_801EF6C8.prim += sizeof(PolyGT4w);
            }
        }
        D_8017E500++;
    }
}
