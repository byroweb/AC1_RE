#include "ac1/ac1.h"
/* ovl201 0x8007250C - once the func_8005BA2C object is gone and
 * func_800517C8 says 0, take the save-data copy, clear bit set 10 and open
 * func_800744A8 for the cursor's slot; then park. */

typedef struct {
    u8 b[0xA0];
} Blob160;

typedef struct {
    UiObj *cursor;              /* +0x7C */
} CursorWork;

extern Blob160 D_80031A94;
extern Blob160 D_80042AA8;
extern u32 D_800485A0[][2];
extern void func_8005BA2C(void);
extern s16 func_800517C8(void);
extern void func_80083A58(void);
extern void func_800744A8(UiObj *o, s32 a, u8 slot);

void func_8007250C(UiObj *o)
{
    if (obj_find_by_handle(func_8005BA2C) == &g_ui_none && func_800517C8() == 0) {
        D_80042AA8 = D_80031A94;
        func_80083A58();
        D_800485A0[10][0] = 0;
        D_800485A0[10][1] = 0;
        func_800744A8(&g_ui_none, 0, ((CursorWork *)&o->w)->cursor->w.raw[0]);
        o->update = ui_obj_nop2;
    }
}
