#include "ac1/ac1.h"

typedef struct {
    s32 pad;
    void (*fn)();               /* +0x04 */
} Handler;

typedef struct {
    u8 unk00[0x14];
    Handler *h;                 /* +0x14 */
    u8 unk18[0x34 - 0x18];
} Ovl203Obj;

typedef struct {
    u8 unk000[0x24E];
    s16 alarm;                  /* +0x24E */
    u8 unk250[0x394 - 0x250];
    void *target;               /* +0x394 */
} Ovl203Ctl;

extern AcEntity D_8019B550[2];
extern Ovl203Obj D_80194A70[512];
extern WorldStruct D_801C40F0[];
extern s32 D_801DC0F4;          /* D_801C40F0 entry count */
extern Ovl203Ctl *D_80039D30;
extern void func_8007408C(void *, u16 *, s32, s32);

void func_80082284(void *src, void *obj, u16 *a2, s32 a3, s32 a4) {
    Ovl203Ctl *ctl;
    Handler *h;

    if ((AcEntity *)obj >= D_8019B550 && (AcEntity *)obj <= &D_8019B550[1]) {
        ctl = D_80039D30;
        if (obj != ctl->target && ((u32)src & 0x7FFFFFFF) == ((u32)ctl->target & 0x7FFFFFFF)) {
            ctl->alarm = 40;
        }
        func_8007408C(obj, a2, a3, a4);
    } else if ((Ovl203Obj *)obj >= D_80194A70 && (Ovl203Obj *)obj <= &D_80194A70[511]) {
        h = ((Ovl203Obj *)obj)->h;
        if (h != 0 && h->fn != 0) {
            h->fn(obj, a2, a3, a4, a2[1]);
        }
    } else if ((WorldStruct *)obj >= D_801C40F0 && (WorldStruct *)obj <= &D_801C40F0[D_801DC0F4 - 1]) {
        h = ((WorldStruct *)obj)->on_hit;
        if (h != 0 && h->fn != 0) {
            h->fn(obj, 0x8000, 0, a2, a3, a4);
        }
    }
}
