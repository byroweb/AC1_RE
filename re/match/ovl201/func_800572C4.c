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

/* preprocessed TMD packet: flat triangle with one normal (0x10) */
typedef struct {
    u8 type;
    u8 pad1;
    s16 count;
    Rgbc c;     /* +0x04 */
    s16 n0;     /* +0x08 normal index */
    s16 v0;     /* +0x0A byte offsets into the vertex tables */
    s16 v1;
    s16 v2;
} TmdPktF3;

typedef struct {
    u32 tag;
    Rgbc c;
    s32 xy0, xy1, xy2;
} PolyF3;

typedef struct {
    u8 idx;         /* 0x801EF6C8 */
    u8 pad[3];
    u8 *prim;       /* 0x801EF6CC next free primitive */
} GfxBuf;

extern TmdPktF3 *D_8017E500;
extern GfxBuf D_801EF6C8;
extern u8 D_8019F428[];    /* screen xy per vertex */
extern u8 D_801F07F0[];    /* sz per vertex */

#define SXY(off) (*(s32 *)(D_8019F428 + (off)))
#define SZ(off) (*(s32 *)(D_801F07F0 + (off)))

void func_800572C4(u32 *ot, SVECTOR *normals)
{
    TmdPktF3 *pk;
    PolyF3 *p;
    s16 n;
    s32 opz;
    s32 otz;

    for (n = D_8017E500->count; n > 0; n--) {
        pk = D_8017E500;
        p = (PolyF3 *)D_801EF6C8.prim;
        gte_ldsxy3(SXY(pk->v0), SXY(pk->v1), SXY(pk->v2));
        gte_nclip();
        gte_stopz(&opz);
        if (opz > 0) {
            p->xy0 = SXY(pk->v0);
            p->xy1 = SXY(pk->v1);
            p->xy2 = SXY(pk->v2);
            gte_ldv0(&normals[pk->n0]);
            gte_ldrgb(&pk->c);
            gte_nccs();
            gte_strgb(&p->c);
            p->tag = 0x04FFFFFF;
            gte_ldsz3(SZ(pk->v0), SZ(pk->v1), SZ(pk->v2));
            gte_avsz3();
            gte_stotz(&otz);
            if (otz > 1 && otz < 1406) {
                addPrim(ot + otz, p);
                D_801EF6C8.prim += sizeof(PolyF3);
            }
        }
        D_8017E500++;
    }
}
