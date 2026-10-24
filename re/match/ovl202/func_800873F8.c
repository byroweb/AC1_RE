/* MATCHED (wave4 a6). Levers: counter "for (i = n, i--; i != -1; i--)" (the split init adds refs so i outranks the cell-loop givs: i=s3), 80087070 last arg is the 8006E6B4 result. */
#include "ac1/ac1.h"
/* ovl203: blow debris off object o (an AC, a projectile-pool object or a
 * world cell): take up to n part positions from its model (func_80055EA8),
 * crumble cell parts (func_80085F98/func_80085EB4), then spawn one tumbling
 * piece per part (func_80085D1C) with random velocity, spin and angle. */
typedef struct {                /* 32: one model part from func_80055EA8 */
    u8 pad0[0x0C];
    s32 id;                     /* +0x0C -1 = none */
    u8 a[4];                    /* +0x10 position */
    u8 b[4];                    /* +0x14 */
    u8 c[2];                    /* +0x18 */
    u16 kind;                   /* +0x1A */
    u8 d[4];                    /* +0x1C */
} DPart3;
typedef struct {
    s16 h12_pad[9];
    s16 h12;                    /* +0x12 */
} DPiece3;
extern u8 D_8019B550[][0x1590];
extern u8 D_80194A70[];
extern u8 D_801C40F0[][0x40];
extern s32 D_801DC0F4;
extern s32 func_80055EA8(void *m, s32 a, void *pos, void *b, s32 n, DPart3 *out);
extern s32 func_8006DB54(s32 a, void *b);
extern s32 func_80014DAC(s32 n);
extern s32 rand(void);
extern DPiece3 *func_80085D1C(u8 *o, void *src, SVECTOR *ang, u8 *pos, SVECTOR *vel, SVECTOR *spin, s32 a, s32 b);
extern void func_80085F98(u8 *a, u8 *b, u8 *c, u8 *d);
extern void func_80085EB4(u8 *a, u8 *b, u8 *c);

void func_800873F8(u8 *o, void *src, s32 a2, s32 life, s32 n, s32 own)
{
    DPart3 parts[64];
    s32 unused[10];
    SVECTOR ang;
    SVECTOR vel;
    SVECTOR spin;
    SVECTOR rot;
    DPart3 *p;
    DPiece3 *e;
    s32 i;
    s32 sec;

    if (n > 64) {
        n = 64;
    }
    if (o >= D_8019B550[0] && o <= D_8019B550[1]) {
        if (*(u16 *)(o + 0x32) & 1) {
            return;
        }
        if (!func_80055EA8(*(void **)(o + 0x60), *(s16 *)(o + 0x0E) + 128, o + 8, o + 0x10, n, parts)) {
            return;
        }
    spawn:
        sec = func_8006DB54(-1, src);
        p = parts;
        for (i = n, i--; i != -1; i--) {
            {
                if (p->id != -1) {
                    vel.vx = func_80014DAC(50);
                    vel.vy = func_80014DAC(50) - 35;
                    vel.vz = func_80014DAC(50);
                    spin.vx = func_80014DAC(128);
                    spin.vy = func_80014DAC(128);
                    spin.vz = func_80014DAC(128);
                    ang.vx = rand() & 0xFFF;
                    ang.vy = rand() & 0xFFF;
                    ang.vz = rand() & 0xFFF;
                    if (own == 0) {
                        src = (u8 *)p + 4;
                    }
                    e = func_80085D1C(o, src, &ang, p->a, &vel, &spin, a2, sec);
                    e->h12 = func_80014DAC(life >> 2) + life;
                }
                p++;
            }
        }
    } else if (o >= D_80194A70 && o <= D_80194A70 + 0x67CC) {
        if (func_80055EA8(*(void **)(o + 0x10), *(s16 *)(o + 6) + 160, o, o + 8, n, parts)) {
            goto spawn;
        }
    } else if (o >= D_801C40F0[0] && o <= D_801C40F0[D_801DC0F4 - 1] && (*(u32 *)(o + 0x30) & 0x20000)) {
        rot.vz = 0;
        rot.vx = 0;
        rot.vy = *(u16 *)(o + 0x16) << 10;
        if (!func_80055EA8(0, *(s16 *)(o + 0x1E) + 260, o + 0x10, &rot, n, parts)) {
            return;
        }
        p = parts;
        for (i = n, i--; i != -1; i--) {
            {
                if (p->id != -1) {
                    if (p->kind == 0) {
                        func_80085F98(&p->a[0], &p->b[0], &p->c[0], &p->d[0]);
                        func_80085F98(&p->a[1], &p->b[1], &p->c[1], &p->d[1]);
                    } else {
                        func_80085EB4(&p->a[0], &p->b[0], &p->c[0]);
                        func_80085EB4(&p->a[1], &p->b[1], &p->c[1]);
                    }
                }
                p++;
            }
        }
        goto spawn;
    }
}
