/* fdat016 (16 twins): timed trigger update (set up by func_801C76E4).  Once the flag
 * byte is 1, count the delay down; at 0, if func_8006D45C allows, run func_8006D39C on
 * the object and fire func_8008A0B0 with the stats record's arguments. */
#include "ac1/ac1.h"
typedef struct {
    u8 pad0[0x10];
    s32 delay;          /* +0x10 */
} Func801C7624Hit;
typedef struct {
    u8 armed;           /* +0x00 */
    u8 pad1[3];
    u8 a;               /* +0x04 */
    u8 b;               /* +0x05 */
    u16 c;              /* +0x06 */
    u8 d;               /* +0x08 */
    u8 e;               /* +0x09 */
} Func801C7624Stats;
typedef struct {
    u8 pad0[0x2C];
    Func801C7624Hit *on_hit; /* +0x2C */
    u8 pad30[8];
    Func801C7624Stats *stats; /* +0x38 */
} Func801C7624World;
extern s32 func_8006D45C(void);
extern void func_8006D39C(Func801C7624World *w);
extern s32 func_8008A0B0(s32 a, s32 b, s32 c, s32 d, s32 e);
void func_801C7624(Func801C7624World *w)
{
    Func801C7624Hit *h;
    Func801C7624Stats *s;

    if (w->stats->armed == 1) {
        h = w->on_hit;
        if (h->delay == 0) {
            if (func_8006D45C()) {
                func_8006D39C(w);
                s = w->stats;
                func_8008A0B0(s->a, s->b, s->c, s->d, s->e);
            }
        } else {
            h->delay--;
        }
    }
}
