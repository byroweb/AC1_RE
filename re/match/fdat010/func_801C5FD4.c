/* fdat010 0x801C5FD4: case 6 sums written (s16)(x - 7000) + r so fold keeps the constant on x (retail order); case 2 (v*128)>>12. */
#include "ac1/ac1.h"

typedef struct {
    u8 pad0[8];
    s32 cnt;            /* +0x08 */
} Ctr;
typedef struct {
    s16 x;
    s16 y;
    s16 z;
} V3;
typedef struct {
    s8 a;               /* +0x00 */
    u8 pad1;
    u16 b;              /* +0x02 */
    u8 c;               /* +0x04 */
    u8 pad5[3];
} Tag;
typedef struct {
    s16 x, y, z;        /* +0x00 */
    u8 pad6[8];
    u16 idx;            /* +0x0E */
    u8 pad10[4];
    Ctr *ctr;           /* +0x14 */
    u8 pad18[0x0A];
    u8 type;            /* +0x22 */
    u8 pad23;
    u16 scale;          /* +0x24 */
    u16 count;          /* +0x26 */
    u8 b28;             /* +0x28 */
    u8 b29;             /* +0x29 */
    u16 h2a;            /* +0x2A */
} Slot;

extern s32 rand(void);
extern u32 D_801AC860[];
extern u8 D_801C6610[];
extern u8 D_801C6614[];
extern u8 D_801C6618[];
extern u8 D_801C661C[];
extern u8 D_801C6620[];
extern u8 D_801C6624[];
extern void func_80014774(void *, V3 *, void *);
extern void func_80086598(s32, Tag *, void *, void *, Slot *, void *, V3 *, s32, s32);
extern void func_800889B4(s32, Slot *, V3 *, Tag *, s32, s32);
extern void func_80089628(Slot *, V3 *, s32, s32);
extern void func_800898A4(Tag *, void *, void *, Slot *, V3 *, void *, s32);
extern void func_80089A80(Slot *, V3 *);
extern void func_80089C30(V3 *, V3 *, s32);

void func_801C5FD4(Slot *s)
{
    Ctr *c;
    s32 cnt;
    s32 n;
    s32 t;
    s32 u;
    s16 t16;
    Tag tag;
    V3 v;
    V3 w;

    c = s->ctr;
    cnt = c->cnt;
    if (cnt != -1) {
        if (cnt == 0) {
            tag.a = 7;
            tag.b = s->h2a;
            tag.c = s->b29;
            c->cnt = s->b28;
            func_80014774((u8 *)s + 8, &v, c);
            switch (s->type) {
            case 0:
                s->scale = 576;
                v.x = (s->scale * v.x) >> 12;
                v.y = (s->scale * v.y) >> 12;
                v.z = (s->scale * v.z) >> 12;
                func_80086598(0, &tag, D_801C6610, D_801C6614, s, (u8 *)s + 8, &v, 55, 30000);
                func_800889B4(0, s, &v, &tag, 30000, 9);
                return;
            case 1:
            case 3:
                if (D_801AC860[(s16)s->idx >> 5] & (1 << (s->idx & 31))) {
                    v.x = (s->scale * v.x) >> 12;
                    v.y = (s->scale * v.y) >> 12;
                    v.z = (s->scale * v.z) >> 12;
                    func_80089628(s, &v, 12, s->type);
                }
                break;
            case 2:
                if (D_801AC860[(s16)s->idx >> 5] & (1 << (s->idx & 31))) {
                    v.x = (v.x * 128) >> 12;
                    v.y = (v.y * 128) >> 12;
                    v.z = (v.z * 128) >> 12;
                    func_800898A4(&tag, D_801C6618, D_801C661C, s, &v, (u8 *)s + 8, 6);
                }
                break;
            case 4:
                v.x = (s->scale * v.x) >> 12;
                v.y = (s->scale * v.y) >> 12;
                v.z = (s->scale * v.z) >> 12;
                func_80086598(0, &tag, D_801C6620, D_801C6624, s, (u8 *)s + 8, &v, 59, 30000);
                break;
            case 5:
                if (D_801AC860[(s16)s->idx >> 5] & (1 << (s->idx & 31))) {
                    v.x = (s->scale * v.x) >> 12;
                    v.y = (s->scale * v.y) >> 12;
                    v.z = (s->scale * v.z) >> 12;
                    func_80089A80(s, &v);
                }
                break;
            case 6:
                n = s->count;
                v.x = (s->scale * v.x) >> 12;
                v.y = (s->scale * v.y) >> 12;
                v.z = (s->scale * v.z) >> 12;
                n--;
                if (n != -1) {
                    do {
                        n--;
                        w.x = (s16)(s->x - 7000) + ((rand() * 14000) >> 15);
                        w.y = (s16)(s->y - 2000) + ((rand() * 2800) >> 15);
                        w.z = s->z;
                        func_80089C30(&w, &v, 96);
                    } while (n != -1);
                }
                break;
            }
        } else {
            c->cnt = cnt - 1;
        }
    }
}
