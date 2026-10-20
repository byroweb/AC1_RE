/* ovl202 0x800730DC  size 40  permuter tier (tools/loop/permute.py): levers bytes var_v1 += 0x40, from the ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

void func_800730DC(void) {
    WorldStruct *var_v1;
    s32 var_v0;

    var_v1 = g_world_structs;
    var_v0 = g_world_struct_count;
    do {
        var_v1->on_hit = NULL;
        var_v0 -= 1;
        var_v1 = (WorldStruct *)((u8 *)var_v1 + 0x40);
    } while (var_v0 != 0);
}
