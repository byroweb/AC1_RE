#include "ac1_types.h"
/* ovl204 func_8006176C (twin of ovl202 func_80062BD8) - per-frame camera/view update: optionally take a new pose,
 * build the view matrices (scaled copies and two chains of four through
 * func_800135E8), set the translation from the pose, transform the eye vector,
 * then move every entry of the object table into view space with the GTE (RTV0)
 * and refresh its +/- radius box, and finally run the per-frame updaters. */

/* PsyQ inline_c.h GTE macros (no shared GTE header in the repo) */
#define gte_ldv0(r0) __asm__ volatile ("lwc2 $0, 0(%0)\n\tlwc2 $1, 4(%0)" : : "r"(r0))
#define gte_rtv0() __asm__ volatile ("nop\n\tnop\n\t.word 0x4A480012")
#define gte_stsv(r0) __asm__ volatile ("mfc2 $12, $9\n\tmfc2 $13, $10\n\tmfc2 $14, $11\n\tsh $12, 0(%0)\n\tsh $13, 2(%0)\n\tsh $14, 4(%0)" : : "r"(r0) : "$12", "$13", "$14", "memory")

typedef struct {
    s16 vx, vy, vz, pad;
} SVECTOR;

typedef struct {
    s32 vx, vy, vz, pad;
} VECTOR;

typedef struct {
    s16 m[3][3];
    s32 t[3];
} MATRIX;

typedef struct {
    SVECTOR pos;    /* +0x00 */
    SVECTOR rot;    /* +0x08 */
} Func80062BD8Pose;

typedef struct {
    u8 pad0[4];
    SVECTOR pos;    /* +0x04 */
    SVECTOR view;   /* +0x0C */
    SVECTOR lo;     /* +0x14 */
    SVECTOR hi;     /* +0x1C */
    u8 pad24[8];
    s16 radius;     /* +0x2C */
    s16 pad2E;
} Func80062BD8Entry;

typedef struct {
    Func80062BD8Pose pose;      /* +0x000 */
    MATRIX m10;                 /* +0x010 */
    VECTOR v30;                 /* +0x030 */
    MATRIX m40;                 /* +0x040 */
    MATRIX mats[8];             /* +0x060 */
    MATRIX m160;                /* +0x160 */
    MATRIX m180;                /* +0x180 */
    u8 pad1A0[0x11C0 - 0x1A0];
    Func80062BD8Entry entry[1]; /* +0x11C0 */
} Func80062BD8View;

typedef struct {
    SVECTOR v0;     /* +0x00 */
    SVECTOR v8;     /* +0x08 */
    u8 m10[0x10];   /* +0x10 */
} Func80062BD8Scratch;

extern Func80062BD8View D_801B4F8C;
extern s32 D_801AD668;
extern s32 D_801B6450;
extern u8 D_801B4118[];
extern s32 D_801AC81C;
extern MATRIX D_800315B4;

extern void func_80013F5C(SVECTOR *pos, SVECTOR *rot, MATRIX *m, void *work, MATRIX *out);
extern void func_800135E8(MATRIX *src, MATRIX *dst, s32 n);
extern void func_80062934(void);
extern VECTOR *ApplyMatrix(MATRIX *m, SVECTOR *v, VECTOR *r);
extern s32 func_8006E380(s32 h, SVECTOR *v);
extern void func_80061698(void);
extern void func_800611D0(void);
extern void func_8005BFE0(void);
extern void func_800602C8(void);
extern void SetTransMatrix(MATRIX *m);
extern void func_80015538(void *p, s32 c, s32 n);
extern void func_8005715C(void);
extern void func_8005586C(void);
extern void func_8006260C(void);
extern void func_80053190(void);
extern void func_8005CE3C(void);
extern void func_8006131C(void);
extern void func_80056DD4(void);
extern void func_800571F0(void);
extern void func_80055964(void);

void func_8006176C(void)
{
    Func80062BD8Scratch *scr = (Func80062BD8Scratch *)0x1F800200;
    s16 *srcA;
    s16 *dstA;
    s32 iA;
    s16 *srcB;
    s16 *dstB;
    s32 iB;
    s16 *srcC;
    s16 *dstC;
    s32 iC;
    s32 x;
    s32 y;
    s32 z;
    s32 n;
    Func80062BD8Entry *e;

    func_80013F5C(&D_801B4F8C.pose.pos, &D_801B4F8C.pose.rot, &D_801B4F8C.m180,
                  scr->m10, &D_801B4F8C.m10);

    srcA = &D_801B4F8C.m180.m[0][0];
    dstA = &D_801B4F8C.m160.m[0][0];
    for (iA = 9; iA != 0; iA--) {
        *dstA++ = *srcA++ >> 2;
    }
    srcB = &D_801B4F8C.m10.m[0][0];
    dstB = &D_801B4F8C.m40.m[0][0];
    for (iB = 9; iB != 0; iB--) {
        *dstB++ = *srcB++ >> 2;
    }
    srcC = &D_801B4F8C.m10.m[0][0];
    dstC = &D_801B4F8C.mats[0].m[0][0];
    for (iC = 9; iC != 0; iC--) {
        *dstC++ = *srcC++ >> 3;
    }

    func_800135E8(&D_801B4F8C.mats[0], &D_801B4F8C.mats[1], 1);
    func_800135E8(&D_801B4F8C.mats[1], &D_801B4F8C.mats[2], 1);
    func_800135E8(&D_801B4F8C.mats[2], &D_801B4F8C.mats[3], 1);
    D_801B4F8C.mats[4] = D_801B4F8C.m10;
    func_800135E8(&D_801B4F8C.mats[4], &D_801B4F8C.mats[5], 1);
    func_800135E8(&D_801B4F8C.mats[5], &D_801B4F8C.mats[6], 1);
    func_800135E8(&D_801B4F8C.mats[6], &D_801B4F8C.mats[7], 1);

    D_801B4F8C.m10.t[0] = x = D_801B4F8C.pose.pos.vx;
    scr->v8.vx = x;
    D_801B4F8C.m10.t[1] = y = D_801B4F8C.pose.pos.vy;
    scr->v8.vy = y;
    D_801B4F8C.m10.t[2] = z = D_801B4F8C.pose.pos.vz;
    scr->v8.vz = z;
    scr->v0.vx = -scr->v8.vx;
    scr->v0.vy = -scr->v8.vy;
    scr->v0.vz = -scr->v8.vz;
    ApplyMatrix(&D_801B4F8C.m10, &scr->v0, &D_801B4F8C.v30);
    func_80061698();
    SetTransMatrix(&D_800315B4);

    e = D_801B4F8C.entry;
    n = D_801B6450;
    while (--n != -1) {
        scr->v0.vx = e->pos.vx - scr->v8.vx;
        scr->v0.vy = e->pos.vy - scr->v8.vy;
        scr->v0.vz = e->pos.vz - scr->v8.vz;
        gte_ldv0(&scr->v0);
        gte_rtv0();
        gte_stsv(&e->view);
        e->lo.vx = e->view.vx - e->radius;
        e->hi.vx = e->view.vx + e->radius;
        e->lo.vy = e->view.vy - e->radius;
        e->hi.vy = e->view.vy + e->radius;
        e->lo.vz = e->view.vz - e->radius;
        e->hi.vz = e->view.vz + e->radius;
        e++;
    }

    func_80015538(D_801B4118, 0, 16);
    func_80015538(D_801B4118 + 64, 0, 16);
    func_800611D0();
    func_8005BFE0();
    func_800602C8();
}
