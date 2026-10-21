#include "ac1/ac1.h"
/* ovl202 0x8006F08C - walk the polygon buckets still set in mask and append a hit
 * record for every polygon func_8006EE8C hits (mode 0: corner pick, 1: bounce) */
typedef struct HitRec {
    CollPoly *poly;             /* +0x00 */
    WorldStruct *ws;            /* +0x04 */
    CollMesh *mesh;             /* +0x08 */
    SVECTOR d;                  /* +0x0C */
    SVECTOR pt;                 /* +0x14 */
} HitRec;
typedef struct Probe {
    u8 unk00[0x48];
    HitRec *out;                /* +0x48 next hit record */
    s32 mode;                   /* +0x4C */
    SVECTOR res;                /* +0x50 */
    u8 unk58[0x34];
    s32 group;                  /* +0x8C polygons per bucket bit */
    WorldStruct *ws;            /* +0x90 */
    CollMesh *mesh;             /* +0x94 */
    u8 unk98[4];
    CollPoly poly;              /* +0x9C copy of the polygon under test */
    CollPoly *cur;              /* +0xB0 */
    CollPoly *end;              /* +0xB4 */
    CollPoly *group_end;        /* +0xB8 */
    u32 mask;                   /* +0xBC buckets still to test */
} Probe;
s32 func_8006EE8C(Probe *o);
void func_8006F300(s32 code, Probe *o);
void func_8006F3E0(Probe *o);

void func_8006F08C(Probe *o)
{
    s32 r;

    do {
        if (o->mask & 1) {
            o->group_end = o->cur + o->group;
            if (o->end < o->group_end) {
                o->group_end = o->end;
            }
            do {
                o->poly = *o->cur;
                r = func_8006EE8C(o);
                if (r != 0) {
                    if (o->mode == 0) {
                        func_8006F300(r, o);
copy:
                        o->out->d = o->res;
fields:
                        o->out->ws = o->ws;
                        o->out->poly = o->cur;
                        o->out->mesh = o->mesh;
                        o->out++;
                    } else if (o->mode == 1) {
                        func_8006F3E0(o);
                        goto copy;
                    } else {
                        goto fields;
                    }
                }
                o->cur++;
            } while (o->cur < o->group_end);
        } else {
            o->cur += o->group;
        }
        o->mask >>= 1;
    } while (o->cur < o->end);
}
