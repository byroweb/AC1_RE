#include "ac1/ac1.h"
/* ovl203 func_80062020 (port of ovl202 func_80062BD8) - per-frame camera/view update: optionally take a new pose,
 * build the view matrices (scaled copies and two chains of four through
 * func_800135E8), set the translation from the pose, transform the eye vector,
 * then move every entry of the object table into view space with the GTE (RTV0)
 * and refresh its +/- radius box, and finally run the per-frame updaters. */

/* PsyQ inline_c.h GTE macros (no shared GTE header in the repo) */
#define gte_ldv0(r0) __asm__ volatile ("lwc2 $0, 0(%0)\n\tlwc2 $1, 4(%0)" : : "r"(r0))
#define gte_rtv0() __asm__ volatile ("nop\n\tnop\n\t.word 0x4A480012")
#define gte_stsv(r0) __asm__ volatile ("mfc2 $12, $9\n\tmfc2 $13, $10\n\tmfc2 $14, $11\n\tsh $12, 0(%0)\n\tsh $13, 2(%0)\n\tsh $14, 4(%0)" : : "r"(r0) : "$12", "$13", "$14", "memory")

typedef struct {
    SVECTOR pos;
    SVECTOR rot;
} V62020Pose;

typedef struct {
    u8 pad0[4];
    SVECTOR pos;    /* +0x04 */
    SVECTOR view;   /* +0x0C */
    SVECTOR lo;     /* +0x14 */
    SVECTOR hi;     /* +0x1C */
    u8 pad24[8];
    s16 radius;     /* +0x2C */
    s16 pad2E;
} V62020Entry;

typedef struct {
    V62020Pose pose;      /* +0x000 */
    MATRIX m10;                 /* +0x010 */
    VECTOR v30;                 /* +0x030 */
    MATRIX m40;                 /* +0x040 */
    MATRIX mats[8];             /* +0x060 */
    MATRIX m160;                /* +0x160 */
    MATRIX m180;                /* +0x180 */
    u8 pad1A0[0x11C0 - 0x1A0];
    V62020Entry entry[1]; /* +0x11C0 */
} V62020View;

typedef struct {
    SVECTOR v0;     /* +0x00 */
    SVECTOR v8;     /* +0x08 */
    u8 m10[0x10];   /* +0x10 */
} V62020Scratch;

extern V62020View D_801A4BF4;
extern s32 D_801A4BF0;
extern s32 D_801A60B8;
extern u8 D_801A3DA8[];
extern s32 D_801A3DA4;
extern MATRIX D_800315B4;

extern void func_80013F5C(SVECTOR *pos, SVECTOR *rot, MATRIX *m, void *work, MATRIX *out);
extern void func_800135E8(MATRIX *src, MATRIX *dst, s32 n);
extern void func_80061D7C(void);
extern VECTOR *ApplyMatrix(MATRIX *m, SVECTOR *v, VECTOR *r);
extern s32 func_8006D820(s32 h, SVECTOR *v);
extern void func_80061F4C(void);
extern void SetTransMatrix(MATRIX *m);
extern void func_80015538(void *p, s32 c, s32 n);
extern void func_80056B58(void);
extern void func_80055268(void);
extern void func_8005414C(void *a, s32 b, s32 c, s32 d, s32 e, s32 f, s32 g, void *h);
extern void func_80061A44(void);
extern void func_8005C82C(void);
extern void func_80060B3C(void);
extern void func_800567D0(void);
extern void func_80056BEC(void);
extern void func_80055360(void);
extern AcEntity D_8019B550[];

void func_80062020(V62020Pose *pose)
{
    V62020Scratch *scr = (V62020Scratch *)0x1F800200;
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
    V62020Entry *e;
    AcEntity *a;
    s32 i;

    if (pose != 0) {
        D_801A4BF4.pose = *pose;
    }
    func_80013F5C(&D_801A4BF4.pose.pos, &D_801A4BF4.pose.rot, &D_801A4BF4.m180,
                  scr->m10, &D_801A4BF4.m10);

    srcA = &D_801A4BF4.m180.m[0][0];
    dstA = &D_801A4BF4.m160.m[0][0];
    for (iA = 9; iA != 0; iA--) {
        *dstA++ = *srcA++ >> 2;
    }
    srcB = &D_801A4BF4.m10.m[0][0];
    dstB = &D_801A4BF4.m40.m[0][0];
    for (iB = 9; iB != 0; iB--) {
        *dstB++ = *srcB++ >> 2;
    }
    srcC = &D_801A4BF4.m10.m[0][0];
    dstC = &D_801A4BF4.mats[0].m[0][0];
    for (iC = 9; iC != 0; iC--) {
        *dstC++ = *srcC++ >> 3;
    }

    func_800135E8(&D_801A4BF4.mats[0], &D_801A4BF4.mats[1], 1);
    func_800135E8(&D_801A4BF4.mats[1], &D_801A4BF4.mats[2], 1);
    func_800135E8(&D_801A4BF4.mats[2], &D_801A4BF4.mats[3], 1);
    D_801A4BF4.mats[4] = D_801A4BF4.m10;
    func_800135E8(&D_801A4BF4.mats[4], &D_801A4BF4.mats[5], 1);
    func_800135E8(&D_801A4BF4.mats[5], &D_801A4BF4.mats[6], 1);
    func_800135E8(&D_801A4BF4.mats[6], &D_801A4BF4.mats[7], 1);
    func_80061D7C();

    D_801A4BF4.m10.t[0] = x = D_801A4BF4.pose.pos.vx;
    scr->v8.vx = x;
    D_801A4BF4.m10.t[1] = y = D_801A4BF4.pose.pos.vy;
    scr->v8.vy = y;
    D_801A4BF4.m10.t[2] = z = D_801A4BF4.pose.pos.vz;
    scr->v8.vz = z;
    scr->v0.vx = -scr->v8.vx;
    scr->v0.vy = -scr->v8.vy;
    scr->v0.vz = -scr->v8.vz;
    ApplyMatrix(&D_801A4BF4.m10, &scr->v0, &D_801A4BF4.v30);
    D_801A4BF0 = func_8006D820(D_801A4BF0, &scr->v8);
    func_80061F4C();
    SetTransMatrix(&D_800315B4);

    e = D_801A4BF4.entry;
    n = D_801A60B8;
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

    func_80015538(D_801A3DA8, 0, 16);
    func_80015538(D_801A3DA8 + 64, 0, 16);
    func_80056B58();
    func_80055268();
    a = D_8019B550;
    for (i = 2; i != 0; i--) {
        if (a->def != 0) {
            func_8005414C(&a->unk60, *(s16 *)((u8 *)a + 0xE) + 0x80, a->mover.mode,
                          *(s16 *)((u8 *)a + 0x6E), *(s16 *)((u8 *)a + 0x72), a->mover.weapon_slot,
                          a->mover.fire_timer, (u8 *)a + 0x76);
        }
        a++;
    }
    func_80061A44();
    func_8005C82C();
    func_80060B3C();
    func_800567D0();
    func_80056BEC();
    func_80055360();
    { s32 *p = &D_801A3DA4; (*p)++; }
}
