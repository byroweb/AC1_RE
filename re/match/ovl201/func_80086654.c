#include "ac1/ac1.h"

typedef struct {
    s32 w[30];
} Blk120;

extern Blk120 D_80041B10;
extern Blk120 D_801F3750;

void func_80086654(void) {
    D_80041B10 = D_801F3750;
}
