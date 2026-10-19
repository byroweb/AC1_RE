#include "ac1_types.h"
typedef struct {
    u8 pad[1];
    u8 active;      /* +0x01 */
    u8 pad2[0x74 - 2];
    void (*update)(void);   /* +0x74 */
    u8 pad3[0x7E - 0x78];
    u8 state;       /* +0x7E */
} Func800AD868Obj;
typedef struct {
    u8 pad[1];
    u8 active;      /* +0x01 */
} Func800AD868Sub;
extern u8 D_8005CC48[];
extern s32 D_801A2574;
extern u8 D_80039CAB;
extern void func_80051450(s32 a, s32 b);
extern Func800AD868Sub *func_80050078(void *key);
extern void func_800506EC(s32 kind);
extern Func800AD868Sub *func_8005015C(s32 kind);
extern void func_8005B2CC(void);
extern void func_800AD96C(Func800AD868Obj *o);
extern void func_80050E94(void);
extern void func_80050C18(void);
extern void func_800A2D0C(s32 a);
extern void func_800A9AFC(void);
void func_800AD868(Func800AD868Obj *o)
{
    if (o->state == 0) {
        func_80051450(4, 5);
        func_80050078(D_8005CC48)->active = 1;
        func_800506EC(4);
        func_8005015C(3)->active = 1;
        func_800506EC(6);
        func_8005015C(5)->active = 1;
        func_800506EC(8);
        func_8005015C(7)->active = 1;
        func_800506EC(2);
        func_8005015C(1)->active = 1;
        D_801A2574 = 0;
        o->active = 1;
        D_80039CAB = 0;
        func_8005B2CC();
        func_800AD96C(o);
        func_80050E94();
        func_80050C18();
        func_800A2D0C(1);
    } else if (o->state == 1) {
        o->update = func_800A9AFC;
    }
}
