#include "ac1/ac1.h"
/* ovl201 0x80072134 - once the func_8005C2D0 object is gone, set up the two
 * 320 x 240 draw environments (top and bottom half of VRAM) with their
 * offsets centred, park, and hand over to func_80072298 (func_80068A20). */

typedef struct {
    RECT clip;
    s16 ofs[2];
    u8 rest[0x50];
} DRAWENV;

extern DRAWENV D_801BCEEC;
extern DRAWENV D_801D6344;
extern void func_8005C2D0(void);
extern void func_80072298(void);
extern DRAWENV *SetDefDrawEnv(DRAWENV *env, int x, int y, int w, int h);
extern void func_800506EC(s32 kind);
extern void func_80068A20(UiObj *o, void (*fn)(void));

void func_80072134(UiObj *o)
{
    if (obj_find_by_handle(func_8005C2D0) == &g_ui_none) {
        SetDefDrawEnv(&D_801BCEEC, 0, 0, 320, 240);
        SetDefDrawEnv(&D_801D6344, 0, 240, 320, 240);
        D_801BCEEC.ofs[0] = D_801BCEEC.clip.x + (D_801BCEEC.clip.w - 320) / 2;
        D_801BCEEC.ofs[1] = D_801BCEEC.clip.y + (D_801BCEEC.clip.h - 240) / 2;
        D_801D6344.ofs[0] = D_801D6344.clip.x + (D_801D6344.clip.w - 320) / 2;
        D_801D6344.ofs[1] = D_801D6344.clip.y + (D_801D6344.clip.h - 240) / 2;
        o->update = ui_obj_nop2;
        func_800506EC(4);
        func_80068A20(o, func_80072298);
    }
}
