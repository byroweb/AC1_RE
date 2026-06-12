/*
 * farsi_name_shape.c — runtime contextual shaper for TYPED Farsi names (AC1).
 *
 * Validated reference: farsi_runtime_shape.py, which is byte-identical to the
 * build-time farsi_shape.py. This C mirrors that logic exactly so it can be
 * compiled into FDAT entry 201 and trusted.
 *
 *   in  : ords[n] = logical-order keyboard letter ordinals (0..NLET-1)
 *   out : draw-order Farsi glyph bytes (>=0x80) + '>' (0x3E) terminator
 *   ret : number of glyph bytes written (excluding terminator)
 *
 * The patched draw_string renders each out[] byte via draw_farsi. Integration:
 * keep a small logical-ordinal scratch buffer for the name being typed; after
 * every append/delete, call shape_name() and write the result into the existing
 * name buffer (AC 0x80031BD4 / PILOT 0x80031BE6). Every name-display site already
 * draw_strings that buffer, so no display code changes.
 *
 * Placement target on console: reclaimed draw_kanji region @0x80065DBC.
 *
 * Tables generated from farsi_runtime_shape.KEYBOARD / farsi_glyphs (atlas
 * contract: per-letter forms contiguous; D=[isol,init,medi,fina], R=[isol,fina];
 * lamalef.isol/.fina = 0xF4/0xF5).
 *
 * NOTE: names are letters-only here (no embedded space). A space/break ordinal
 * can be added later as a join-run breaker if name spaces are wanted.
 */

#define F_DUAL 0x01
#define F_LAM  0x02
#define F_ALEF 0x04
#define LAMALEF_ISOL 0xF4
#define LAMALEF_FINA 0xF5
#define NLET   33
#define NAME_MAX 8           /* engine caps names at 8 chars (cursor +26 wraps) */

/* isolated-form atlas index per keyboard ordinal (form group base) */
static const unsigned char LTAB_ISOL[NLET] = {
    0x00,0x02,0x06,0x0A,0x0E,0x12,0x16,0x1A,0x1E,0x22,0x24,0x26,0x28,0x2A,0x2C,
    0x30,0x34,0x38,0x3C,0x40,0x44,0x48,0x4C,0x50,0x54,0x58,0x5C,0x60,0x64,0x68,
    0x6A,0x6E,0x72
};
/* flags per ordinal: bit0 dual-joining(D), bit1 LAM, bit2 ALEF(alef/alefmadda) */
static const unsigned char LTAB_FLAGS[NLET] = {
    4,1,1,1,1,1,1,1,1,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,3,1,1,0,1,1,4
};
/* dual-join contextual offset, indexed by (prev_conn<<1 | next_conn) */
static const unsigned char DOFF[4] = {0,1,3,2};   /* isol,init,fina,medi */

/* Non-letter literal pass-throughs. A token >= NLET indexes this table; the
 * shaper emits the byte verbatim and breaks the join run. Order matches
 * farsi_runtime_shape.LITERALS: space, d0..d9, . ( ) , (decimal) (kashida).
 * Punct bytes are kept-ASCII font cells (Persian-punct glyphs drawn into them);
 * exact cells are a font-layout decision. */
#define NLIT 17
static const unsigned char LITERALS[NLIT] = {
    0x20,                                           /* space            */
    0xF6,0xF7,0xF8,0xF9,0xFA,0xFB,0xFC,0xFD,0xFE,0xFF,  /* ۰..۹         */
    0x2E,0x28,0x29,0x2C,0x60,0x5F                   /* . ( ) ، ٫ ـ      */
};
#define IS_LETTER(t) ((t) < NLET)

int shape_name(const unsigned char *toks, int n, unsigned char *out)
{
    unsigned char tmp[NAME_MAX + 1];    /* logical-order shaped bytes */
    int m = 0;
    int i = 0;
    int k;
    int prev_conn = 0;   /* running: did the previous emitted unit connect left? */

    while (i < n) {
        unsigned char o = toks[i];

        /* non-letter literal: emit verbatim, breaks the join run */
        if (!IS_LETTER(o)) {
            tmp[m++] = LITERALS[o - NLET];
            prev_conn = 0;
            i++;
            continue;
        }

        {
            unsigned char fl = LTAB_FLAGS[o];
            int dual      = fl & F_DUAL;
            int next_let  = (i + 1 < n) && IS_LETTER(toks[i + 1]);
            int next_conn = dual && next_let;

            /* lam + alef(+madda) mandatory ligature. Alef is right-joining, so
               the unit after the ligature never connects left (prev_conn = 0). */
            if ((fl & F_LAM) && next_let && (LTAB_FLAGS[toks[i + 1]] & F_ALEF)) {
                tmp[m++] = prev_conn ? LAMALEF_FINA : LAMALEF_ISOL;
                prev_conn = 0;
                i += 2;
                continue;
            }

            if (dual)
                tmp[m++] = 0x80 + LTAB_ISOL[o] + DOFF[(prev_conn << 1) | (next_conn ? 1 : 0)];
            else
                tmp[m++] = 0x80 + LTAB_ISOL[o] + (prev_conn ? 1 : 0);
            /* the next letter connects left iff this one is dual-joining */
            prev_conn = dual ? 1 : 0;
            i++;
        }
    }

    /* reverse logical -> RTL draw order, terminate */
    for (k = 0; k < m; k++)
        out[k] = tmp[m - 1 - k];
    out[m] = 0x3E;          /* '>' */
    return m;
}

#ifdef HOST_TEST
#include <stdio.h>
int main(void)
{
    /* test vectors as ordinal arrays (logical order), mirrored from the py harness */
    struct { const char *w; unsigned char o[9]; int n; } T[] = {
        {"salaam",  {14,26,0,27},          4},
        {"iran",    {0,31,11,0,28},        5},
        {"baazi",   {1,0,12,31},           4},
        {"baalaa",  {1,0,26,0},            4},
        {"core",    {24,29,11},            3},
        {"baba",    {1,0,1,0},             4},
        {"laa",     {26,0},                2},
        {"baalaalaa",{1,0,26,0,26,0},      6},
        {"nnnn",    {28,28,28,28},         4},
        {"bbbb",    {1,1,1,1},             4},
        {"bar2",    {1,0,11,36},           4},   /* digit literal       */
        {"123",     {35,36,37},            3},   /* digits reverse RTL  */
        {"ba_la",   {1,0,33,26,0},         5},   /* space breaks join   */
        {"core.",   {24,29,11,44},         4},   /* ASCII period        */
        {"salam_donya",{14,26,0,27,33,9,28,31,0}, 9},
    };
    int i, j, m;
    unsigned char out[16];
    for (i = 0; i < (int)(sizeof T / sizeof T[0]); i++) {
        m = shape_name(T[i].o, T[i].n, out);
        printf("%-10s ", T[i].w);
        for (j = 0; j < m; j++) printf("%02X ", out[j]);
        printf("\n");
    }
    return 0;
}
#endif
