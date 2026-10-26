/* ovl202: mission script VM, one tick over every thread (g_mission_ctl.vm_thread_tab,
 * 24 bytes each).  A thread whose timer is 0 runs opcodes until 0xFFF0 (wait N
 * ticks) or 0xFFFF (end); then the per-tick work selected by the opcode bit mask
 * (opcodes < 32 set bit op) interpolates position/rotation, drives the mover,
 * spawns effects and copies the result onto the bound entity.
 *
 * Shape levers: the opcode loop is a real for(;;) around one switch (the compare
 * tree and case-body order are cc1's); `neg` is a user variable so loop.c keeps the
 * burst loop's -1 in its own preheader instead of hoisting it out of the thread
 * loop; `ok` keeps landed's refs low so it is the pseudo that spills (sp+96);
 * spawns/ents are locals (spilled, rematerialised as t3) added index-first; the
 * do/while(0) in 0x100B lets k outrank v in local-alloc. */
#include "ac1/ac1.h"
#ifndef NULL
#define NULL 0
#endif

typedef struct VmState {
    SVECTOR pos;        /* +0x00 */
    SVECTOR from;       /* +0x08 from.pad = blob offset of the target */
    SVECTOR rot;        /* +0x10 */
    SVECTOR rfrom;      /* +0x18 rfrom.pad = blob offset of the rotation target */
    u16 flip_ofs;       /* +0x20 */
    u16 pad22;
    u16 fx_ofs;         /* +0x24 */
    u16 burst_ofs;      /* +0x26 */
} VmState;

typedef struct VmThread {
    u16 *pc;            /* +0x00 */
    AcEntity *ent;      /* +0x04 */
    s32 mask;           /* +0x08 */
    u16 mode;           /* +0x0C */
    u16 dur;            /* +0x0E */
    u16 timer;          /* +0x10 0xFFFF = finished */
    u16 pad12;
    VmState *st;        /* +0x14 */
} VmThread;

typedef struct VmNode {
    SVECTOR a;          /* +0x00 */
    SVECTOR b;          /* +0x08 */
    u8 pad10[0x0C];
    u16 f1C;            /* +0x1C */
    u8 pad1e[0x34 - 0x1E];
} VmNode;

typedef struct VmSpawn {
    u8 kind;            /* +0x00 */
    u8 slot;            /* +0x01 */
    u8 pad2[0x12];
    u16 flags;          /* +0x14 */
    u8 pad16[0x2C - 0x16];
} VmSpawn;

typedef struct AcBox {
    u8 pad0[0x2A];
    u16 height;         /* +0x2A */
    u8 pad2c[2];
    s16 floor_y;        /* +0x2E */
} AcBox;

typedef struct VmStats {
    u8 pad0[6];
    u8 f6;
} VmStats;

typedef struct { u32 flags; } VmFlags;
extern VmFlags D_8019F524;
extern VmNode D_80198B08[];
extern VmSpawn D_8019FAB8[];

void LoadAverageShort12(SVECTOR *v0, SVECTOR *v1, long p0, long p1, SVECTOR *v2);
int rand(void);
void func_8004C318(u16);
void func_8004C98C(u16, s32, VmState *, SVECTOR *, u16, u16, u16, s16, s16);
void func_8004DB18(u16);
void func_8004E98C(AcMover *, u16, u16, u16);
void func_8004EC4C(AcMover *);
void func_8004ECA8(AcMover *);
void func_8004ED5C(AcMover *);
s32 func_8004F024(AcMover *, AcPos *);
void func_800529F0(s32, s32, u16);
void func_80073104(void);
void func_80073170(u16);
void func_800736AC(void);
void func_80078250(AcEntity *, u16, u16, s32, s32, s32, s32, s32);
void func_8007832C(AcEntity *, u16, u16);
void func_80078598(AcEntity *);
void func_80078CFC(VmSpawn *);
void func_80078D78(AcEntity *, VmSpawn *);
void func_800796F4(void);
void func_8008792C(void);
void func_8008B320(VmThread *);

void func_8008B380(void) {
    SVECTOR saved;
    SVECTOR tmp1;
    SVECTOR tmp2;
    SVECTOR delta;
    u8 *blob;
    VmThread *rec;
    s32 i;
    VmSpawn *spawns;
    AcEntity *ents;
    VmState *st;
    u16 *pc;
    u16 op;
    s32 mask;
    s32 elapsed;
    s32 t;
    s32 u;

    i = g_mission_ctl.vm_threads;
    rec = g_mission_ctl.vm_thread_tab;
    blob = g_mission_ctl.vm_blob;
    spawns = D_8019FAB8;
    ents = g_ac_entities;
    while (--i != -1) {
        if (rec->timer != 0xFFFF) {
            st = rec->st;
            if (rec->timer == 0) {
                pc = rec->pc;
                rec->mask = 0;
                for (;;) {
                    op = *pc++;
                    if (op < 32) {
                        rec->mask |= 1 << op;
                    }
                    switch (op) {
                    case 0x0:
                        st->from = st->pos;
                        st->from.pad = (u8 *)pc - blob;
                        pc += 3;
                        break;
                    case 0x2:
                        st->rfrom = st->rot;
                        st->rfrom.pad = (u8 *)pc - blob;
                        pc += 3;
                        break;
                    case 0x8:
                    case 0x9: {
                        u16 a, b, c;
                        a = *pc++;
                        b = *pc++;
                        c = *pc++;
                        func_8004E98C(&rec->ent->mover, a, b, c);
                        break;
                    }
                    case 0xA: {
                        AcEntity *e = rec->ent;
                        st->flip_ofs = (u8 *)pc - blob;
                        pc++;
                        e->mover.weapon_slot = *pc++;
                        e->mover.fire_timer = 0;
                        *(u16 *)((u8 *)e + 0x7C) = *pc++;
                        *(u16 *)((u8 *)e + 0x76) = *pc++;
                        *(u16 *)((u8 *)e + 0x78) = *pc++;
                        *(u16 *)((u8 *)e + 0x7A) = *pc++;
                        e->flags = 0;
                        break;
                    }
                    case 0xB:
                        func_8008B320(rec);
                        break;
                    case 0xE:
                        st->fx_ofs = (u8 *)pc - blob;
                        pc += 7;
                        break;
                    case 0xF:
                        st->burst_ofs = (u8 *)pc - blob;
                        pc += *pc * 2 + 1;
                        break;
                    case 0x1000:
                        rec->ent = (AcEntity *)&((VmThread *)g_mission_ctl.vm_thread_tab)[*pc++];
                        break;
                    case 0x1001: {
                        u16 v = *pc++;
                        st->pos.vx = v;
                        if (v == 0x8000) {
                            st->pos = D_80198B08[*pc].a;
                            pc += 2;
                        } else {
                            st->pos.vy = *pc++;
                            st->pos.vz = *pc++;
                        }
                        break;
                    }
                    case 0x1002:
                        st->rot.vx = *pc++;
                        if ((u16)st->pos.vx == 0x8000) {
                            st->pos = D_80198B08[*pc].b;
                            pc += 2;
                        } else {
                            st->rot.vy = *pc++;
                            st->rot.vz = *pc++;
                        }
                        break;
                    case 0x1003: {
                        VmSpawn *s = (VmSpawn *)(*pc++ * sizeof(VmSpawn) + (u32)spawns);
                        if (s->kind != 1) {
                            func_80078CFC(s);
                        }
                        s->flags &= ~0x40;
                        rec->ent = (AcEntity *)(s->slot * sizeof(AcEntity) + (u32)ents);
                        st->pos = *(SVECTOR *)&rec->ent->pos;
                        st->rot = *(SVECTOR *)&rec->ent->unk10;
                        break;
                    }
                    case 0x100C: {
                        VmSpawn *s = (VmSpawn *)(*pc++ * sizeof(VmSpawn) + (u32)spawns);
                        s->flags |= 0x40;
                        if (s->kind == 1) {
                            func_80078D78((AcEntity *)(s->slot * sizeof(AcEntity) + (u32)ents), s);
                        }
                        break;
                    }
                    case 0x1004:
                        func_800529F0(0, 0, *pc++);
                        break;
                    case 0x1005: {
                        AcEntity *e = ents;
                        s32 n = 16;
                        do {
                            if (e->def != NULL) {
                                *(u16 *)&e->unk28[0xA] |= 0x8000;
                            }
                            e++;
                        } while (--n != 0);
                        break;
                    }
                    case 0x1006: {
                        u16 *d = (u16 *)&st->rot.vy;
                        s32 j = 0;
                        s32 n = 256;
                        st->rot.vx |= 1;
                        do {
                            *d++ = g_proj_pool[j].free;
                            if (g_proj_pool[j].free >= 0) {
                                g_proj_pool[j].free = -2;
                            }
                            j++;
                        } while (--n != 0);
                        break;
                    }
                    case 0x1007:
                        func_8004DB18(*pc++);
                        break;
                    case 0x1008: {
                        s32 n, k;
                        u16 v;
                        n = *pc++;
                        k = *pc++;
                        v = *pc++;
                        while (--n != -1) {
                            ((VmStats *)g_world_structs[k].stats)->f6 = v;
                            k++;
                        }
                        break;
                    }
                    case 0x1009: {
                        s32 n, k, v;
                        WorldStruct *w;
                        n = *pc++;
                        k = *pc++;
                        v = *pc++;
                        while (--n != -1) {
                            w = &g_world_structs[k];
                            if (w->on_hit != NULL) {
                                void (*fn)(WorldStruct *, s32, s32) = ((void (**)(WorldStruct *, s32, s32))w->on_hit)[1];
                                if (fn != NULL) {
                                    fn(w, v + 0x8200, 0);
                                }
                            }
                            k++;
                        }
                        break;
                    }
                    case 0x100A:
                        func_8004C318(*pc++);
                        break;
                    case 0x100B: {
                        s32 k, v;
                        k = *pc++;
                        v = *pc++;
                        do {
                            D_80198B08[k].f1C = v;
                        } while (0);
                        func_80073170(v);
                        break;
                    }
                    case 0xFFF0:
                        rec->timer = rec->dur = *pc++;
                        rec->pc = pc;
                        goto run;
                    case 0xFFFF:
                        rec->timer = 0xFFFF;
                        if (rec->mode == 1) {
                            if (st->rot.vx & 1) {
                                u16 *d = (u16 *)&st->rot.vy;
                                Projectile *q;
                                s32 n;
                                st->rot.vx |= 1;
                                q = g_proj_pool;
                                n = 256;
                                do {
                                    q->free = *d++;
                                    q++;
                                } while (--n != 0);
                            }
                        }
                        goto next;
                    }
                }
            }
        run:
            mask = rec->mask;
            saved = st->pos;
            elapsed = rec->dur - rec->timer;
            t = (elapsed << 12) / rec->dur;
            u = 0x1000 - t;
            if (mask & 1) {
                s16 *p = (s16 *)(blob + st->from.pad);
                u16 v = p[0];
                if (v == 0x8000) {
                    LoadAverageShort12(&D_80198B08[p[1]].a, &st->from, t, u, &st->pos);
                } else {
                    tmp1.vx = v;
                    tmp1.vy = p[1];
                    tmp1.vz = p[2];
                    LoadAverageShort12(&tmp1, &st->from, t, u, &st->pos);
                }
            }
            if (mask & 4) {
                s16 *p = (s16 *)(blob + st->rfrom.pad);
                u16 v = p[0];
                if (v == 0x8000) {
                    LoadAverageShort12(&D_80198B08[p[1]].b, &st->rfrom, t, u, &st->rot);
                } else {
                    tmp2.vx = v;
                    tmp2.vy = p[1];
                    tmp2.vz = p[2];
                    LoadAverageShort12(&tmp2, &st->rfrom, t, u, &st->rot);
                }
            }
            if (mask & 0x100) {
                func_8004EC4C(&rec->ent->mover);
            }
            if (mask & 0x200) {
                func_8004ECA8(&rec->ent->mover);
            }
            if (mask & 0x400) {
                AcEntity *e = rec->ent;
                if (e->mover.weapon_slot != 0xFF) {
                    func_8004ED5C(&e->mover);
                }
                e->flags ^= *(u16 *)(blob + st->flip_ofs);
            }
            if (mask & 0x800) {
                AcEntity *e = rec->ent;
                if (func_8004F024(&e->mover, &e->pos) == 0) {
                    func_8008B320(rec);
                }
                st->pos.vx = e->pos.x;
                st->pos.vy = e->pos.y;
                st->pos.vz = e->pos.z;
            }
            if (mask & 0x4000) {
                u16 *p;
                s16 a4, a5;
                p = (u16 *)(blob + st->fx_ofs);
                delta.vx = st->pos.vx - saved.vx;
                delta.vy = st->pos.vy - saved.vy;
                delta.vz = st->pos.vz - saved.vz;
                a4 = p[4];
                a5 = p[5];
                if (p[6] == 0) {
                    func_8004C98C(p[0], i + 0x5000, st, &delta, p[1], p[2], p[3], a4, a5);
                } else if (elapsed % p[6] == 0) {
                    func_8004C98C(p[0], 0, st, &delta, p[1], p[2], p[3], a4, a5);
                }
            }
            if (mask & 0x8000) {
                s32 landed;
                AcEntity *e;
                u16 *p;
                s32 n;
                s32 first;
                s32 neg;
                s16 top;
                landed = 0;
                e = rec->ent;
                top = ((AcBox *)e)->floor_y;
                p = (u16 *)(blob + st->burst_ofs);
                first = 1;
                if (top <= e->pos.y) {
                    s32 ok = top >= e->pos.y - *(u16 *)((u8 *)e + 0x2A);
                    landed = ok;
                }
                n = *p++;
                while (--n != -1) {
                    if (landed) {
                        if (first) {
                            func_80078250(e, p[0], p[1], 0, 160, (rand() & 0x7F) + 300, 66, i + 0x5100);
                        } else {
                            neg = -1;
                            func_80078250(e, p[0], p[1], 0, 160, (rand() & 0x7F) + 300, neg, 0);
                        }
                    } else {
                        func_8007832C(e, p[0], p[1]);
                    }
                    first = 0;
                    p += 2;
                }
            }
            if (mask & 0x10000) {
                func_8008792C();
            }
            if (mask & 0x20000) {
                func_800736AC();
            }
            if (mask & 0x40000) {
                func_80073104();
            }
            if (mask & 0x80000) {
                D_8019F524.flags |= 0x400;
                func_800796F4();
            }
            if (rec->mode == 2) {
                AcEntity *e = rec->ent;
                if (e != NULL) {
                    e->pos.x = st->pos.vx;
                    e->pos.y = st->pos.vy;
                    e->pos.z = st->pos.vz;
                    *(s16 *)e->unk10 = st->rot.vx;
                    e->yaw = st->rot.vy;
                    *(s16 *)e->unk14 = st->rot.vz;
                    e->section = world_section_find(e->section, &e->pos);
                    func_80078598(e);
                }
            }
            rec->timer--;
        }
    next:
        rec++;
    }
}
