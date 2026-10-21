#include "ac1/ac1.h"
/* ovl201 0x80080EB0 - print seven lines of the D_800B827C text table,
 * starting at the object's first line (+0x8D), 16 pixels apart. */

typedef struct {
    u8 pad7c[0x11];
    u8 first;                   /* +0x8D */
} LinesWork;

extern char *D_800B827C[];
extern void func_8005D15C(char *s, s32 x, s16 y);

void func_80080EB0(UiObj *o)
{
    s16 i;
    u8 k = ((LinesWork *)&o->w)->first;

    for (i = 0; i < 7; i++) {
        func_8005D15C(D_800B827C[k], 960, i * 16 + 64);
        k++;
    }
}
