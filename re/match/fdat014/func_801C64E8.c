/* MATCHED (wave4 a6), twin of ovl203 0x800860A4: counter "for (i = n, i--; i != -1; i--)". */
#include "ac1/ac1.h"
/* fdat014 twin of ovl203 0x800860A4: blow debris off an AC, a projectile-pool
 * object or a world cell. */
typedef struct {                /* 32: one model part */
    u8 pad0[0x0C];
    s32 id;                     /* +0x0C -1 = none */
    u8 a[4];                    /* +0x10 */
    u8 b[4];                    /* +0x14 */
    u8 c[2];                    /* +0x18 */
    u16 kind;                   /* +0x1A */
    u8 d[4];                    /* +0x1C */
} DPart4;
typedef struct {
    s16 h12_pad[9];
    s16 h12;                    /* +0x12 */
} DPiece4;
extern u8 D_801A26B8[][0x170];
extern u8 D_80198B08[];
extern u8 D_801D0B68[][0x40];
extern s32 D_801E8B6C;
extern u8 D_801C8500[];
extern s32 func_800564AC(void *m, s32 a, void *pos, void *b, s32 n, DPart4 *out);
extern s32 func_8006E6B4(s32 a, void *b);
extern s32 func_80014DAC(s32 n);
extern s32 rand(void);
extern DPiece4 *func_80087070(u8 *o, void *src, SVECTOR *ang, u8 *pos, SVECTOR *vel, SVECTOR *spin, s32 a, s32 b);
extern void func_801C63DC(u8 *a, u8 *b, u8 *c, u8 *d);
extern void func_801C62F8(u8 *a, u8 *b, u8 *c);

void func_801C64E8(u8 *o, void *src, s32 a2, s32 life, s32 n, s32 own)
{
    DPart4 parts[64];
    SVECTOR spare;
    SVECTOR ang;
    SVECTOR vel;
    SVECTOR spin;
    SVECTOR rot;
    DPart4 *p;
    DPiece4 *e;
    s32 i;
    s32 sec;

    if (n > 64) {
        n = 64;
    }
    if (o >= D_801A26B8[0] && o <= D_801A26B8[15]) {
        if (*(u16 *)(o + 0x32) & 1) {
            return;
        }
        if (!func_800564AC(*(void **)(o + 0x60), *(s16 *)(o + 0x0E) + 128, o + 8, o + 0x10, n, parts)) {
            return;
        }
    spawn:
        sec = func_8006E6B4(-1, src);
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
                    e = func_80087070(o, src, &ang, D_801C8500, &vel, &spin, a2, sec);
                    e->h12 = func_80014DAC(life >> 2) + life;
                }
                p++;
            }
        }
    } else if (o >= D_80198B08 && o <= D_80198B08 + 0x67CC) {
        if (func_800564AC(*(void **)(o + 0x10), *(s16 *)(o + 6) + 160, o, o + 8, n, parts)) {
            goto spawn;
        }
    } else if (o >= D_801D0B68[0] && o <= D_801D0B68[D_801E8B6C - 1] && (*(u32 *)(o + 0x30) & 0x20000)) {
        rot.vz = 0;
        rot.vx = 0;
        rot.vy = *(u16 *)(o + 0x16) << 10;
        if (!func_800564AC(0, *(s16 *)(o + 0x1E) + 260, o + 0x10, &rot, n, parts)) {
            return;
        }
        p = parts;
        for (i = n, i--; i != -1; i--) {
            {
                if (p->id != -1) {
                    if (p->kind == 0) {
                        func_801C63DC(&p->a[0], &p->b[0], &p->c[0], &p->d[0]);
                        func_801C63DC(&p->a[1], &p->b[1], &p->c[1], &p->d[1]);
                    } else {
                        func_801C62F8(&p->a[0], &p->b[0], &p->c[0]);
                        func_801C62F8(&p->a[1], &p->b[1], &p->c[1]);
                    }
                }
                p++;
            }
        }
        goto spawn;
    }
}
