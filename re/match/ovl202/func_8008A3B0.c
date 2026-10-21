#include "ac1/ac1.h"

typedef struct {
    s32 pad;
    void (*fn)(void *, s32, s32);
} HitRec;

void func_8008A3B0(void *obj, s32 msg) {
    AcEntity *e;
    WorldStruct *w;
    HitRec *h;

    e = obj;
    w = obj;
    if (e >= g_ac_entities && e <= &g_ac_entities[15]) {
        if (e->fn_move != 0) {
            ((void (*)(AcEntity *, s32))e->fn_move)(e, msg);
        }
    } else if (w >= g_world_structs && w <= &g_world_structs[g_world_struct_count - 1]) {
        h = w->on_hit;
        if (h != 0 && h->fn != 0) {
            h->fn(w, msg, 0);
        }
    }
}
