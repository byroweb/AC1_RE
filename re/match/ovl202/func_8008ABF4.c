#include "ac1/ac1.h"
/* ovl202 0x8008ABF4: draw two full-screen 255x240 sprites (tpage 0x85 / 0x87) side by side */
typedef struct {
    SprInfo spr;
    s16 pad[2];
} SprCell;
extern u32 D_80090920;
void func_80060B54(DVECTOR *xy, SprInfo *spr, s32 rot, u32 *rgb, s32 z, s32 abr);
void func_8008ABF4(void)
{
    DVECTOR xy[2] = { { 0, 0 }, { 255, 0 } };
    SprCell spr[2] = {
        { { 0x7800, 0x85, 0, 0, 255, 240 }, { 0, 0 } },
        { { 0x7800, 0x87, 0, 0, 64, 240 }, { 0, 0 } },
    };

    func_80060B54(&xy[0], &spr[0].spr, 0, &D_80090920, 0, 0);
    func_80060B54(&xy[1], &spr[1].spr, 0, &D_80090920, 0, 0);
}
