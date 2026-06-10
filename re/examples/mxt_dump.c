/* mxt_dump.c — exercise the MXT reference model against a real ".T" image.
 *
 *   mxt_dump <file.T> [entry]
 *
 * With no entry: prints the archive summary and validates the checksums of the
 * known code overlays (FDAT.T entries 201-204). With an entry index: prints that
 * entry's offset/size, checksum status, and (if it's a code overlay) its header.
 *
 * Demonstrates that the behavioral model reproduces the game's container layout
 * and checksum exactly: run it on fdat_extracted.T and every code overlay
 * validates against seed 0x12345678. */
#include "ac1_mxt.h"
#include <stdio.h>
#include <stdlib.h>

static uint8_t *slurp(const char *path, size_t *n) {
    FILE *f = fopen(path, "rb");
    if (!f) { perror(path); return NULL; }
    fseek(f, 0, SEEK_END); long sz = ftell(f); fseek(f, 0, SEEK_SET);
    uint8_t *b = malloc((size_t)sz);
    if (b && fread(b, 1, (size_t)sz, f) != (size_t)sz) { free(b); b = NULL; }
    fclose(f);
    if (n) *n = (size_t)sz;
    return b;
}

static void show_entry(const ac1_mxt *m, uint32_t i) {
    size_t off, len;
    if (ac1_mxt_entry(m, i, &off, &len) != 0) { printf("  entry %u out of range\n", i); return; }
    printf("  entry %3u: sectors %u..%u  off 0x%zx  %zu B", i,
           m->offset[i], m->offset[i + 1], off, len);
    if (len == 0) { printf("  (empty)\n"); return; }
    const void *e = m->data + off;
    int ok = ac1_mxt_verify(e, len);
    uint32_t cks = ac1_mxt_checksum(e, len);
    printf("  checksum %s (0x%08x)\n", ok ? "OK" : "--", cks);
    if (ok) {
        ac1_overlay_header h;
        ac1_overlay_read_header(e, &h);
        if (h.magic[0] == 'E' && h.magic[5] == 'Y')   /* "ENERGY" */
            printf("            overlay: entry_fn=0x%08x magic=\"%.6s\"\n", h.entry_fn, h.magic);
    }
}

int main(int argc, char **argv) {
    if (argc < 2) { fprintf(stderr, "usage: %s <file.T> [entry]\n", argv[0]); return 2; }
    size_t n; uint8_t *buf = slurp(argv[1], &n);
    if (!buf) return 1;

    ac1_mxt m;
    if (ac1_mxt_open(&m, buf, n, AC1_TOC_AUTO) != 0) {
        fprintf(stderr, "not a valid MXT/.T image\n"); free(buf); return 1;
    }
    printf("%s: %zu B, %u sectors, %s, %u entries\n", argv[1], n, m.sectors,
           m.kind == AC1_TOC_COUNT_FIRST ? "count-first" : "offset-first", m.count);

    if (argc >= 3) { show_entry(&m, (uint32_t)strtoul(argv[2], NULL, 0)); free(buf); return 0; }

    /* default: validate the FDAT code overlays */
    static const struct { uint32_t e; const char *name; } ovl[] = {
        {201, "front_end"}, {202, "mission"}, {203, "link_vs"}, {204, "local_battle"}
    };
    int allok = 1;
    printf("code overlays:\n");
    for (size_t k = 0; k < sizeof ovl / sizeof ovl[0]; k++) {
        if (ovl[k].e + 1 > m.count) continue;
        printf("  [%-12s] ", ovl[k].name);
        size_t off, len; ac1_mxt_entry(&m, ovl[k].e, &off, &len);
        int ok = ac1_mxt_verify(m.data + off, len);
        allok &= ok;
        printf("entry %u  %zu B  checksum %s\n", ovl[k].e, len, ok ? "OK" : "FAIL");
    }
    printf("overlay checksums: %s\n", allok ? "ALL VALID" : "MISMATCH");
    free(buf);
    return allok ? 0 : 1;
}
