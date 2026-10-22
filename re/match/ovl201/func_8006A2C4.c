#include "ac1/ac1.h"

typedef struct { u8 b[12]; } EmbSample;
extern EmbSample D_800B7EE0[];
void func_800506EC(s32);
void func_8005D754(s32, void *, s16, s16, s32);

typedef struct EmbSampleWork {  /* UiObj +0x7C */
    u8 pad[0x2C];
    u8 *state;                  /* +0xA8 */
    u8 padAC[0x1C];
    u16 scroll;                 /* +0xC8 */
} EmbSampleWork;

void func_8006A2C4(UiObj *o)
{
    EmbSampleWork *w = (EmbSampleWork *)&o->w;
    s16 n = 0;
    s16 row, col;

    func_800506EC(14);
    w->state[0] = 1;
    w->state[1] = 0;
    for (row = 0; row < 4; row++) {
        for (col = 0; col < 4; col++) {
            func_8005D754(3, &D_800B7EE0[n++], col * 0x44 + 8, row * 0x44 - (w->scroll - 6), 14);
        }
    }
}
