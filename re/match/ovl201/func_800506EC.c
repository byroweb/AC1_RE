/* ovl201 func_800506EC: retire (kill = 1) every used object of one pool kind;
 * the same switch as func_800505F0 (ui_pool_set_light). */
#include "ac1/ac1.h"
void func_800506EC(s16 kind)
{
    UiObj *p;
    UiObj *end;

    switch (kind) {
    case 2:
        p = g_ui_pool_k2;
        end = p + 19;
        break;
    case 4:
        p = g_ui_pool_k4;
        end = p + 49;
        break;
    case 6:
        p = g_ui_pool_k6;
        end = p + 49;
        break;
    case 8:
        p = g_ui_pool_k8;
        end = p + 69;
        break;
    case 10:
        p = g_ui_pool_k10;
        end = p + 69;
        break;
    case 12:
        p = g_ui_pool_k12;
        end = p + 69;
        break;
    case 14:
        p = g_ui_pool_k14;
        end = p + 49;
        break;
    case 16:
        p = g_ui_pool_k16;
        end = p + 29;
        break;
    case 18:
        p = g_ui_pool_k18;
        end = p + 29;
        break;
    case 20:
        p = g_ui_pool_k20;
        end = p + 29;
        break;
    }
    for (; p <= end; p++) {
        if (p->used) {
            p->kill = 1;
        }
    }
}
