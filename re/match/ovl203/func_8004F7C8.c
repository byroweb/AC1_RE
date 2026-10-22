#include "ac1/ac1.h"

typedef struct Pilot {
    u8 unk000[0x27B];
    u8 ac_no;                   /* +0x27B */
    u8 unk27C;
    u8 parts[8];                /* +0x27D */
} Pilot;

typedef struct {
    u8 unk0[4];
    u32 flags;                  /* +0x04 */
    s32 off;                    /* +0x08 */
} ModelHdr;

typedef struct {
    void *model;                /* 0x8019D968 */
    ModelHdr *tbl[1];           /* 0x8019D96C */
} RenderGlobals;

typedef struct {
    u8 b[0x2C];
} PartSlot;

extern Pilot *D_80039D18;
extern s32 D_8004A394;
extern char D_800895D0[];
typedef struct { u8 *cur; } MdlBuf;
extern MdlBuf D_8019B488;
extern PartSlot D_8019B4A0[];
extern RenderGlobals D_8019D968;
extern void func_800165E4(s32 a, s32 b, u8 *buf);
extern void func_80016678(u16 id, char *name);
extern s32 func_80053008(u8 *buf, s32 a, s32 b, s32 id);
extern s32 func_800531F8(u8 *buf, s32 a, s32 part, s32 id, s32 slot);
extern void func_80072B40(PartSlot *s, u8 *buf, s32 n);

void func_8004F7C8(s32 n)
{
    s32 save;
    s32 id;
    s32 slot;
    ModelHdr *m;

    D_800895D0[1] = D_80039D18->ac_no / 20 + '0';
    D_800895D0[5] = D_80039D18->ac_no / 10 + '0';
    save = D_8004A394;
    D_800895D0[6] = D_80039D18->ac_no % 10 + '0';
    func_80016678(0, D_800895D0);
    func_800165E4(0, 1, D_8019B488.cur);
    func_80072B40(&D_8019B4A0[n], D_8019B488.cur, n);
    id = n + 0x80;
    slot = n * 16;
    D_8019B488.cur += func_80053008(D_8019B488.cur, 0, 0, id);
    if (D_80039D18->parts[0] != 0xFF) {
        D_8019B488.cur += func_800531F8(D_8019B488.cur, 0, D_80039D18->parts[0] + 2, id, slot + 0xE0);
    }
    if (D_80039D18->parts[1] != 0xFF) {
        D_8019B488.cur += func_800531F8(D_8019B488.cur, 0, D_80039D18->parts[1] + 10, id, slot + 0xE1);
    }
    if (D_80039D18->parts[2] != 0xFF) {
        D_8019B488.cur += func_800531F8(D_8019B488.cur, 0, D_80039D18->parts[2] + 44, id, slot + 0xE2);
    }
    if (D_80039D18->parts[3] != 0xFF) {
        D_8019B488.cur += func_800531F8(D_8019B488.cur, 0, D_80039D18->parts[3] + 88, id, slot + 0xE3);
    }
    if (D_80039D18->parts[4] != 0xFF) {
        D_8019B488.cur += func_800531F8(D_8019B488.cur, 0, D_80039D18->parts[4] + 60, id, slot + 0xE4);
    }
    if ((u32)(D_80039D18->parts[3] - 8) >= 8) {
        if (D_80039D18->parts[5] != 0xFF) {
            D_8019B488.cur += func_800531F8(D_8019B488.cur, 0, D_80039D18->parts[5] + 70, id, slot + 0xE5);
        }
    }
    if (D_80039D18->parts[6] != 0xFF) {
        D_8019B488.cur += func_800531F8(D_8019B488.cur, 0, D_80039D18->parts[6] + 108, id, slot + 0xE6);
    }
    if (D_80039D18->parts[7] != 0xFF) {
        D_8019B488.cur += func_800531F8(D_8019B488.cur, 0, D_80039D18->parts[7] + 142, id, slot + 0xE7);
    }
    m = D_8019D968.tbl[id];
    *(u32 *)((u8 *)m + m->off + 4) |= 1;
    D_8004A394 = save;
}
