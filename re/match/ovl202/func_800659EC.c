#include "ac1/ac1.h"

#define gte_SetRotMatrix(r0) __asm__ volatile ("lw $12, 0(%0)\n\tlw $13, 4(%0)\n\tctc2 $12, $0\n\tctc2 $13, $1\n\tlw $12, 8(%0)\n\tlw $13, 12(%0)\n\tlw $14, 16(%0)\n\tctc2 $12, $2\n\tctc2 $13, $3\n\tctc2 $14, $4" : : "r"(r0) : "$12", "$13", "$14")
#define gte_SetTransMatrix(r0) __asm__ volatile ("lw $12, 20(%0)\n\tlw $13, 24(%0)\n\tctc2 $12, $5\n\tlw $14, 28(%0)\n\tctc2 $13, $6\n\tctc2 $14, $7" : : "r"(r0) : "$12", "$13", "$14")
#define gte_ldlv0(r0) __asm__ volatile ("lhu $13, 4(%0)\n\tlhu $12, 0(%0)\n\tsll $13, $13, 16\n\tor $12, $12, $13\n\tmtc2 $12, $0\n\tlwc2 $1, 8(%0)" : : "r"(r0) : "$12", "$13")
#define gte_rtv0tr() __asm__ volatile ("nop\n\tnop\n\t.word 0x4A480012")
#define gte_stlvnl(r0) __asm__ volatile ("swc2 $25, 0(%0)\n\tswc2 $26, 4(%0)\n\tswc2 $27, 8(%0)" : : "r"(r0) : "memory")
#define gte_ldclmv(r0) __asm__ volatile ("lhu $12, 0(%0)\n\tlhu $13, 6(%0)\n\tlhu $14, 12(%0)\n\tmtc2 $12, $9\n\tmtc2 $13, $10\n\tmtc2 $14, $11" : : "r"(r0) : "$12", "$13", "$14")
#define gte_rtir() __asm__ volatile ("nop\n\tnop\n\t.word 0x4A49E012")
#define gte_stclmv(r0) __asm__ volatile ("mfc2 $12, $9\n\tmfc2 $13, $10\n\tmfc2 $14, $11\n\tsh $12, 0(%0)\n\tsh $13, 6(%0)\n\tsh $14, 12(%0)" : : "r"(r0) : "$12", "$13", "$14", "memory")

typedef struct {                /* 0x7C model joint */
    u8 pad00[0x12];
    s16 part;                   /* +0x12 model part, < 0 = none */
    u8 pad14[0x14];
    MATRIX m;                   /* +0x28 local matrix (t at +0x3C) */
    u8 pad48[0x34];
} Joint;

typedef struct {
    u8 pad00[0x20];
    Joint *joints;              /* +0x20 */
} JointObj;

typedef struct {
    u8 pad000[0xEC];
    MATRIX base;                /* +0x0EC */
    MATRIX r;                   /* +0x10C */
    VECTOR t;                   /* +0x12C */
} JScr;

#define JSCR ((JScr *)0x1F800000)

typedef struct {
    u8 *model;                  /* 0x801A5F30 */
    u8 pad[0x610];
    SVECTOR *verts;             /* 0x801A6544 */
} MdlCur;

extern MATRIX D_800B1228;
extern MdlCur D_801A5F30;
extern void func_800658B8(u32 mask);
extern void func_80064DC8(u16 idx, void *a, s32 bits);
extern void func_8006611C(u16 idx, void *a, s32 mode, s32 bits);

/* draw every joint of a model with the camera matrix (D_800B1228) times
 * the joint's matrix */
void func_800659EC(JointObj *obj, u8 *info, void *arg2, u32 lights, u16 mode, s32 mask, s32 base)
{
    Joint *j;
    s32 i;
    s32 k;
    s32 idx;
    s32 *d;
    s32 *src;
    JScr *scr;

    scr = JSCR;
    j = obj->joints;
    i = info[5];
    func_800658B8(lights);
    src = (s32 *)&D_800B1228;
    d = (s32 *)&JSCR->base;
    k = 5;
    do {
        *d++ = *src++;
    } while (--k);
    scr->base.t[0] = D_800B1228.t[0];
    scr->base.t[1] = D_800B1228.t[1];
    scr->base.t[2] = D_800B1228.t[2];
    for (i--; i != -1; j++, i--) {
        if (j->part >= 0) {
            gte_SetRotMatrix(&scr->base);
            gte_SetTransMatrix(&scr->base);
            gte_ldlv0(j->m.t);
            gte_rtv0tr();
            gte_stlvnl(&scr->t);
            gte_ldclmv(&j->m.m[0][0]);
            gte_rtir();
            gte_stclmv(&scr->r.m[0][0]);
            gte_ldclmv(&j->m.m[0][1]);
            gte_rtir();
            gte_stclmv(&scr->r.m[0][1]);
            gte_ldclmv(&j->m.m[0][2]);
            gte_rtir();
            gte_stclmv(&scr->r.m[0][2]);
            gte_SetRotMatrix(&scr->r);
            gte_SetTransMatrix((MATRIX *)((u8 *)&scr->t - 20));
            idx = j->part + base;
            D_801A5F30.verts = *(SVECTOR **)(D_801A5F30.model + (idx * 32 + 12) - idx * 4);
            if (mode == 0) {
                func_80064DC8(idx, arg2, (1 << j->part) & mask);
            } else {
                func_8006611C(idx, arg2, mode, (1 << j->part) & mask);
            }
        }
    }
}
