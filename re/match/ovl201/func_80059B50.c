#include "ac1/ac1.h"

/* PsyQ inline_c.h GTE macros */
#define gte_ldv0(r0) __asm__ volatile ("lwc2 $0, 0(%0)\n\tlwc2 $1, 4(%0)" : : "r"(r0))
#define gte_rtps() __asm__ volatile ("nop\n\tnop\n\t.word 0x4A180001")
#define gte_stsxy(r0) __asm__ volatile ("swc2 $14, 0(%0)" : : "r"(r0) : "memory")
#define gte_stszotz(r0) __asm__ volatile ("mfc2 $12, $19\n\tnop\n\tsra $12, $12, 2\n\tsw $12, 0(%0)" : : "r"(r0) : "$12", "memory")

/* TMD object table entry (0x1C) */
typedef struct {
    SVECTOR *vert_top;
    s32 n_vert;
    void *normal_top;
    s32 n_normal;
    u8 *primitive_top;
    u32 n_primitive;
    s32 scale;
} TmdObj;

typedef struct {
    u32 id;
    u32 flags;
    u32 nobj;
    TmdObj obj[1];
} TmdHdr;

typedef void (*TmdPrimFn)(UiObj *o, void *normals);

extern TmdHdr *D_801F36D0[];
extern u8 *D_8017E508;
extern s32 D_8019F424[];
extern s32 D_801F07EC[];
extern TmdPrimFn D_800B7048[];

void func_80059B50(UiObj *o, s32 oi, s32 mi, MATRIX *m)
{
    TmdObj *obj;
    SVECTOR *v;
    s32 *sxy;
    s32 *z;
    s32 i;
    u32 nprim;

    obj = &D_801F36D0[mi]->obj[oi];
    D_8017E508 = obj->primitive_top;
    nprim = obj->n_primitive;
    SetRotMatrix(m);
    SetTransMatrix(m);
    sxy = D_8019F424;
    z = D_801F07EC;
    v = obj->vert_top;
    i = obj->n_vert;
    while (i > 0) {
        gte_ldv0(v);
        gte_rtps();
        v++;
        sxy++;
        z++;
        i--;
        gte_stsxy(sxy);
        gte_stszotz(z);
    }
    for (i = 0; i < nprim; i++) {
        D_800B7048[*D_8017E508](o, obj->normal_top);
    }
}
