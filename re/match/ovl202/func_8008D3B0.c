#include "ac1/ac1.h"
/* ovl202 0x8008D3B0 (matched, no extra flags, no asm: rp is set in identical arms of a free `(s32)&p < 0` test so cse
 * forgets its constant and keeps the a2 copy (jump2 merges the arms and deletes the branch); a do/while(0) around the
 * first call keeps sched1 from launching sc's `lui s1` next to that branch; vx/vy loaded before the vx store, entity +0x2A read as an in-struct field): set up the scratchpad matrices for a player-centred overlay view
 * (light matrix, per-level scaled copies), then draw it (0x8008CA14 / D288 / CB5C / CC88). */
typedef struct {
    MATRIX m[9];        /* +0x000 */
    SVECTOR rot;        /* +0x120 */
    u8 pad128[0x140 - 0x128];
    u32 *ot;            /* +0x140 */
    u8 *cur;            /* +0x144 */
} Scr8008D3B0;
typedef struct { u8 pad[0x2A]; u16 h; } Ent2A;
#define SCR ((Scr8008D3B0 *)0x1F800000)
extern MATRIX D_8009087C;
extern MATRIX D_8009089C;
extern SVECTOR D_801ED798;
extern s16 D_801ED790;
extern s16 D_801ED792;
extern s16 D_801ED794;
extern u8 D_80041218;
extern u8 D_8004C1AC[];
extern DrawCtx D_801A5F24;
extern void func_8005715C(void);
extern void func_800571F0(void);
extern void SetColorMatrix(MATRIX *m);
extern void SetBackColor(long r, long g, long b);
extern void func_800135E8(MATRIX *src, MATRIX *dst, s32 n);
extern void func_80013BE8(SVECTOR *a, MATRIX *m, SVECTOR *rot);
extern void func_8008CA14(AcPos *p, s32 yaw, void *a);
extern void func_8008D288(void);
extern void func_8008CB5C(void);
extern void func_8008CC88(void);

void func_8008D3B0(void)
{
    AcPos p;
    s16 *src;
    s16 *dst;
    int i;
    Scr8008D3B0 *sc = SCR;

    do {
        func_8005715C();
    } while (0);
    SetColorMatrix(&D_8009089C);
    SetBackColor(128, 128, 128);
    SCR->m[5] = D_8009087C;
    func_800135E8(&SCR->m[5], &SCR->m[6], 1);
    func_800135E8(&SCR->m[6], &SCR->m[7], 1);
    func_800135E8(&SCR->m[7], &SCR->m[8], 1);
    {
        SVECTOR *rp;
        if ((s32)&p < 0) {
            rp = &SCR->rot;
            *(AcPos *)rp = g_ac_entities[0].pos;
        } else {
            rp = &SCR->rot;
            *(AcPos *)rp = g_ac_entities[0].pos;
        }
        {
            s16 tx = rp->vx;
            s16 ty = SCR->rot.vy;
            rp->vx = tx >> 2;
            SCR->rot.vy = ty >> 2;
        }
        SCR->rot.vz >>= 2;
        func_80013BE8(&D_801ED798, &SCR->m[0], rp);
    }
    src = SCR->m[0].m[0];
    dst = SCR->m[1].m[0];
    SCR->m[0].t[0] = D_801ED790;
    SCR->m[0].t[1] = D_801ED792;
    SCR->m[0].t[2] = D_801ED794;
    i = 9;
    do {
        *dst++ = *src++ >> 4;
    } while (--i != 0);
    func_800135E8(&sc->m[1], &sc->m[2], 1);
    func_800135E8(&sc->m[2], &sc->m[3], 1);
    func_800135E8(&sc->m[3], &sc->m[4], 1);
    {
        u8 *tb;
        u8 *c;
        u32 *ot;
        tb = D_8004C1AC;
        c = D_801A5F24.buf->cur;
        ot = D_801A5F24.ot;
        p = g_ac_entities[0].pos;
        sc->ot = ot;
        sc->cur = c;
        p.y -= ((Ent2A *)&g_ac_entities[0])->h >> 1;
        func_8008CA14(&p, g_ac_entities[0].yaw, tb);
    }
    func_8008D288();
    if (!(D_80041218 & 2)) {
        func_8008CB5C();
    }
    func_8008CC88();
    D_801A5F24.buf->cur = sc->cur;
    func_800571F0();
}
