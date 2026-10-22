#include "ac1/ac1.h"
#include "m2c_compat.h"

typedef struct {
    u16 x, y, z;        /* +0x00 */
    u16 pad6;
    u16 a, b, c;        /* +0x08 */
    u8 pad0e[0x0E];
    u16 flags;          /* +0x1C */
    u8 pad1e[0x16];
} Slot34;               /* 0x34 */

extern Slot34 D_80198B08[];
extern u8 D_801C70A4[];
extern u16 D_801C70CE[];
extern s16 func_800132CC(s16, s16, s32);
extern s32 func_800148E4(s16, s16, s32);
extern s32 func_800754D0(void *, void *);
extern void func_80077B78(void *, s32, u16, u8, u16 *, s32, s32, s32);
extern void func_80078E1C(void *);
extern s32 func_801C5A90(void *);

void func_801C5B70(void *arg0) {
    u16 temp_s1;
    void *temp_v0;
    void *temp_v0_2;
    void *temp_v1_3;
    Slot34 *s;

    temp_v0 = M2C_FIELD(arg0, void **, 0x60);
    if (temp_v0 != NULL) {
        temp_v0_2 = M2C_FIELD(temp_v0, void **, 0x20);
        M2C_FIELD(temp_v0_2, u16 *, 0x274) = (u16) (M2C_FIELD(temp_v0_2, u16 *, 0x274) + 0x40);
        M2C_FIELD(temp_v0_2, u16 *, 0x292) = (u16) (M2C_FIELD(temp_v0_2, u16 *, 0x292) | 1);
    }
    M2C_FIELD(arg0, s16 *, 0x148) = 0;
    M2C_FIELD(arg0, s32 *, 0x15C) = 0;
    if (M2C_FIELD(arg0, u8 *, 0x40) != 0xFF) {
        if (M2C_FIELD(arg0, u8 *, 0x40) == 0) {
            s = &D_80198B08[M2C_FIELD(arg0, u16 *, 0xFE)];
            s->flags = s->flags & 0x7FFF;
            s->x = M2C_FIELD(arg0, u16 *, 8);
            s->y = M2C_FIELD(arg0, u16 *, 0xA);
            s->z = M2C_FIELD(arg0, u16 *, 0xC);
            s->a = M2C_FIELD(arg0, u16 *, 0x10);
            s->b = M2C_FIELD(arg0, u16 *, 0x12);
            s->c = M2C_FIELD(arg0, u16 *, 0x14);
        }
        if (func_800754D0(arg0, D_801C70A4) != 0) {
            func_80078E1C(arg0);
        }
    } else {
        if ((M2C_FIELD(arg0, s16 *, 0x7C) == 0) && (func_801C5A90(arg0) != 0)) {
            M2C_FIELD(arg0, s16 *, 0x12) = func_800132CC(M2C_FIELD(arg0, s16 *, 0x12), M2C_FIELD(arg0, s16 *, 0x78), 0x20);
            M2C_FIELD(arg0, s16 *, 0x7C) = 0;
            if (func_800148E4(M2C_FIELD(arg0, s16 *, 0x12), M2C_FIELD(arg0, s16 *, 0x78), 0x20) != 0) {
                if (M2C_FIELD(arg0, u8 *, 0x3F) == 0) {
                    D_801C70CE[0] = M2C_FIELD(arg0, u16 *, 0xFC);
                    temp_s1 = (u16) M2C_FIELD(arg0, s16 *, 0x78);
                    temp_v1_3 = M2C_FIELD(arg0, void **, 0);
                    M2C_FIELD(arg0, s16 *, 0x78) = 0;
                    func_80077B78(arg0, 0x17, M2C_FIELD(temp_v1_3, u16 *, 0x1C), M2C_FIELD(temp_v1_3, u8 *, 0x20), &D_801C70CE[-3], 1, 0, 1);
                    M2C_FIELD(arg0, s16 *, 0x148) = 0xF;
                    M2C_FIELD(arg0, u8 *, 0x14A) = 0xB4;
                    M2C_FIELD(arg0, u8 *, 0x14B) = 0xA0;
                    M2C_FIELD(arg0, u8 *, 0x14C) = 0x8C;
                    M2C_FIELD(arg0, s16 *, 0x78) = (s16) temp_s1;
                    M2C_FIELD(arg0, s32 *, 0x15C) = 1;
                    M2C_FIELD(arg0, u8 *, 0x3F) = 0x3CU;
                    goto block_12;
                }
                goto block_13;
            }
        }
block_12:
        if (M2C_FIELD(arg0, u8 *, 0x3F) != 0) {
block_13:
            M2C_FIELD(arg0, u8 *, 0x3F) = (u8) (M2C_FIELD(arg0, u8 *, 0x3F) - 1);
        }
    }
}
