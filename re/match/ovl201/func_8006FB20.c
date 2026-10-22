#include "ac1/ac1.h"

typedef struct EmbEdit {
    u8 pad[0x84];
    u8 zoom;                    /* +0x84 2/4/8 */
} EmbEdit;

typedef struct EmbPen {
    u8 hdr[0x7E];
    s16 x;                      /* +0x7E screen x */
    s16 y;                      /* +0x80 screen y */
    u8 pad82[2];
    EmbEdit *edit;              /* +0x84 */
} EmbPen;

extern u8 D_800B7EB0[];
extern u8 D_800B7EBC[];
extern u8 D_800B7EC8[];
extern u8 D_800B7ED4[];

void func_8006FB20(EmbPen *p)
{
    UiObj *o = (UiObj *)p;

    if ((u16)(p->x - 172) >= 128 || (u16)(p->y - 36) >= 128) {
        o->res = D_800B7EB0;
        o->pos.vx = p->x;
        o->pos.vy = p->y;
    } else {
        o->pos.vx = ((p->x - 172) & -p->edit->zoom) + 171;
        o->pos.vy = ((p->y - 36) & -p->edit->zoom) + 35;
        if (p->edit->zoom == 2) {
            o->res = D_800B7EBC;
        } else if (p->edit->zoom == 4) {
            o->res = D_800B7EC8;
        } else {
            o->res = D_800B7ED4;
        }
    }
}
