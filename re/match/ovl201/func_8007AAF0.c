#include "ac1/ac1.h"
/* ovl201 0x8007AAF0 - once func_800517C8 says 0, load the slot's settings
 * (model, category, flags, text) from the D_800B8204.. tables, copy the model
 * of the part equipped in that category (16 when none) into the preview, set
 * the spin speed (0 for categories 1 and 4..10) and go to func_80078A38. */

typedef struct {
    u8 pad7c[0x14];
    u16 text;                   /* +0x90 */
    s16 cat;                    /* +0x92 */
    s16 model;                  /* +0x94 */
    u8 slot;                    /* +0x96 */
    u8 pad97[0xA];
    u8 view;                    /* +0xA1 */
    u8 padA2[4];
    s16 part;                   /* +0xA6 */
    s16 part2;                  /* +0xA8 */
    u8 padAA;
    u8 flags;                   /* +0xAB */
    s16 spin;                   /* +0xAC */
} SlotWork;

extern u8 D_800B8204[];
extern u8 D_800B8210[];
extern u8 D_800B821C[];
extern u16 D_800B8228[];
extern u8 D_800B8240[];
extern u8 D_80031A58[];
extern u8 D_80039CA2;
extern s16 func_800517C8(void);
extern void func_800788C8(s16 dst, s16 src);
extern void func_80078A38(UiObj *o);

void func_8007AAF0(UiObj *o)
{
    SlotWork *w = (SlotWork *)&o->w;
    s16 k;

    if (func_800517C8() == 0) {
        k = w->slot;
        w->model = D_800B8204[k];
        w->cat = D_800B8210[k];
        w->flags = D_800B821C[k];
        w->text = D_800B8228[k];
        w->part = w->part2 = D_80031A58[D_800B8210[k]];
        if (D_80031A58[D_800B8210[k]] == 255) {
            func_800788C8(w->view, 16);
        } else {
            func_800788C8(w->view, D_800B8204[k]);
        }
        switch (D_800B8210[k]) {
        case 1:
        case 4 ... 10:
            w->spin = 0;
            break;
        default:
            w->spin = D_800B8240[k] * D_80039CA2;
            break;
        }
        o->update = func_80078A38;
    }
}
