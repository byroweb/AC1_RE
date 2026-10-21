#include "ac1/ac1.h"
#include "ac1/ovl202_t42.h"

typedef struct {                /* 0x34: map marker */
    SVECTOR pos;                /* +0x00 pos.pad = marker kind (-1 = none) */
    u8 pad08[6];
    s16 ws;                     /* +0x0E g_world_structs index */
    u8 pad10[0x12];
    char name[0x12];            /* +0x22 */
} MapMark;

typedef struct {                /* 0x18 */
    u8 pad00[0x0E];
    s16 type;                   /* +0x0E 0xFE = labelled */
    u8 pad10[8];
} MarkKind;

typedef struct {
    char s[16];
    s16 end;
} Label16;

extern MapMark D_80198B08[512];
extern MarkKind D_80198800[];
extern void func_8008C6FC(SVECTOR *v);
extern void func_8008C7BC(SVECTOR *in, ScrPt *out, s32 n);
extern void func_8008C890(ScrPt *r, Label16 *a1, s32 idx, s32 a3);
extern void func_80015508(void *dst, void *src, s32 n);

void func_8008CB5C(void) {
    Label16 lab;
    T42Spad *sp = SPAD;
    MapMark *m;
    WorldStruct *w;
    s32 i;

    m = D_80198B08;
    for (i = 512; i != 0; i--) {
        if (m->pos.pad != -1 && D_80198800[m->pos.pad].type == 0xFE) {
            w = &g_world_structs[m->ws];
            if ((*(u32 *)&w->unk30[0] & 0x10000) || (*(u32 *)&w->unk30[4] & 0x01000000)) {
                func_8008C6FC(&m->pos);
                func_8008C7BC(&sp->v, sp->pts, 1);
                if (sp->pts[0].clip != 255) {
                    func_80015508(&lab, m->name, 8);
                    lab.end = 0;
                    func_8008C890(sp->pts, &lab, 7, 8);
                }
            }
        }
        m++;
    }
}
