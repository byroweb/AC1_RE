#include "ac1/ac1.h"

typedef struct {
    s8 count;                   /* +0x000 */
    s8 rows;                    /* +0x001 */
    u8 pad2[0x22];
    u16 top;                    /* +0x024 */
} NameList;

typedef struct {
    u8 pad0[0x1944];
    NameList *list;             /* +0x1944 0x801F5094 */
} Garage3750;

extern Garage3750 D_801F3750;
typedef struct {
    volatile u8 unk0;
    s8 cursor;
    u8 scroll;
} ListState;
extern ListState D_801F37C8;
extern s8 D_801F37D4[64];
extern s16 D_801A2318;
extern void func_80086FD4(void);
extern void func_80086FFC(void);
extern void func_8005BA2C(s32 x, s32 y, s32 w, s32 h, s32 a, s32 b, s32 c);
extern void func_80088040(UiObj *o);

void func_80087AEC(UiObj *o)
{
    s32 i;

    if (obj_find_by_id(3) == &g_ui_none && D_801F37C8.unk0 == 0) {
        func_80086FD4();
        func_80086FFC();
        i = 64;
        D_801F3750.list->count = 0;
        while (--i != -1) {
            if (D_801F37D4[i] >= 0) {
                D_801F3750.list->count++;
            }
        }
        if (D_801F3750.list->count >= 6) {
            D_801F3750.list->rows = 5;
        } else {
            D_801F3750.list->rows = D_801F3750.list->count;
        }
        if (D_801F3750.list->count < D_801F37C8.cursor) {
            D_801F37C8.cursor = D_801F3750.list->count - 1;
        }
        D_801F3750.list->top = (5 - D_801F3750.list->rows) * 12;
        func_8005BA2C(0x20, (s16)(D_801F3750.list->top + 92), 256, D_801F3750.list->rows * 24, 3, 0, 0);
        SsUtKeyOn(D_801A2318, 0, 6, 60, 0, 127, 127);
        o->update = func_80088040;
    }
}
