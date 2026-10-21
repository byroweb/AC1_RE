/* include/ac1/uiobj.h - overlay 201 (front end) object pool.
 *
 * Every front-end / garage / menu object is a 0xD0-byte UiObj in the pool
 * g_ui_pool (0x801A25E8, 510 slots, end bound 0x801BC448).  ui_obj_alloc
 * (0x800501BC) takes a kind 0..21; each kind owns a fixed run of slots (the
 * even kinds 2..20 are the g_ui_pool_kN arrays in symbols.tsv).
 *
 * Field meanings come from the per-frame loop in ovl201 (0x8009BCD0..):
 *   - every used slot gets update(o) (0x8009BD20, lw 0x74);
 *   - used slots go into the active list g_ui_active (0x801BC5EC, end
 *     pointer g_ui_active_end at 0x801BC5E8);
 *   - kill != 0: destroy(o) is called, then kill and used are cleared
 *     (0x8009BDB0..0x8009BDE4);
 *   - parent != 0 and parent->used == 0: same retirement (0x8009BE34..);
 *   - has_xform: RotMatrix(&rot, &mat) then TransMatrix(&mat, &pos);
 *   - parent: CompMatrix-style compose of parent->mat into mat (0x80027AE4);
 *   - attached: compose attach->mat into mat;
 *   - visible: drawn; view picks the 32-byte matrix at 0x800B8D84 + view*32
 *     (view 2 = none), light picks the one at 0x800B8DC4 + light*32,
 *     scaled applies scale (0x800280A4), then render_type is dispatched
 *     (COMBAT_PHYSICS.md section 1 lists the render types).
 *
 * Lookups: obj_find_by_handle(ctor) returns the first used, not-killed
 * object whose ctor matches; obj_find_by_id(id) matches the u16 at +0x08.
 * Both return &g_ui_none (0x801BC4F8, a spare UiObj outside the pool) when
 * nothing matches, so callers can store through the result unchecked.
 *
 * Bytes 0x00..0x7B are common to every class.  0x7C..0xCF is a per-class
 * work area (UiWork): the same offsets hold a pointer in list rows and
 * bytes in sequencers, so each class layout is a view of the union.
 */
#ifndef AC1_UIOBJ_H
#define AC1_UIOBJ_H

#include "types.h"

struct UiObj;

/* g_ui_views (0x801A2320): 10 x 0x38, refreshed by the frame loop with
 * RotMatrix(&rot, &mat) and TransMatrix(&mat, &pos).  ui_obj_alloc points
 * a new object's attach field at this array. */
typedef struct UiView {
    SVECTOR rot;                /* +0x00 */
    VECTOR pos;                 /* +0x08 */
    MATRIX mat;                 /* +0x18 */
} UiView;
AC1_ASSERT_SIZE(UiView, 0x38);

/* Per-class work area at UiObj +0x7C.  Offsets in comments are UiObj
 * offsets.  Add a view here when a matched function proves one. */
typedef union UiWork {
    u8 raw[0x54];               /* +0x7C..+0xCF */
    struct {                    /* list rows: pos.vy = base + f(*src) */
        void *src;              /* +0x7C the byte or object the row reads */
        s16 base;               /* +0x80 row origin */
    } list;
    struct {                    /* sequencer objects (func_800AD868) */
        u8 pad7c[2];
        u8 state;               /* +0x7E step index */
    } seq;
    struct {                    /* object another row's list.src points at */
        u8 pad7c[0x0E];
        s8 index;               /* +0x8A selected row */
    } sel;
    struct {                    /* six-item ring (func_8004F024, func_8004EC24) */
        u8 pad7c[4];
        struct UiObj *item[6];  /* +0x80 */
        struct UiObj *lead;     /* +0x98 its update step gates func_8004EC24 */
        u8 pad9c[0x12];
        s8 front;               /* +0xAE item at the front */
    } ring;
    struct {                    /* sliding bar (func_8004ECAC, func_8004EDC0) */
        u8 pad7c[0x20];
        struct UiObj *bar;      /* +0x9C */
        u8 padA0[0xC];
        s16 stop;               /* +0xAC x where the slide ends */
        s8 mode;                /* +0xAE 0..5: bar image and speed */
    } slide;
} UiWork;

typedef struct UiObj {
    u8 used;                    /* +0x00 slot taken; free slots are skipped */
    u8 kill;                    /* +0x01 1 = retire this frame (destroy is called) */
    u8 visible;                 /* +0x02 draw it */
    u8 attached;                /* +0x03 compose attach->mat */
    u8 scaled;                  /* +0x04 apply scale */
    u8 has_xform;               /* +0x05 rebuild mat from rot/pos each frame */
    u8 view;                    /* +0x06 view-matrix index, 2 = none */
    u8 pad07;
    u16 id;                     /* +0x08 key for obj_find_by_id */
    u16 render_type;            /* +0x0A renderer selector (6 = draw_string) */
    void *res;                  /* +0x0C render resource (string for type 6) */
    u8 unk10[4];
    s16 unk14;                  /* +0x14 */
    u16 light;                  /* +0x16 light-matrix index */
    u8 unk18[8];
    MATRIX mat;                 /* +0x20 local-to-world */
    SVECTOR rot;                /* +0x40 */
    VECTOR pos;                 /* +0x48 */
    VECTOR scale;               /* +0x58 */
    UiView *attach;             /* +0x68 */
    struct UiObj *parent;       /* +0x6C retired when parent->used == 0 */
    void *ctor;                 /* +0x70 the constructor that built it; key for obj_find_by_handle */
    void (*update)(struct UiObj *);     /* +0x74 called every frame */
    void (*destroy)(struct UiObj *);    /* +0x78 called once on kill */
    UiWork w;                   /* +0x7C per-class area */
} UiObj;
AC1_ASSERT_SIZE(UiObj, 0xD0);

typedef struct UiNameList {     /* *0x801F5094: a scrolling name list (func_8008B094) */
    s8 count;                   /* +0x000 entries */
    s8 rows;                    /* +0x001 rows on screen */
    u8 unk002[0x21B];
    char name[1][34];           /* +0x21D row texts, shifted one place per scroll step */
} UiNameList;

typedef struct UiListState {    /* 0x801F37C8 */
    u8 unk0;
    s8 cursor;                  /* +0x01 selected entry */
    u8 scroll;                  /* +0x02 scroll position, +/-1 per shifted row */
} UiListState;
AC1_ASSERT_SIZE(UiListState, 3);

#endif
