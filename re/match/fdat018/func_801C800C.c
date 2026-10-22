#include "ac1/ac1.h"

typedef struct {
    u8 pad[0x2C];
} Slot2C;
extern Slot2C D_8019FB68[];
extern u16 D_80041226;
extern u16 D_8019F528;
extern void func_800529F0(s32, s32, s32);
extern void func_80078CFC(Slot2C *);

s32 func_801C800C(s32 phase)
{
    if (D_8019F528 == 0) {
        switch (phase) {
        case 0:
            if (D_80041226 & 1) {
                func_800529F0(0, 0, 131);
            }
            break;
        case 1:
            if (D_80041226 & 1) {
                func_800529F0(0, 0, 129);
                func_800529F0(0, 0, 130);
            }
            func_80078CFC(&D_8019FB68[0]);
            func_80078CFC(&D_8019FB68[1]);
            func_80078CFC(&D_8019FB68[2]);
            break;
        case 2:
            break;
        }
    }
    return 1;
}
