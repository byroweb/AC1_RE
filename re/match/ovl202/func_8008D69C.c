#include "ac1/ac1.h"

typedef struct {                /* 0x801ED790: map camera */
    s16 x;
    s16 y;
    s16 z;                      /* +0x04 zoom distance */
    u8 pad06[2];
    s16 pitch;                  /* +0x08 */
    u16 yaw;                    /* +0x0A */
} MapCam;

extern MapCam D_801ED790;
extern u32 D_80040FA8;
extern u8 D_80041218;
extern u32 D_80090850;
extern u32 func_80016A84();
extern void func_80062BD8();
extern void func_8004CFC8(s32, s32);
extern void func_8004D020(s32, s32, s32);
extern s16 func_8008D668(s32 v, s32 d, s32 lo, s32 hi);
extern void func_8008D3B0(void);

/* Map screen: pad-driven camera until the map button is pressed again. */
void func_8008D69C(void) {
    s32 n;
    u32 prev;
    u32 cur;

    DrawSync(0);
    if (D_80041218 & 1) {
        n = g_world_struct_count;
        {
            WorldStruct *w = g_world_structs;

            do {
                n--;
                *(u32 *)&w->unk30[4] &= 0xFEFFFFFF;
                w++;
            } while (n != 0);
        }
    }
    func_80062BD8(0);
    func_8004CFC8(30, 64);
    cur = D_80040FA8;
    D_801ED790.x = D_801ED790.y = 0;
    for (;;) {
        prev = cur;
        cur = func_80016A84(1);
        if ((cur & 0x100) && !(prev & 0x100)) {
            break;
        }
        if (cur & 0x8000) {
            D_801ED790.yaw -= 64;
        } else if (cur & 0x2000) {
            D_801ED790.yaw += 64;
        }
        if (cur & 0x1000) {
            D_801ED790.pitch = func_8008D668(D_801ED790.pitch, -64, -512, 1024);
        } else if (cur & 0x4000) {
            D_801ED790.pitch = func_8008D668(D_801ED790.pitch, 64, -512, 1024);
        }
        if (cur & 0x80) {
            D_801ED790.x = func_8008D668(D_801ED790.x, -200, -10000, 10000);
        } else if (cur & 0x20) {
            D_801ED790.x = func_8008D668(D_801ED790.x, 200, -10000, 10000);
        }
        if (cur & 0x10) {
            D_801ED790.y = func_8008D668(D_801ED790.y, -200, -10000, 10000);
        } else if (cur & 0x40) {
            D_801ED790.y = func_8008D668(D_801ED790.y, 200, -10000, 10000);
        }
        if (cur & 0x8) {
            D_801ED790.z = func_8008D668(D_801ED790.z, -400, 1000, 15000);
        } else if (cur & 0x4) {
            D_801ED790.z = func_8008D668(D_801ED790.z, 400, 1000, 15000);
        }
        if (cur & 0xF0FC) {
            func_8004D020(32, 0x7F00, 64);
        }
        func_8008D3B0();
        if (D_80090850 == 1024) {
            func_8004CFC8(31, 40);
        }
    }
    DrawSync(0);
    func_8004CFC8(30, 64);
}
