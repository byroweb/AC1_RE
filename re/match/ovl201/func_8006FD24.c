#include "ac1/ac1.h"

typedef struct EmbEdit {
    u8 pad[0x84];
    u8 zoom;                    /* +0x84 2/4/8 */
} EmbEdit;

typedef struct EmbZoomW {
    s16 x;                      /* +0x7C */
    s16 y;                      /* +0x7E */
    EmbEdit *edit;              /* +0x80 */
} EmbZoomW;

extern u8 D_800B818C[];
extern u8 D_800B8198[];
extern u8 D_800B81A4[];

void func_8006FD24(UiObj *o)
{
    EmbZoomW *w = (EmbZoomW *)&o->w;
    u8 zoom = w->edit->zoom;

    if (zoom == 2) {
        o->res = D_800B818C;
    } else if (zoom == 4) {
        o->res = D_800B8198;
    } else if (zoom == 8) {
        o->res = D_800B81A4;
    }
    o->pos.vx = w->x + 40;
    o->pos.vy = w->y + 48;
}
