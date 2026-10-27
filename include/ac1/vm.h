/* include/ac1/vm.h - overlay 202 mission script VM threads, the map-object table and
 * the spawn-template table (MISSION_SCRIPT_VM.md, MISSION_STAGES.md,
 * DESTRUCTIBLE_OBJECTS.md).  Opt-in: include after ac1/ac1.h.
 *
 * Layouts merged from the matched sources (tools/match/structsurvey.py merge):
 *   VmThread / VmState  func_8008B380 (VM tick), func_8008C178 (vm_load_threads)
 *   MapObj   0x80198B08, 26 sources: ovl202 func_8006131C, func_800731E8, func_80078F00,
 *            func_8007909C, func_800835E8, func_8008B380, func_8008CB5C, fdat000
 *            func_801C6ED8 and twins, fdat004/012/016 Slot34, ...
 *   SpawnTmpl 0x8019FAB8, 19 sources: ovl202 func_80073B74 (fills it), func_8008B380,
 *            fdat000 func_801C5428, fdat016 func_801C804C, fdat098 func_801C662C, ...
 */
#ifndef AC1_VM_H
#define AC1_VM_H

#include "types.h"

/* Per-thread work area of kind 2 (40 bytes, allocated by vm_load_threads). */
typedef struct VmState {
    SVECTOR pos;                /* +0x00 interpolated position */
    SVECTOR from;               /* +0x08 from.pad = blob offset of the target */
    SVECTOR rot;                /* +0x10 */
    SVECTOR rfrom;              /* +0x18 rfrom.pad = blob offset of the rotation target */
    u16 flip_ofs;               /* +0x20 */
    u16 pad22;
    u16 fx_ofs;                 /* +0x24 */
    u16 burst_ofs;              /* +0x26 */
} VmState;
AC1_ASSERT_SIZE(VmState, 0x28);

/* g_mission_ctl.vm_thread_tab: vm_threads records, bump-allocated from the mission
 * heap.  A thread runs opcodes until 0xFFF0 (wait) or 0xFFFF (end). */
typedef struct VmThread {
    u16 *pc;                    /* +0x00 next opcode (blob word after the kind) */
    struct AcEntity *ent;       /* +0x04 bound unit (0 at load) */
    s32 mask;                   /* +0x08 per-tick work bits (opcodes < 32 set bit op) */
    u16 mode;                   /* +0x0C thread kind from the blob: 0, 1 or 2 picks the work
                                 *       area size 16, 530 or 40 (VmState) */
    u16 dur;                    /* +0x0E */
    u16 timer;                  /* +0x10 wait ticks; 0xFFFF = finished (0 at load) */
    u16 pad12;
    VmState *st;                /* +0x14 work area (u8 * for kinds 0 and 1) */
} VmThread;
AC1_ASSERT_SIZE(VmThread, 0x18);

/* 0x80198B08: 512 map-object records ("52-B records", the destructible class with a
 * handler at +0x14, node boxes the enemy ACs walk).  Different object kinds read
 * some words differently; those readings are noted per field. */
typedef struct MapObj {
    SVECTOR pos;                /* +0x00 pos.pad (+0x06) = kind; fdat matches call it kind,
                                 *       state or slot */
    SVECTOR rot;                /* +0x08 rot.pad (+0x0E) = an s16 the fdat matches call bit */
    union {
        SVECTOR scl;            /* +0x10 func_8006131C (drawn objects) */
        struct {
            s32 unk10;
            void *handler;      /* +0x14 record whose +0 / +4 are methods (func_800835E8 hit,
                                 *       VM opcode 0x0011 -> func_800736AC calls +0) */
        } obj;
    } u10;
    s8 zbias;                   /* +0x18 */
    u8 unk19;
    s16 unk1a;
    u16 flags;                  /* +0x1C (5 sources) */
    u16 unk1e;
    u16 unk20;
    s8 lo[3];                   /* +0x22 box min (node box, bmin).  Map markers read
                                 *       +0x22..+0x33 as an 18-byte name (func_8008CB5C) */
    s8 hi[3];                   /* +0x25 box max */
    union {
        u8 id[2];               /* +0x28 neighbour nodes */
        u16 any;                /* tested as one halfword (func_801C6B30, func_801C6C00) */
    } kids;
    u8 group;                   /* +0x2A */
    u8 anim;                    /* +0x2B */
    u8 type;                    /* +0x2C */
    u8 sticky;                  /* +0x2D */
    u8 weight;                  /* +0x2E */
    u8 bonus;                   /* +0x2F (+0x2C..+0x2F named in fdat000 func_801C6ED8 only) */
    u8 unk30[4];
} MapObj;
AC1_ASSERT_SIZE(MapObj, 0x34);

/* 0x8019FAB8: 256 instance templates, filled from the mission's spawn records by
 * func_80073B74 (chunk 12).  AcEntity.def points at one. */
typedef struct SpawnTmpl {
    u8 state;                   /* +0x00 0 = idle; the matches call it state, st or kind */
    u8 slot;                    /* +0x01 index into g_ac_entities */
    u16 hp;                     /* +0x02 copy of hw11 at fill time; hits do not change it
                                 *       (LIVE_CHECKLIST S-5) */
    s16 x, y, z;                /* +0x04 hw0..hw2 position */
    s16 kind;                   /* +0x0A hw3: geometry block + dispatch type; -1 = none.  Some
                                 *       fdat matches call it link */
    u16 hw4, hw5, hw6;          /* +0x0C */
    s16 section;                /* +0x12 hw7 world section (MISSION_DAMAGE_EVENTS) */
    u16 flags;                  /* +0x14 hw8 */
    u16 obj;                    /* +0x16 hw9 map-object index */
    u16 hw10;                   /* +0x18 */
    u16 hp_seed;                /* +0x1A hw11 unit HP (copied to AcEntity +0x160) */
    u16 hw12, hw13, hw14;       /* +0x1C */
    u16 wid;                    /* +0x22 hw15 index into g_world_structs */
    u16 hw16, hw17, hw18, hw19; /* +0x24 */
} SpawnTmpl;
AC1_ASSERT_SIZE(SpawnTmpl, 0x2C);

/* Retail often addresses the templates and g_ac_entities (0x801A26B8) from one base;
 * nine fdat matches declare this block to get that form. */
typedef struct SpawnBlock {
    SpawnTmpl tmpl[256];        /* +0x0000 0x8019FAB8 */
    struct AcEntity ents[16];   /* +0x2C00 0x801A26B8 g_ac_entities */
} SpawnBlock;
AC1_ASSERT_SIZE(SpawnBlock, 0x4300);

/* --- symbols: GENERATED by tools/match/ctx.py gen from symbols.tsv; do not edit --- */
extern MapObj g_map_objs[512];                           /* 0x80198B08 */
extern MapObj *g_map_obj_list[128];                      /* 0x8019F308 */
extern SpawnTmpl g_spawn_tmpl[256];                      /* 0x8019FAB8 */
/* --- end of generated symbols --- */

#endif
