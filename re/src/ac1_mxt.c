/* ac1_mxt.c — behavioral reference model of the AC1 MXT container + checksum.
 * Portable C99. See ac1_mxt.h and docs/MXT_LOADER.md. */
#include "ac1_mxt.h"
#include <string.h>

static uint16_t rd16(const uint8_t *p) { return (uint16_t)(p[0] | (p[1] << 8)); }
static uint32_t rd32(const uint8_t *p) {
    return (uint32_t)p[0] | ((uint32_t)p[1] << 8) |
           ((uint32_t)p[2] << 16) | ((uint32_t)p[3] << 24);
}

/* Heuristic mirror of tools/extract/extract_t.py: count-first archives store the entry
 * count in u16[0] with offsets beginning at u16[1] (FDAT.T, MENU_TIM.T);
 * offset-first store offset[0]==1 in u16[0]. */
static ac1_toc_kind detect_kind(const uint8_t *s0, uint32_t sectors) {
    uint16_t u0 = rd16(s0), u1 = rd16(s0 + 2);
    if (u1 == 1 && u0 > 1 && u0 <= sectors)
        return AC1_TOC_COUNT_FIRST;
    return AC1_TOC_OFFSET_FIRST;
}

int ac1_mxt_open(ac1_mxt *m, const uint8_t *data, size_t size, ac1_toc_kind hint) {
    if (!m || !data || size < AC1_MXT_SECTOR || (size % AC1_MXT_SECTOR))
        return -1;
    memset(m, 0, sizeof *m);
    m->data = data;
    m->size = size;
    m->sectors = (uint32_t)(size / AC1_MXT_SECTOR);
    m->kind = (hint == AC1_TOC_AUTO) ? detect_kind(data, m->sectors) : hint;

    /* index of the first offset in the u16 table */
    uint32_t base = (m->kind == AC1_TOC_COUNT_FIRST) ? 1 : 0;
    uint32_t count = (m->kind == AC1_TOC_COUNT_FIRST) ? rd16(data) : 0;

    /* For offset-first, walk the leading monotonic run as offsets. */
    if (m->kind == AC1_TOC_OFFSET_FIRST) {
        uint32_t prev = 0, n = 0;
        for (uint32_t i = 0; i < 1024; i++) {
            uint16_t v = rd16(data + i * 2);
            if (v < prev || v > m->sectors) break;
            prev = v; n++;
        }
        count = (n > 0) ? n - 1 : 0;  /* last value is the end sentinel */
    }
    if (count == 0 || count > 1024) return -1;
    m->count = count;
    for (uint32_t i = 0; i <= count; i++)
        m->offset[i] = rd16(data + (base + i) * 2);
    return 0;
}

int ac1_mxt_entry(const ac1_mxt *m, uint32_t i, size_t *off, size_t *len) {
    if (!m || i >= m->count) return -1;
    uint32_t s0 = m->offset[i], s1 = m->offset[i + 1];
    if (off) *off = (size_t)s0 * AC1_MXT_SECTOR;
    if (len) *len = (size_t)(s1 - s0) * AC1_MXT_SECTOR;
    return 0;
}

uint32_t ac1_mxt_checksum(const void *entry, size_t len) {
    const uint8_t *p = (const uint8_t *)entry;
    size_t nwords = len / 4;
    uint32_t sum = AC1_MXT_CHECKSUM_SEED;
    if (nwords == 0) return sum;
    for (size_t i = 0; i + 1 < nwords; i++)   /* every word except the last */
        sum += rd32(p + i * 4);
    return sum;
}

int ac1_mxt_verify(const void *entry, size_t len) {
    size_t nwords = len / 4;
    if (nwords == 0) return 0;
    uint32_t stored = rd32((const uint8_t *)entry + (nwords - 1) * 4);
    return ac1_mxt_checksum(entry, len) == stored;
}

void ac1_mxt_fix_checksum(void *entry, size_t len) {
    size_t nwords = len / 4;
    if (nwords == 0) return;
    uint32_t cks = ac1_mxt_checksum(entry, len);
    uint8_t *w = (uint8_t *)entry + (nwords - 1) * 4;
    w[0] = (uint8_t)(cks);       w[1] = (uint8_t)(cks >> 8);
    w[2] = (uint8_t)(cks >> 16); w[3] = (uint8_t)(cks >> 24);
}

void ac1_overlay_read_header(const void *entry, ac1_overlay_header *h) {
    const uint8_t *p = (const uint8_t *)entry;
    h->flags    = rd32(p);
    h->entry_fn = rd32(p + 4);
    memcpy(h->magic, p + 8, 8);
}
