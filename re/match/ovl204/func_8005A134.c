#include "ac1/ac1.h"
#include "m2c_compat.h"

/* PsyQ inline_c.h GTE macros */
#define gte_SetLightMatrix(r0) __asm__ volatile ("lw $12, 0(%0)\n\tlw $13, 4(%0)\n\tctc2 $12, $8\n\tctc2 $13, $9\n\tlw $12, 8(%0)\n\tlw $13, 12(%0)\n\tlw $14, 16(%0)\n\tctc2 $12, $10\n\tctc2 $13, $11\n\tctc2 $14, $12" : : "r"(r0) : "$12", "$13", "$14")
#define gte_SetRotMatrix(r0) __asm__ volatile ("lw $12, 0(%0)\n\tlw $13, 4(%0)\n\tctc2 $12, $0\n\tctc2 $13, $1\n\tlw $12, 8(%0)\n\tlw $13, 12(%0)\n\tlw $14, 16(%0)\n\tctc2 $12, $2\n\tctc2 $13, $3\n\tctc2 $14, $4" : : "r"(r0) : "$12", "$13", "$14")
#define gte_SetTransMatrix(r0) __asm__ volatile ("lw $12, 20(%0)\n\tlw $13, 24(%0)\n\tctc2 $12, $5\n\tlw $14, 28(%0)\n\tctc2 $13, $6\n\tctc2 $14, $7" : : "r"(r0) : "$12", "$13", "$14")

typedef struct {
    u8 m[0x14];                 /* one light matrix (3x3 s16 + pad) */
} LightMtx;

typedef struct {
    LightMtx lm[4];             /* +0x00 per-object light matrices */
    MATRIX col;                 /* +0x50 colour matrix */
    u8 unk70[0];
} LightSet;

typedef struct {
    u8 unk0[8];
    s32 off;                    /* +0x08 byte offset of the model body */
} ModelHdr;

typedef struct {
    s32 t[3];                   /* +0x00 translation */
    s16 mi;                     /* +0x0C matrix index; -1 ends the list */
    u16 part;                   /* +0x0E byte offset of the part in the model */
    WorldStruct *ws;            /* +0x10 */
    LightSet *lt;               /* +0x14 */
} DrawEnt;

/* render globals from 0x801ADAD8 (ovl204 copy) */
typedef struct {
    void *model;                /* +0x0000 */
    ModelHdr *tbl[0x1CC4];      /* +0x0004 */
    MATRIX rot[1];              /* +0x7314 (0x801B4FEC) */
} RenderGlobals;

extern void SetColorMatrix(MATRIX *m);
extern void SetBackColor(s32 r, s32 g, s32 b);
extern void SetFogNearFar(s32 a, s32 b, s32 h);
extern void func_8005A374(void *part, s32 a, s32 b);
extern s32 D_801B4F5C;
extern RenderGlobals D_801ADCD8;

/* func_800639D8 for whole models: fog only when the set has a fog range,
 * which also switches depth cueing (D_801B4F5C) */
void func_8005A134(DrawEnt *list)
{
    DrawEnt *e;
    LightSet *last;
    ModelHdr *m;
    u32 f;

    e = list;
    last = 0;
    for (; e->mi != -1; e++) {
        if (last != e->lt) {
            last = e->lt;
            SetColorMatrix(&last->col);
            SetBackColor(M2C_FIELD(last, u8 *, 0x62), M2C_FIELD(last, u8 *, 0x63), M2C_FIELD(last, u8 *, 0x64));
            f = M2C_FIELD(last, u16 *, 0x66);
            if (f != 0) {
                SetFogNearFar((f * 2) >> 5, (f * 2) >> 3, 80);
            }
            D_801B4F5C = M2C_FIELD(last, u16 *, 0x66);
        }
        gte_SetLightMatrix(&last->lm[e->mi]);
        gte_SetRotMatrix(&D_801ADCD8.rot[e->mi]);
        gte_SetTransMatrix((u8 *)e - 20);
        m = D_801ADCD8.tbl[e->ws->mesh + 260];
        func_8005A374((u8 *)m + m->off + 12, M2C_FIELD(e->ws, u32 *, 0x30) >> 28, 120);
    }
}
