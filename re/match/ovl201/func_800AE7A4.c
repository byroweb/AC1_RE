#include "ac1/ac1.h"

typedef struct {
    s32 w[12];
} Blk48;

extern Blk48 D_80048630;
extern Blk48 D_800BDDA0;
extern u8 D_80048660;
extern u8 D_80048661;
extern u8 D_80048662;
extern u8 D_80048663;
extern u8 D_80048664;
extern u8 D_80041BB8;
extern u8 D_80041BBA;

/* Restore the default config block. */
void func_800AE7A4(void) {
    D_80048630 = D_800BDDA0;
    D_80048660 = 1;
    D_80048661 = 63;
    D_80048662 = 31;
    D_80041BB8 = 126;
    D_80048663 = 0;
    D_80048664 = 0;
    D_80041BBA = 62;
}
