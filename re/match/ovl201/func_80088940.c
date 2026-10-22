#include "ac1/ac1.h"

typedef struct {
    s8 count;
    s8 rows;
    u8 pad2[8];
    volatile u8 bA;             /* +0x00A */
} NameList;

typedef struct {
    u8 pad0[0x1944];
    NameList *list;             /* +0x1944 0x801F5094 */
} Garage3750;

extern Garage3750 D_801F3750;
extern void func_80088838(void *p, s32 x, s32 y, s32 a, s32 b);

void func_80088940(void *p)
{
    switch ((s8)D_801F3750.list->bA) {
    case 1:
        func_80088838(p, 896, 0, 0, 481);
        break;
    case 2:
        func_80088838(p, 960, 0, 0, 482);
        break;
    case 3:
        func_80088838(p, 896, 128, 0, 483);
        break;
    }
    D_801F3750.list->bA = 0;
    DrawSync(0);
    free2(p);
}
