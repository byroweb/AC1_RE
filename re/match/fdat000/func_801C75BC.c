/* fdat000 (mission library, entry 0 @ 0x801C4B40; 31 twins, group g0007): AI event
 * handler, func(o, event, ...).  1..3: remember the attacker (varargs pointer) and
 * sometimes switch sub-state; 4: land (clear vel Y and kind); 8: react to a hit between
 * two points; 9: step to the child node nearer to the area node; 0 (and after 9): pick
 * the state (def+0xD8, 0x58 bytes each, weights from def+0xCF), its sub-state, an action
 * (801C737C) and the weapon (def+0x4E) by weighted rolls (func_80014DFC) when their
 * timers run out.  Always returns 0.
 * The dispatch is an if/goto chain shaped like GCC's case tree: a switch on these six
 * case values gets a jump table from this cc1 (count 6 >= CASE_VALUES_THRESHOLD), so
 * retail's compare tree cannot come from one switch.  Events 4 and 1..3 each compute
 * their own sub pointer (cross-jumped into one tail), the node ids are u8 copies with
 * kids[0] stored on two paths, and *c = *w is written once per arm: each extra use
 * moves the allocation priority to retail's registers. */
#include "ac1/ac1.h"
typedef struct {
    u8 weights[0x0C];   /* +0x00 */
    u16 hold[(0x24 - 0x0C) / 2]; /* +0x0C per choice, in timer units */
} Func801C75BCSub;
typedef struct {
    s32 until;          /* +0x00 */
    s32 until_cur;      /* +0x04 */
    s32 until_sub;      /* +0x08 */
    u16 hold;           /* +0x0C */
    Func801C75BCSub sub[2]; /* +0x0E */
    u8 pad56[2];
} Func801C75BCState;
typedef struct {
    u8 pad0[0x0D];
    u8 weight;          /* +0x0D */
    u8 secs;            /* +0x0E */
    u8 pad0f[0x28 - 0x0F];
} Func801C75BCWeapon;
typedef struct {
    u8 pad0[0x4C];
    u16 n_weapons;      /* +0x4C */
    Func801C75BCWeapon weapons[3]; /* +0x4E */
    u8 padc6[0xC8 - 0xC6];
    u16 n_states;       /* +0xC8 */
    u16 n_subs;         /* +0xCA */
    u16 n_actions;      /* +0xCC */
    u8 hit_kind;        /* +0xCE */
    u8 weights[9];      /* +0xCF */
    Func801C75BCState states[7]; /* +0xD8 */
    u8 pad340[0x397 - 0x340];
    u8 actions[0x3B8 - 0x397]; /* +0x397, indexed from 1 */
    u8 action_w[8][33]; /* +0x3B8 */
} Func801C75BCDef;
typedef struct {
    AcPos pos;          /* +0x00 */
    u8 pad08[0x28 - 0x08];
    u8 kids[2];         /* +0x28 */
    u8 pad2a;
    u8 anim;            /* +0x2B */
    u8 pad2c[0x34 - 0x2C];
} Func801C75BCNode;
typedef struct {
    u8 pad0[0x3C];
    u8 dir;             /* +0x3C */
    u8 kind;            /* +0x3D */
    u8 pad3e[2];
    u8 ai_mode;         /* +0x40 */
    u8 pad41;
    u8 state;           /* +0x42 */
    u8 pad43;
    u16 timer;          /* +0x44 */
    u8 pad46[2];
    void *target;       /* +0x48 */
    u8 pad4c[0xCC - 0x4C];
    u16 vel[3];         /* +0xCC */
    u8 padd2[0xFE - 0xD2];
    u8 group;           /* +0xFE */
    u8 padff[0x108 - 0xFF];
    Func801C75BCDef *def; /* +0x108 */
    u8 pad10c[0x124 - 0x10C];
    s16 area;           /* +0x124 */
    s16 node;           /* +0x126 */
    u8 pad128[2];
    u16 sub_timer;      /* +0x12A */
    u16 weapon_timer;   /* +0x12C */
    u8 pad12e[2];
    u8 choice;          /* +0x130 */
    u8 pad131;
    u8 weapon;          /* +0x132 */
    u8 pad133[3];
    u8 sub;             /* +0x136 */
} Func801C75BCObj;
extern Func801C75BCNode D_80198B08[];
extern s32 func_80073714(Func801C75BCObj *o, s32 max);
extern s32 func_8002A4A0(void);
extern void func_80075B08(Func801C75BCObj *o, s32 kind, AcPos *at);
extern s32 func_801C6C00(Func801C75BCNode *n, s32 id, s32 depth);
extern s32 func_80073904();
extern s32 func_801C6ED8(Func801C75BCObj *o);
extern s32 func_80073924(s32 timer);
extern s32 func_80014DFC(s32 n, u8 *weights, s32 stride);
extern void func_801C737C(Func801C75BCObj *o, s32 st);
s32 func_801C75BC(Func801C75BCObj *o, s32 event, ...)
{
    Func801C75BCDef *d = o->def;
    AcPos at;
    u8 cand[8];
    s32 now, r, a;
    Func801C75BCState *st;
    Func801C75BCSub *sub;
    Func801C75BCNode *n;
    AcPos *pts;
    Func801C75BCState *s;
    u8 *w, *c;
    s32 k;
    u8 kl, kr;

    now = func_80073714(o, 0x7FFFFFFF);
    if (event == 4)
        goto ev4;
    if (event < 5) {
        if (event == 0)
            goto ev0;
        if (event < 0)
            goto out;
        goto ev1;
    }
    if (event == 8)
        goto ev8;
    if (event < 9)
        goto out;
    if (event == 9)
        goto ev9;
    goto out;
    {
    ev4:
        if (o->dir == 97 && o->ai_mode != 255)
            o->ai_mode = 0;
        o->vel[1] = 0;
        o->kind = 0;
        st = &d->states[o->state];
        sub = &st->sub[o->sub];
        goto pick;
    ev1:
        o->target = *(void **)((char *)&event + 4);
        if (func_8002A4A0() >= 9831)
            return 0;
        st = &d->states[o->state];
        sub = &st->sub[o->sub];
        goto pick;
    ev8:
        pts = *(AcPos **)((char *)&event + 4);
        at.x = (pts[0].x + pts[1].x) >> 1;
        at.z = (pts[0].z + pts[1].z) >> 1;
        switch (d->hit_kind) {
        case 1:
        case 48:
            at.y = pts[1].y - 1000;
            func_80075B08(o, d->hit_kind, &at);
            break;
        case 65:
            at.y = (pts[0].y + pts[1].y) >> 1;
            func_80075B08(o, d->hit_kind, &at);
            break;
        }
        return 0;
    ev9:
        n = &D_80198B08[o->node];
        kl = n->kids[0];
        if (kl != 0) {
            kr = n->kids[1];
            if (kr == 0) {
                o->node = kl;
                goto set_node;
            } else if (kl == o->area) {
                o->node = kl;
            set_node:
                o->state = D_80198B08[o->node].anim;
                o->timer = func_80073904();
            } else if (kr == o->area) {
                o->node = kr;
                goto set_node;
            } else {
                a = func_801C6C00(&D_80198B08[kl], o->area, 0);
                if (func_801C6C00(&D_80198B08[n->kids[1]], o->area, 0) >= a)
                    o->node = n->kids[0];
                else
                    o->node = n->kids[1];
                goto set_node;
            }
        } else if (n->kids[1] != 0) {
            o->node = n->kids[1];
            goto set_node;
        } else {
            o->node = -1;
        }
        /* fall through */
    ev0:
        if (o->group != 0 && func_801C6ED8(o)) {
            st = &d->states[o->state];
        } else {
            st = &d->states[o->state];
            if (!(now < st->until_cur) || func_80073924(o->timer) > st->hold) {
                c = cand;
                w = d->weights;
                k = d->n_states;
                s = d->states;
                do {
                    *c = 0;
                    if (s == st) {
                        if (now < st->until_cur)
                            *c = *w;
                    } else if (now < s->until)
                        *c = *w;
                    s++;
                    c++;
                    k--;
                    w++;
                } while (k != 0);
                o->state = func_80014DFC(d->n_states, cand, 1);
                o->timer = func_80073904();
                st = &d->states[o->state];
            }
        }
        if (now >= st->until_sub)
            o->sub = 0;
        else
            o->sub = 1;
        sub = &st->sub[o->sub];
        if (func_80073924(o->sub_timer) > sub->hold[o->choice]) {
        pick:
            o->choice = func_80014DFC(d->n_subs, sub->weights, 1);
            o->sub_timer = func_80073904();
        }
        r = func_80014DFC(d->n_actions + 1, d->action_w[o->choice], 1);
        if (r)
            func_801C737C(o, *(d->actions + r));
        if (func_80073924(o->weapon_timer) > d->weapons[o->weapon].secs * 60) {
            o->weapon = func_80014DFC(d->n_weapons, &d->weapons[0].weight, 40);
            o->weapon_timer = func_80073904();
        }
    }
out:
    return 0;
}
