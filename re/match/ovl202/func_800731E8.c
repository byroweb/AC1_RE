#include "ac1/ac1.h"

typedef struct {
    s16 x, y, z, w;
} V4;

typedef struct {
    V4 a;               /* +0x00 (+0x06 type) */
    V4 b;               /* +0x08 */
    s32 u10;            /* +0x10 */
    s32 u14;            /* +0x14 */
    u8 u18, u19;
    u16 u1a;
    u16 f1c, f1e, f20;  /* +0x1C */
    u16 c[8];           /* +0x22 */
    u8 pad32[2];
} Node;

typedef struct {
    u16 a, b, c;
    u8 pad[0x12];
} NodeType;

extern Node D_80198B08[];
extern NodeType D_80198808[];
extern void func_80073170(void);

void func_800731E8(void *src)
{
    u16 *s;
    Node *n;
    s32 i;
    s32 j;
    u16 *d;
    NodeType *t;
    void (*fn)(Node *);

    n = D_80198B08;
    i = 0x200;
    s = src;
    do {
        n->a = *(V4 *)s;
        s += 4;
        n->b = *(V4 *)s;
        s += 4;
        d = n->c;
        j = 8;
        do {
            *d++ = *s++;
        } while (--j);
        n->u18 = 0;
        n->u19 = 0;
        n->u1a = 0;
        n->u10 = 0;
        n->u14 = 0;
        if (n->a.w != -1) {
            t = &D_80198808[n->a.w];
            n->f1c = t->a;
            n->f1e = t->b;
            n->f20 = t->c;
            fn = ((void (**)(Node *))((u8 *)g_objective_obj + 0xA0))[n->a.w];
            if (fn != 0) {
                fn(n);
            }
        } else {
            n->f1c = 0xC000;
        }
        n++;
    } while (--i);
    func_80073170();
}
