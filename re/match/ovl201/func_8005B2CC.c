#include "ac1/ac1.h"

typedef struct {
    u8 c[3];
} Rgb3;

typedef struct {
    char s[2][18];
} AcNames;

extern int StoreImage(RECT *rect, u32 *p);
extern Rgb3 D_80031A64[4][4];
extern Rgb3 D_800B73A8[4][4];
extern u8 D_80039CA2;
extern u8 D_80039CA3;
extern RECT D_800B73E0;
extern RECT D_800B73D8;
extern u32 D_8004AC18[];
extern u32 D_80042B70[];
extern AcNames D_80031BD4;
extern AcNames D_800B7384;

void func_8005B2CC(void)
{
    s16 i;
    s16 j;

    D_80039CA2 = 0;
    D_80039CA3 = 0;
    for (i = 0; i < 4; i++) {
        for (j = 0; j < 4; j++) {
            D_80031A64[i][j] = D_800B73A8[i][j];
        }
    }
    StoreImage(&D_800B73E0, D_8004AC18);
    StoreImage(&D_800B73D8, D_80042B70);
    D_80031BD4 = D_800B7384;
}
