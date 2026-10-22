#include "ac1/ac1.h"

/* per-view ordering tables, double buffered (2 x 0x580 words each) */
extern u32 D_80180868[2][0x580];
extern u32 D_80183468[2][0x580];
extern u32 D_80186068[2][0x580];
extern u32 D_80188C68[2][0x580];
extern u32 D_8018B868[2][0x580];
extern u32 D_8018E470[2][0x580];
extern u32 D_80191070[2][0x580];
extern u32 D_80193C70[2][0x580];
extern u32 D_80196870[2][0x580];
extern u32 D_80199470[2][0x580];
extern MATRIX D_800B6ECC;    /* identity / default object matrix */

/* ui_obj_alloc: take the first free slot of the run reserved for this kind */
UiObj *func_800501BC(s16 kind)  /* ui_obj_alloc; symbols.h has (s32 kind) */
{
    UiObj *o;
    UiObj *end;
    UiView *view;
    u32 *ot[2];

    switch (kind) {
    case 0:
        o = &g_ui_pool[0];
        end = &g_ui_pool[19];
        view = &g_ui_views[0];
        ot[0] = D_80180868[0];
        ot[1] = D_80180868[1];
        break;
    case 1:
        o = &g_ui_pool[20];
        end = &g_ui_pool[20];
        view = &g_ui_views[0];
        ot[0] = D_80180868[0];
        ot[1] = D_80180868[1];
        break;
    case 2:
        o = &g_ui_pool[21];
        end = &g_ui_pool[40];
        view = &g_ui_views[0];
        ot[0] = D_80180868[0];
        ot[1] = D_80180868[1];
        break;
    case 3:
        o = &g_ui_pool[41];
        end = &g_ui_pool[42];
        view = &g_ui_views[1];
        ot[0] = D_80183468[0];
        ot[1] = D_80183468[1];
        break;
    case 4:
        o = &g_ui_pool[43];
        end = &g_ui_pool[92];
        view = &g_ui_views[1];
        ot[0] = D_80183468[0];
        ot[1] = D_80183468[1];
        break;
    case 5:
        o = &g_ui_pool[93];
        end = &g_ui_pool[94];
        view = &g_ui_views[2];
        ot[0] = D_80186068[0];
        ot[1] = D_80186068[1];
        break;
    case 6:
        o = &g_ui_pool[95];
        end = &g_ui_pool[144];
        view = &g_ui_views[2];
        ot[0] = D_80186068[0];
        ot[1] = D_80186068[1];
        break;
    case 7:
        o = &g_ui_pool[145];
        end = &g_ui_pool[146];
        view = &g_ui_views[3];
        ot[0] = D_80188C68[0];
        ot[1] = D_80188C68[1];
        break;
    case 8:
        o = &g_ui_pool[147];
        end = &g_ui_pool[216];
        view = &g_ui_views[3];
        ot[0] = D_80188C68[0];
        ot[1] = D_80188C68[1];
        break;
    case 9:
        o = &g_ui_pool[217];
        end = &g_ui_pool[218];
        view = &g_ui_views[4];
        ot[0] = D_8018B868[0];
        ot[1] = D_8018B868[1];
        break;
    case 10:
        o = &g_ui_pool[219];
        end = &g_ui_pool[288];
        view = &g_ui_views[4];
        ot[0] = D_8018B868[0];
        ot[1] = D_8018B868[1];
        break;
    case 11:
        o = &g_ui_pool[289];
        end = &g_ui_pool[290];
        view = &g_ui_views[5];
        ot[0] = D_8018E470[0];
        ot[1] = D_8018E470[1];
        break;
    case 12:
        o = &g_ui_pool[291];
        end = &g_ui_pool[360];
        view = &g_ui_views[5];
        ot[0] = D_8018E470[0];
        ot[1] = D_8018E470[1];
        break;
    case 13:
        o = &g_ui_pool[361];
        end = &g_ui_pool[362];
        view = &g_ui_views[6];
        ot[0] = D_80191070[0];
        ot[1] = D_80191070[1];
        break;
    case 14:
        o = &g_ui_pool[363];
        end = &g_ui_pool[412];
        view = &g_ui_views[6];
        ot[0] = D_80191070[0];
        ot[1] = D_80191070[1];
        break;
    case 15:
        o = &g_ui_pool[413];
        end = &g_ui_pool[414];
        view = &g_ui_views[7];
        ot[0] = D_80193C70[0];
        ot[1] = D_80193C70[1];
        break;
    case 16:
        o = &g_ui_pool[415];
        end = &g_ui_pool[444];
        view = &g_ui_views[7];
        ot[0] = D_80193C70[0];
        ot[1] = D_80193C70[1];
        break;
    case 17:
        o = &g_ui_pool[445];
        end = &g_ui_pool[446];
        view = &g_ui_views[8];
        ot[0] = D_80196870[0];
        ot[1] = D_80196870[1];
        break;
    case 18:
        o = &g_ui_pool[447];
        end = &g_ui_pool[476];
        view = &g_ui_views[8];
        ot[0] = D_80196870[0];
        ot[1] = D_80196870[1];
        break;
    case 19:
        o = &g_ui_pool[477];
        end = &g_ui_pool[478];
        view = &g_ui_views[9];
        ot[0] = D_80199470[0];
        ot[1] = D_80199470[1];
        break;
    case 20:
        o = &g_ui_pool[479];
        end = &g_ui_pool[508];
        view = &g_ui_views[9];
        ot[0] = D_80199470[0];
        ot[1] = D_80199470[1];
        break;
    case 21:
        o = &g_ui_pool[509];
        end = &g_ui_pool[509];
        view = &g_ui_views[9];
        ot[0] = D_80199470[0];
        ot[1] = D_80199470[1];
        break;
    }
    for (; o <= end; o++) {
        if (o->used == 0) {
            o->used = 1;
            o->kill = 0;
            o->visible = 1;
            o->attached = 1;
            o->has_xform = 1;
            o->view = 0;
            o->id = kind;
            o->parent = 0;
            o->scaled = 0;
            o->ot[0] = ot[0];
            o->ot[1] = ot[1];
            o->attach = view;
            o->mat = D_800B6ECC;
            return o;
        }
    }
    return &g_ui_none;
}
