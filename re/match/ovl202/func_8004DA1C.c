#include "ac1/ac1.h"

extern void *CdSyncCallback(void *func);
extern s32 CdRead2(s32 mode);
extern s32 func_8001628C(void);
extern void func_8004D948(s32 a);
extern u16 D_80031764[][18];
extern u8 D_80041BBD;
extern u16 D_80041BBE;
extern volatile u8 D_80041BC2;
extern s32 D_80041BC4;
extern void *D_8008D924;

void func_8004DA1C(u8 status)
{
    u16 id;
    u16 *row;

    if (status == 2) {
        D_80041BC2++;
        if (D_80041BC2 == 0) {
            CdSyncCallback(D_8008D924);
            D_8008D924 = 0;
            while (CdRead2(200) == 0) {
            }
            func_8004D948(D_80041BBD);
            id = D_80041BBE;
            row = D_80031764[id >> 4];
            id &= 0xF;
            D_80041BC4 = func_8001628C() + (row[id] & 0xFFF) * 6;
        }
    }
}
