#include "ac1/ac1.h"

typedef struct {
    u8 pad0[0x2C];
} Slot;
typedef struct {
    u8 pad0[0x10];
    u8 f10;
} Obj;
extern Slot D_8019FB68[];
extern Slot D_8019FC44[];
extern u16 D_8019F528;
extern s32 D_801CDB04;
extern Obj *D_801D3314;
extern Obj *D_801D3614;
extern void func_80078CFC();
extern void func_8008A80C();

void func_801C67AC(void)
{
    u8 t;

    func_8008A80C();
    if (D_8019F528 == 0) {
        switch (D_801CDB04) {
        case 0:
            t = D_801D3614->f10;
            if (t == 1) {
                D_801D3614->f10 = 4;
                func_80078CFC(&D_8019FB68[0]);
                func_80078CFC(&D_8019FB68[1]);
                func_80078CFC(&D_8019FB68[2]);
                func_80078CFC(&D_8019FB68[3]);
                func_80078CFC(&D_8019FB68[4]);
                D_801CDB04 = t;
                return;
            }
            break;
        case 1:
            if (D_801D3314->f10 == D_801CDB04) {
                D_801D3314->f10 = 4;
                func_80078CFC(&D_8019FC44[0]);
                func_80078CFC(&D_8019FC44[1]);
                func_80078CFC(&D_8019FC44[2]);
                func_80078CFC(&D_8019FC44[3]);
                D_801CDB04 = 2;
            }
            break;
        }
    }
}
