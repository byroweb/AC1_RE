#include "ac1/ac1.h"
/* ovl202 0x800835E8: deliver a hit to whatever obj is (AC entity, effect object, world struct); ovl203 twin 0x80082284 */
typedef struct {
    s32 pad;
    void (*fn)();               /* +0x04 */
} Handler;

typedef struct {
    u8 unk00[0x14];
    Handler *h;                 /* +0x14 */
    u8 unk18[0x34 - 0x18];
} Ovl202Obj;

extern Ovl202Obj D_80198B08[512];
extern s16 D_800411EE;
extern void func_80075350(void *, u16 *, s32, s32);

void func_800835E8(void *src, void *obj, u16 *a2, s32 a3, s32 a4) {
    Handler *h;

    if ((AcEntity *)obj >= g_ac_entities && (AcEntity *)obj <= &g_ac_entities[15]) {
        if (obj != g_ac_entities && ((u32)src & 0x7FFFFFFF) == ((u32)g_ac_entities & 0x7FFFFFFF)) {
            D_800411EE = 40;
        }
        func_80075350(obj, a2, a3, a4);
    } else if ((Ovl202Obj *)obj >= D_80198B08 && (Ovl202Obj *)obj <= &D_80198B08[511]) {
        h = ((Ovl202Obj *)obj)->h;
        if (h != 0 && h->fn != 0) {
            h->fn(obj, a2, a3, a4, a2[1]);
        }
    } else if ((WorldStruct *)obj >= g_world_structs && (WorldStruct *)obj <= &g_world_structs[g_world_struct_count - 1]) {
        h = ((WorldStruct *)obj)->on_hit;
        if (h != 0 && h->fn != 0) {
            h->fn(obj, 0x8000, 0, a2, a3, a4);
        }
    }
}
