#include "ac1/ac1.h"
/* matched (wave4 a5): -1 store before the cam copy, s reused for the linked section, seen/drawn chain store, if + do-while with sects set in the if, seen used as a constant in the loop (loop.c hoists it), (k << 6) + (u32)sects, flag via a temp */
/* ovl202 0x8005CE3C - draw the world around the current section: the section itself,
 * then every linked section that is near enough (flag 0x1000: squared distance /
 * 2^18 below its radius) and not hidden by a listed occluder (flag 0x100); marks
 * them in the visible-section bitmaps and hands the collected lists to
 * func_8005AF90 / func_800639D8. */

typedef struct {                /* 0x40, at 0x801D0B68 */
    u8 pad00[0x10];
    s16 x;                      /* +0x10 */
    s16 y;                      /* +0x12 */
    s16 z;                      /* +0x14 */
    u8 pad16[0x10];
    s16 nlink;                  /* +0x26 */
    u16 *link;                  /* +0x28 */
    u8 pad2c[4];
    u32 flags;                  /* +0x30 */
    u32 radius;                 /* +0x34 low 12 bits */
    u8 pad38[8];
} Sect;

typedef struct {
    u8 pad00[0xC];
    s16 end;                    /* +0x0C */
} DrawList;

typedef struct {                /* 0x801AC820 */
    u32 drawn[16];              /* +0x00 */
    u32 seen[16];               /* +0x40 (0x801AC860) */
    u8 pad80[0xDC8];
    Sect *cur;                  /* +0xE48 (0x801AD668) */
    u8 pade4c[0x24];
    s32 cam[3];                 /* +0xE70 (0x801AD690) */
} SectVis;

extern Sect D_801D0B68[];
extern SectVis D_801AC820;
extern s32 D_801987C8;
extern s32 D_801987CC;
extern void Square0(VECTOR *v0, VECTOR *v1);
extern void func_8005C904(Sect *s, s32 flag, DrawList **a, DrawList **b, SVECTOR *cam);
extern void func_8005AF90(u8 *buf);
extern void func_800639D8(u8 *buf);

#define SQ ((VECTOR *)0x1F800000)

void func_8005CE3C(void)
{
    SVECTOR cam;
    u8 unused[32];
    u8 buf1[0xC18];
    u8 buf2[0x1218];
    DrawList *l1;
    DrawList *l2;
    Sect *s;
    u16 *link;
    u16 *last;
    s32 n;
    s32 e;
    s32 k;
    u32 bit;
    Sect *sects;

    s = D_801AC820.cur;
    l1 = (DrawList *)buf1;
    l2 = (DrawList *)buf2;
    if (s == 0) {
        return;
    }
    D_801987CC = -1;
    cam.vx = D_801AC820.cam[0];
    cam.vy = D_801AC820.cam[1];
    cam.vz = D_801AC820.cam[2];
    D_801987C8 = 2;
    func_8005C904(s, 0, &l1, &l2, &cam);
    e = s - D_801D0B68;
    D_801AC820.drawn[e >> 5] = D_801AC820.seen[e >> 5] = 1 << (e & 0x1F);
    n = s->nlink;
    link = s->link;
    last = link + n - 1;
    if (--n != -1) {
    sects = D_801D0B68;
    do {
        e = *link;
        k = e & 0x1FF;
        s = (Sect *)((k << 6) + (u32)sects);
        link++;
        if (s->flags & 0x1000) {
            SQ->vx = (cam.vx - s->x) >> 1;
            SQ->vy = (cam.vy - s->y) >> 1;
            SQ->vz = (cam.vz - s->z) >> 1;
            Square0(SQ, SQ);
            if (((SQ->vx + SQ->vy + SQ->vz) >> 18) >= (s32)(s->radius & 0xFFF)) {
                continue;
            }
        }
        D_801AC820.drawn[k >> 5] |= 1 << (e & 0x1F);
        if ((e & 0x3800) && (sects[last[(e & 0x3800) >> 11]].flags & 0x100)) {
            continue;
        }
        D_801AC820.seen[k >> 5] |= 1 << (e & 0x1F);
        bit = e & 0x4000;
        func_8005C904(s, bit != 0, &l1, &l2, &cam);
    } while (--n != -1);
    }
    l1->end = l2->end = -1;
    func_8005AF90(buf1);
    func_800639D8(buf2);
}
