#include "ac1/ac1.h"

/* AC part model object: work +0x7C holds the part slot it shows */
typedef struct {
    u16 slot;           /* +0x7C index into D_80031A58 */
} PartWork;

extern u8 D_80031A58[12];       /* equipped part per slot, 0xFF = none */
extern UiObj *func_800B3748(s32 kind, UiObj *a, UiObj *b, u16 idx, s32 n);

#define SLOT(o) (((PartWork *)&(o)->w)->slot)

/* Build the assembled AC model from the equipped parts. */
void func_800B3414(void)
{
    UiObj *a;
    UiObj *b;
    u8 t;
    u16 x;

    if (D_80031A58[0] != 0xFF) {
        a = func_800B3748(6, &g_ui_none, &g_ui_none, 0, 8);
        b = func_800B3748(4, a, &g_ui_none, 0, 8);
        func_800B3748(3, a, b, 0, 8);
        t = D_80031A58[SLOT(a)];
        if (t < 21 || t == 0xFF || D_80031A58[3] >= 8) {
            func_800B3748(5, a, b, 0, 8);
            func_800B3748(5, a, b, 1, 8);
        } else {
            func_800B3748(5, a, b, 2, 8);
            func_800B3748(5, a, b, 3, 8);
        }
        if (D_80031A58[3] < 8) {
            func_800B3748(8, a, b, 0, 8);
        }
        if (D_80031A58[6] < 31) {
            func_800B3748(9, a, b, 0, 8);
        } else {
            switch (D_80031A58[0]) {
            case 0:
                func_800B3748(9, a, b, 1, 8);
                break;
            case 1:
                func_800B3748(9, a, b, 2, 8);
                break;
            case 2:
                func_800B3748(9, a, b, 3, 8);
                break;
            }
        }
        if (D_80031A58[7] < 31) {
            func_800B3748(10, a, b, 1, 8);
        } else {
            switch (D_80031A58[0]) {
            case 0:
                func_800B3748(10, a, b, 1, 8);
                break;
            case 1:
                func_800B3748(10, a, b, 2, 8);
                break;
            case 2:
                func_800B3748(10, a, b, 3, 8);
                break;
            }
        }
        x = D_80031A58[SLOT(b)];
        if (x >= 3) {
            x -= 3;
        }
        func_800B3748(14, a, b, x, 8);
    }
}
