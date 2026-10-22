#include "ac1/ac1.h"

/* PsyQ inline_c.h GTE macros */
#define gte_ldv0(r0) __asm__ volatile ("lwc2 $0, 0(%0)\n\tlwc2 $1, 4(%0)" : : "r"(r0))
#define gte_ldrgb(r0) __asm__ volatile ("lwc2 $6, 0(%0)" : : "r"(r0))
#define gte_nccs() __asm__ volatile ("nop\n\tnop\n\t.word 0x4B08041B")
#define gte_strgb(r0) __asm__ volatile ("swc2 $22, 0(%0)" : : "r"(r0) : "memory")
#define gte_ldsz3(r0, r1, r2) __asm__ volatile ("mtc2 %0, $17\n\tmtc2 %1, $18\n\tmtc2 %2, $19" : : "r"(r0), "r"(r1), "r"(r2))
#define gte_avsz3() __asm__ volatile ("nop\n\tnop\n\t.word 0x4B58002D")
#define gte_stotz(r0) __asm__ volatile ("swc2 $7, 0(%0)" : : "r"(r0) : "memory")

/* preprocessed TMD primitive packet (0x18) */
typedef struct {
    u8 type;
    u8 pad1;
    s16 count;
    u8 pad4[14];
    s16 v0;     /* +0x12 byte offset into the vertex tables */
    s16 v1;
    s16 v2;
} TmdPkt;

/* closed flat polyline, 4 points + terminator (0x1C) */
typedef struct {
    u32 tag;
    u8 r0, g0, b0, code;
    s32 xy0, xy1, xy2, xy3;
    u32 term;
} LineF4Loop;

extern TmdPkt *D_8017E508;
typedef struct {
    u8 idx;         /* 0x801EF6C8 */
    u8 pad[3];
    u8 *prim;       /* 0x801EF6CC next free primitive */
} GfxBuf;

extern GfxBuf D_801EF6C8;
extern u8 D_8019F428[];    /* screen xy per vertex */
extern u8 D_801F07F0[];    /* sz per vertex */
extern SVECTOR D_800B7098;
extern u8 D_800B7094[4];

#define SXY(off) (*(s32 *)(D_8019F428 + (off)))
#define SZ(off) (*(s32 *)(D_801F07F0 + (off)))

void func_80059E44(u32 *ot, void *normals)
{
    TmdPkt *pk;
    LineF4Loop *p;
    s16 n;
    s32 otz;

    for (n = D_8017E508->count; n > 0; n--) {
        pk = D_8017E508;
        p = (LineF4Loop *)D_801EF6C8.prim;
        p->xy0 = SXY(pk->v0);
        p->xy1 = SXY(pk->v1);
        p->xy2 = SXY(pk->v2);
        p->xy3 = SXY(pk->v0);
        gte_ldv0(&D_800B7098);
        gte_ldrgb(D_800B7094);
        gte_nccs();
        gte_strgb(&p->r0);
        setlen(p, 6);
        p->code = 0x4C;
        p->term = 0x55555555;
        gte_ldsz3(SZ(pk->v0), SZ(pk->v1), SZ(pk->v2));
        gte_avsz3();
        gte_stotz(&otz);
        if (otz > 1 && otz < 1406) {
            addPrim(ot + otz, p);
            D_801EF6C8.prim += sizeof(LineF4Loop);
        }
        D_8017E508++;
    }
}
