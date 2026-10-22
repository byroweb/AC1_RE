#include "ac1/ac1.h"

typedef struct {
    u8 loc[4];
    u8 pad[0x20];
} SndFile;     /* 0x24 */

extern SndFile D_80031784[];
extern char D_80011138[];
extern char D_8001114C[];
extern char D_80011160[];
extern char D_80011174[];
extern char D_80011188[];
extern char D_8001119C[];
extern char D_800111B0[];
extern char D_800111C4[];
extern char D_800111D8[];
extern char D_800111EC[];
extern char D_80011200[];
extern char D_80011214[];
extern char D_80011228[];
extern char D_8001123C[];
extern char D_80011250[];
extern char D_80011264[];
extern char D_80011278[];
extern char D_8001128C[];
extern char D_800112A0[];
extern char D_800112B4[];
extern u8 D_80041720[];
typedef struct { s16 a; s16 pad; s32 b; } Ent8;
extern Ent8 D_80041BD0[];
typedef struct { s16 a; s8 b; u8 pad[7]; } Ent10;
extern Ent10 D_80041DE0[];
extern s16 D_80042A86[];
typedef struct {
    s8 vol[6];
    u16 seq;
    s8 flag;
} SndState;
extern SndState D_80041BB8;
extern void func_800162E8(void *loc, char *name);
extern void func_800228E4(void);

void func_80016ACC(void)
{
    s32 i;
    Ent8 *p8;
    Ent10 *p10;
    s16 *p;
    s8 *v;

    func_800162E8(&D_80031784[0], D_80011138);
    func_800162E8(&D_80031784[1], D_8001114C);
    func_800162E8(&D_80031784[3], D_80011160);
    func_800162E8(&D_80031784[4], D_80011174);
    func_800162E8(&D_80031784[5], D_80011188);
    func_800162E8(&D_80031784[6], D_8001119C);
    func_800162E8(&D_80031784[7], D_800111B0);
    func_800162E8(&D_80031784[8], D_800111C4);
    func_800162E8(&D_80031784[9], D_800111D8);
    func_800162E8(&D_80031784[10], D_800111EC);
    func_800162E8(&D_80031784[11], D_80011200);
    func_800162E8(&D_80031784[12], D_80011214);
    func_800162E8(&D_80031784[13], D_80011228);
    func_800162E8(&D_80031784[14], D_8001123C);
    func_800162E8(&D_80031784[15], D_80011250);
    func_800162E8(&D_80031784[16], D_80011264);
    func_800162E8(&D_80031784[17], D_80011278);
    func_800162E8(&D_80031784[18], D_8001128C);
    func_800162E8(&D_80031784[19], D_800112A0);
    func_800162E8(&D_80031784[20], D_800112B4);
    SsInit();
    SsSetMVol(0, 0);
    SsSetTableSize(D_80041720, 1, 1);
    SsSetTickMode(1);
    SsStart2();
    SsUtSetReverbType(0);
    func_800228E4();
    SsUtSetReverbDepth(0, 0);
    SsSetSerialAttr(0, 0, 1);
    SsSetSerialVol(0, 0x7F, 0x7F);
    SsUtAllKeyOff(0);
    p8 = D_80041BD0;
    i = 65;
    do {
        p8->a = -1;
        p8->b = 0;
        p8++;
        i--;
    } while (i != -1);
    p10 = D_80041DE0;
    i = 22;
    do {
        p10->b = -1;
        i--;
        p10++;
    } while (i != -1);
    p = D_80042A86;
    i = 3;
    do {
        *p = -1;
        i--;
        p += 4;
    } while (i != -1);
    v = &D_80041BB8.vol[2];
    D_80041BB8.vol[5] = 0x7F;
    D_80041BB8.vol[4] = 0x7F;
    D_80041BB8.vol[3] = 0x7F;
    *v = 0x7F;
    D_80041BB8.vol[1] = 0x7F;
    D_80041BB8.vol[0] = 0x7F;
    *v = 0x7F;
    D_80041BB8.seq = 0xFFFF;
    D_80041BB8.flag = 0;
}
