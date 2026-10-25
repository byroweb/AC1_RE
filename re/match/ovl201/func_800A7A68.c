#include "ac1/ac1.h"

typedef struct { u8 r, g, b; } Rgb3;

extern s32 *func_80050FC4(s32);
extern void func_80051078(void *);
extern void func_8009C964(s32);
extern u8 func_800A8618(void);
extern u8 func_800A869C(s32);
extern s32 func_800309A8(s32 cmd, s32 arg, s32 param);
extern void func_800506EC(s32);
extern void func_80050804();
extern void func_8005C2D0(s32, s32, s32, s32, s32);
extern void func_8005E92C(void);
extern void func_80073DDC(void);
extern void func_800A7928(UiObj *);
extern s32 *D_801BC448;
extern s32 *D_801F07E8;
extern u8 D_80031A58[12];
extern Rgb3 D_80031A64[4][4];
extern u16 D_80039CA0_a[]; /* arrays: may alias, keep retail load/store order */
extern u16 D_80039CA0;
extern u8 D_80039CA2;
extern u8 D_80039CA3;
extern u8 D_800411F8_a[];
#define D_800411F8 D_800411F8_a[0]
extern s8 D_80041594;
extern u8 D_80039C5D;
extern s8 D_80039D2A;
extern RECT D_800BDD14;
extern RECT D_800BDD1C;
extern RECT D_800BDD24;
extern RECT D_800BDD2C;
extern RECT D_8004D2E0;
extern RECT D_8004D2E8;
extern u8 D_8017F498[12];
extern u8 D_8017F4A4[12];

void func_800A7A68(UiObj *o)
{
    RECT r;
    s32 *buf;
    s32 *buf2;
    s32 *q;
    s16 i;
    s16 j;
    u8 s;
    s32 c;

    D_801BC448 = func_80050FC4(2800);
    D_801F07E8 = func_80050FC4(2800);
    func_8009C964(0);
    D_801F07E8[0] = D_80031A58[0];
    {
    s32 t13 = D_80039CA2;
    D_801F07E8[1] = D_80031A58[1];
    D_801F07E8[2] = D_80031A58[2];
    D_801F07E8[3] = D_80031A58[3];
    D_801F07E8[4] = D_80031A58[4];
    D_801F07E8[5] = D_80031A58[5];
    D_801F07E8[6] = D_80031A58[6];
    D_801F07E8[7] = D_80031A58[7];
    D_801F07E8[8] = D_80031A58[8];
    D_801F07E8[9] = D_80031A58[9];
    D_801F07E8[10] = D_80031A58[10];
    {
    s32 a11 = D_80031A58[11];
    s32 b12 = D_80039CA0_a[0];
    D_801F07E8[13] = t13;
    D_801F07E8[12] = b12;
    D_801F07E8[11] = a11;
    }
    D_801F07E8[62] = D_800411F8;
    D_801F07E8[63] = D_80039CA3;
    }
    for (i = 0; i < 4; i++) {
        for (j = 0; j < 4; j++) {
            D_801F07E8[i * 4 + j + 14] = D_80031A64[i][j].r;
            D_801F07E8[i * 4 + j + 30] = D_80031A64[i][j].g;
            D_801F07E8[i * 4 + j + 46] = D_80031A64[i][j].b;
        }
    }
    StoreImage(&D_800BDD14, (void *)(buf = func_80050FC4(0x800)));
    DrawSync(0);
    q = buf;
    for (i = 0; i < 512; i++) {
        D_801F07E8[i + 64] = *q++;
    }
    func_80051078(buf);
    StoreImage(&D_800BDD1C, (void *)(buf2 = func_80050FC4(0x20)));
    DrawSync(0);
    q = buf2;
    for (i = 0; i < 8; i++) {
        D_801F07E8[i + 576] = *q++;
    }
    func_80051078(buf2);
    for (i = 0; i < 12; i++) {
        D_801BC448[i] = 0;
    }
    VSync(3);
    if (func_800A8618() == 0 || func_800A869C(700) == 0) {
        o->update = func_800A7928;
        func_800309A8(2, 0, 0);
        VSync(0);
        func_800309A8(1, 1, 0);
        VSync(0);
        func_800506EC(12);
        obj_find_by_id(11)->kill = 1;
        func_800506EC(8);
        obj_find_by_id(7)->kill = 1;
        func_8005C2D0(28, 170, 264, 32, 11);
        func_8005C2D0(106, 64, 108, 104, 7);
        func_80051078(D_801F07E8);
        func_80051078(D_801BC448);
        return;
    }
    if ((D_80031A58[6] != 0xFF && D_80031A58[6] >= 31) || (D_80031A58[7] != 0xFF && D_80031A58[7] >= 31)) {
        RECT a = D_8004D2E0;
        RECT b = D_8004D2E8;
        MoveImage(&a, 768, 448);
        MoveImage(&b, 0, 493);
        MoveImage(&b, 0, 494);
        DrawSync(0);
    }
    r.x = 704;
    r.y = 256;
    r.w = 64;
    r.h = 256;
    MoveImage(&r, 704, 0);
    r.x = 768;
    r.y = 256;
    r.w = 64;
    r.h = 256;
    MoveImage(&r, 768, 0);
    DrawSync(0);
    r.x = 0;
    r.y = 486;
    r.w = 256;
    r.h = 9;
    MoveImage(&r, 0, 495);
    DrawSync(0);
    r.x = 544;
    r.y = 384;
    r.w = 16;
    r.h = 64;
    MoveImage(&r, 560, 384);
    DrawSync(0);
    r.x = 256;
    r.y = 510;
    r.w = 16;
    r.h = 1;
    MoveImage(&r, 256, 511);
    DrawSync(0);
    LoadImage(&D_800BDD24, (void *)(D_801BC448 + 64));
    DrawSync(0);
    LoadImage(&D_800BDD2C, (void *)(D_801BC448 + 576));
    DrawSync(0);
    D_80041594 = D_801BC448[62];
    for (i = 0; i < 12; i++) {
        D_80031A58[i] = D_801BC448[i];
    }
    D_80039CA0 = D_801BC448[12];
    func_8009C964(1);
    for (i = 0; i < 12; i++) {
        D_80031A58[i] = D_801F07E8[i];
    }
    D_80039CA2 = D_801F07E8[13];
    D_80039CA3 = D_801F07E8[63];
    D_80039CA0 = D_801F07E8[12];
    if (D_801BC448[13] == 0) {
        for (i = 0; i < 4; i++) {
            for (j = 0; j < 4; j++) {
                D_80031A64[i][j].r = D_801BC448[i * 4 + j + 14];
                D_80031A64[i][j].g = D_801BC448[i * 4 + j + 30];
                D_80031A64[i][j].b = D_801BC448[i * 4 + j + 46];
            }
        }
        func_80073DDC();
        for (i = 0; i < 4; i++) {
            for (j = 0; j < 4; j++) {
                D_80031A64[i][j].r = D_801F07E8[i * 4 + j + 14];
                D_80031A64[i][j].g = D_801F07E8[i * 4 + j + 30];
                D_80031A64[i][j].b = D_801F07E8[i * 4 + j + 46];
            }
        }
    } else {
        c = D_801BC448[13] * 4 + 58;
        func_80050804(4, (s16)(c + D_801BC448[63]));
    }
    s = D_801BC448[13];
    for (i = 0; i < 12; i++) {
        D_8017F498[i] = D_801BC448[i];
    }
    func_80051078(D_801F07E8);
    func_80051078(D_801BC448);
    func_80050804(10, D_8017F498[2] + s * 10);
    func_80050804(7, D_8017F498[0] + s * 3);
    func_80050804(9, D_8017F498[3] + s * 16);
    func_80050804(8, D_8017F498[11] + s * 32);
    j = D_8017F498[4];
    if ((u8)j != 0xFF) {
        func_80050804(14, j);
    }
    if (D_8017F498[5] != 0xFF) {
        func_80050804(15, D_8017F498[5]);
    }
    if (D_8017F498[6] != 0xFF) {
        func_80050804(12, D_8017F498[6]);
    }
    if (D_8017F498[7] != 0xFF) {
        func_80050804(13, D_8017F498[7]);
    }
    if (D_8017F498[1] != 0xFF) {
        func_80050804(11, D_8017F498[1]);
    }
    for (i = 0; i < 12; i++) {
        D_8017F4A4[i] = D_80031A58[i];
        D_80031A58[i] = D_8017F498[i];
    }
    func_8005E92C();
    for (i = 0; i < 12; i++) {
        D_80031A58[i] = D_8017F4A4[i];
    }
    D_80039C5D = 0xFF;
    D_80039D2A = 0;
    o->kill = 1;
}
