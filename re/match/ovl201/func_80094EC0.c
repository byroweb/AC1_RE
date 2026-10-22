#include "ac1/ac1.h"
/* ovl201 0x80094EC0 - credits screen setup (after func_80094E58): once the
 * box opener is gone, draw the "CREDITS" and "C" strings, build three text
 * labels at D_8019C1A0..A8 and the static frame, then go to func_80095044. */
typedef struct { u8 b[0xC]; } TextDef;
extern TextDef D_800B8C74[3];
extern u8 D_800B8BA8[];
extern UiObj *volatile D_8019C1A0;
extern UiObj *D_8019C1A4;
extern UiObj *D_8019C1A8;
extern void func_8005BA2C(s32 x, s32 y, s32 w, s32 h, s32 a, s32 b, s32 c);
extern void func_8005D15C(char *s, s32 x, s32 y);
extern UiObj *func_8005CAE0(void *def, s32 x, s32 y, s32 z);
extern void func_8009406C(void);
extern void func_80095044(UiObj *o);
extern int DrawSync(int mode);

void func_80094EC0(UiObj *o)
{
    char title[9] = "CREDITS";
    char mark[9] = "C";

    if (obj_find_by_handle(func_8005BA2C) == &g_ui_none) {
        func_8005D15C(title, 0x3F0, 0xD0);
        D_8019C1A0 = func_8005CAE0(&D_800B8C74[0], 8, 5, 0x10);
        D_8019C1A0->light = 0x14;
        func_8005D15C(mark, 0x3FE, 0xD0);
        D_8019C1A4 = func_8005CAE0(&D_800B8C74[1], 0x80, 5, 0x10);
        D_8019C1A4->light = 0x14;
        func_8009406C();
        D_8019C1A8 = func_8005CAE0(&D_800B8C74[2], 0x40, 5, 0x10);
        D_8019C1A8->light = 0x14;
        DrawSync(0);
        func_8005CAE0(D_800B8BA8, 0, 0, 0x10);
        o->update = func_80095044;
    }
}
