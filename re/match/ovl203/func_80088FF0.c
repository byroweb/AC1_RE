#include "ac1/ac1.h"

typedef struct {
    void *unk0;
    void (*fn)(void *obj, s32 msg, s32 arg);
} HitHandler;

extern AcEntity D_8019B550[];
extern WorldStruct D_801C40F0[];
extern s32 D_801DC0F4;

void func_80088FF0(void *obj, s32 msg)
{
    void (*fn)(void *, s32);
    HitHandler *h;

    if ((AcEntity *)obj >= &D_8019B550[0] && (AcEntity *)obj <= &D_8019B550[1]) {
        fn = (void (*)(void *, s32))((AcEntity *)obj)->fn_move;
        if (fn != 0) {
            fn(obj, msg);
        }
    } else if ((WorldStruct *)obj >= &D_801C40F0[0] && (WorldStruct *)obj <= &D_801C40F0[D_801DC0F4 - 1]) {
        h = ((WorldStruct *)obj)->on_hit;
        if (h != 0 && h->fn != 0) {
            h->fn(obj, msg, 0);
        }
    }
}
