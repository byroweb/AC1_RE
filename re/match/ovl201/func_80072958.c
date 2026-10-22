#include "ac1/ac1.h"

typedef struct AsmPart {
    u8 hdr[0x7C];
    u16 slot;                   /* +0x7C index into the equipped-part bytes */
} AsmPart;

typedef struct AsmOwner {
    u8 hdr[0x88];
    UiObj *arms;                /* +0x88 */
    u8 pad8c[0x94 - 0x8C];
    u16 extra;                  /* +0x94 */
} AsmOwner;

extern u8 D_80031A58[];         /* equipped part ids */
extern u8 D_80031A5B;
extern u8 D_80031A5E;
extern u8 D_80031A5F;
extern void func_800744A8(void);
extern UiObj *func_80072C80(AsmOwner *owner, s32 kind, UiObj *parent, UiObj *parent2, u16 var);

UiObj *func_80072958(void)
{
    AsmOwner *owner;
    UiObj *legs, *core, *arms;
    u32 v;

    owner = (AsmOwner *)obj_find_by_handle(func_800744A8);
    legs = func_80072C80(owner, 6, &g_ui_none, &g_ui_none, 0);
    core = func_80072C80(owner, 4, legs, &g_ui_none, 0);
    func_80072C80(owner, 3, legs, core, 0);
    v = D_80031A58[((AsmPart *)legs)->slot];
    if (v < 21 || v == 255 || D_80031A5B >= 8) {
        arms = func_80072C80(owner, 5, legs, core, 0);
        func_80072C80(owner, 5, legs, core, 1);
    } else {
        arms = func_80072C80(owner, 5, legs, core, 2);
        func_80072C80(owner, 5, legs, core, 3);
    }
    if (D_80031A5B < 8) {
        func_80072C80(owner, 8, legs, core, 0);
    }
    if (D_80031A5E < 31) {
        func_80072C80(owner, 9, legs, core, 0);
    } else {
        switch (D_80031A58[0]) {
        case 0:
            func_80072C80(owner, 9, legs, core, 1);
            break;
        case 1:
            func_80072C80(owner, 9, legs, core, 2);
            break;
        case 2:
            func_80072C80(owner, 9, legs, core, 3);
            break;
        }
    }
    if (D_80031A5F < 31) {
        func_80072C80(owner, 10, legs, core, 1);
    } else {
        switch (D_80031A58[0]) {
        case 0:
            func_80072C80(owner, 10, legs, core, 1);
            break;
        case 1:
            func_80072C80(owner, 10, legs, core, 2);
            break;
        case 2:
            func_80072C80(owner, 10, legs, core, 3);
            break;
        }
    }
    v = D_80031A58[((AsmPart *)core)->slot];
    if (v >= 3) {
        v -= 3;
    }
    func_80072C80(owner, 14, legs, core, v);
    owner->arms = arms;
    if (owner->extra == 11) {
        func_80072C80(owner, 11, legs, core, 0);
    } else if (owner->extra == 12) {
        func_80072C80(owner, 12, legs, core, 0);
    } else if (owner->extra == 13) {
        func_80072C80(owner, 13, legs, core, 0);
    } else if (owner->extra == 7) {
        func_80072C80(owner, 7, legs, core, 0);
    }
    return legs;
}
