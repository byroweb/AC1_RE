#include "ac1_types.h"
/* exe 0x80016678  size 304  score 7.132  2026-10-18
 * Load "\GG\<name>;1" (prefix in retail .sdata at gp+48, gp = 0x80039C5C;
 * the pinned flags are -G0, so gp is a register variable) into a 2 KB buffer,
 * record its CD location / size in the table entry and copy the u16-counted
 * payload into mxt_head_work. */

typedef struct {
    u8 minute, second, sector, track;
} Func80016678Loc;

typedef struct {
    Func80016678Loc pos;
    u32 size;
    char name[16];
} Func80016678File;

typedef struct {
    u8 *data;
    Func80016678Loc pos;
    u32 size;
} Func80016678Entry;

register u32 *gp_base asm("$28");

typedef struct {
    Func80016678Entry tab[20];
    u8 *heap;   /* +0xF0 = 0x8004A394 */
} Func80016678Mxt;

extern Func80016678Mxt D_8004A2A4;
extern u8 D_80048508[];
extern char D_80039C94[];  /* ";1" */
extern char *func_8002A440(char *dst, char *src);
extern Func80016678File *func_800191C0(Func80016678File *fp, char *name);
extern int printf(char *fmt, ...);
extern void func_800165A8(u8 *dst, Func80016678Loc *pos, s32 size);
extern void func_80015508(u8 *dst, u16 *src, s32 n);

void func_80016678(u16 id, char *name)
{
    u8 buf[0x840];
    Func80016678File file;
    u32 path[16];
    char *p;
    Func80016678Entry *e;

    e = &D_8004A2A4.tab[id];
    __builtin_memcpy(path, gp_base + 12, 5);
    p = (char *)path;
    func_8002A440(p, name);
    func_8002A440(p, D_80039C94);
    if (func_800191C0(&file, p) == 0) {
        printf("CdSearchFile error  FileName=%s\n", p);
    }
    e->pos = file.pos;
    e->size = file.size;
    func_800165A8(buf, &e->pos, 2048);
    e->data = D_8004A2A4.heap;
    D_8004A2A4.heap += *(u16 *)buf * 2 + 2;
    if (D_8004A2A4.heap >= D_80048508) {
        printf("ERROR!!! over mxt_head_work  mxtid=%d  tsu=%d\n", id, *(u16 *)buf + 1);
    }
    func_80015508(e->data, (u16 *)buf + 1, *(u16 *)buf + 1);
}
