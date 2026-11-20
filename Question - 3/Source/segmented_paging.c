#include <stdio.h>

/*
 * Program 2: Segmentation with Paging (Simplified Educational Intel x86 Model)
 *
 * Demonstrates the 5-stage address translation flow:
 *   1. Logical address provides (segment, offset).
 *   2. Segment table provides the segment's limit and page-table base.
 *   3. Offset is split into (page_number, page_offset) using PAGE_SIZE.
 *   4. Segment's page table maps page_number -> physical frame (or page fault).
 *   5. Physical address = frame * PAGE_SIZE + page_offset.
 *
 * NOTE: This is a simplified educational model illustrating classic IA-32/x86
 * segmented paging concepts. Modern x86-64 systems in 64-bit long mode use a
 * flat memory model for CS/DS/ES/SS and rely primarily on 4-level/5-level paging.
 */

#define PAGE_SIZE 1024
#define NUM_SEGMENTS 3
#define MAX_PAGES_PER_SEG 4

typedef struct {
    int seg_id;
    const char *name;
    unsigned int limit;                 /* Segment limit in bytes */
    int num_pages;                      /* Pages belonging to this segment */
    int page_table[MAX_PAGES_PER_SEG];  /* Physical frame number, or -1 if not in RAM */
} SegmentedPageEntry;

static const SegmentedPageEntry seg_table[NUM_SEGMENTS] = {
    {0, "Code",  3072, 3, {4, 7, 1, -1}},    /* 3 pages: frames 4, 7, 1 */
    {1, "Data",  2048, 2, {9, -1, -1, -1}},  /* 2 pages: page 0 -> frame 9, page 1 -> not in RAM */
    {2, "Stack", 4096, 4, {2, 5, 8, 12}}     /* 4 pages: frames 2, 5, 8, 12 */
};

static void print_flow_diagram(void) {
    printf("==========================================================================\n");
    printf("   SEGMENTATION WITH PAGING - TRANSLATION PIPELINE (EDUCATIONAL MODEL)    \n");
    printf("==========================================================================\n");
    printf("  Logical Address: (Segment s, Offset d)\n");
    printf("          |\n");
    printf("          v\n");
    printf("  [Stage 1 & 2] Segment Table[s] -> (Page-Table Base, Limit)\n");
    printf("          |---> If d >= Limit : SEGMENTATION FAULT\n");
    printf("          v\n");
    printf("  [Stage 3] Split Offset d -> page_num (p) = d / %d, page_off (o) = d %% %d\n",
           PAGE_SIZE, PAGE_SIZE);
    printf("          |\n");
    printf("          v\n");
    printf("  [Stage 4] Segment's Page Table[p] -> Physical Frame (f)\n");
    printf("          |---> If f == -1    : PAGE FAULT\n");
    printf("          v\n");
    printf("  [Stage 5] Physical Address = (f * %d) + o\n", PAGE_SIZE);
    printf("==========================================================================\n\n");
}

static void translate_segmented_paging(int seg, unsigned int offset) {
    printf("Logical Address: (Segment = %d, Offset = %u)\n", seg, offset);

    /* Stage 1 & 2: Segment table lookup and limit check */
    if (seg < 0 || seg >= NUM_SEGMENTS) {
        printf("  [Stage 1] FAIL: Invalid segment number %d.\n\n", seg);
        return;
    }

    const SegmentedPageEntry *se = &seg_table[seg];
    printf("  [Stage 2] Segment '%s' -> Limit = %u bytes, Page Table Size = %d entries\n",
           se->name, se->limit, se->num_pages);

    if (offset >= se->limit) {
        printf("  [Stage 2] TRAP: Offset %u >= Limit %u -> SEGMENTATION FAULT\n\n",
               offset, se->limit);
        return;
    }

    /* Stage 3: Split offset into page number and page offset */
    unsigned int page_num = offset / PAGE_SIZE;
    unsigned int page_off = offset % PAGE_SIZE;
    printf("  [Stage 3] Split offset %u -> Page Number = %u, Page Offset = %u\n",
           offset, page_num, page_off);

    /* Stage 4: Look up physical frame in the segment's page table */
    if ((int)page_num >= se->num_pages) {
        printf("  [Stage 4] TRAP: Page %u exceeds segment page table -> INVALID PAGE\n\n",
               page_num);
        return;
    }

    int frame = se->page_table[page_num];
    if (frame == -1) {
        printf("  [Stage 4] TRAP: PageTable[%u] = -1 -> PAGE FAULT (page not in RAM)\n\n",
               page_num);
        return;
    }
    printf("  [Stage 4] Segment Page Table[%u] -> Physical Frame = %d\n", page_num, frame);

    /* Stage 5: Combine frame and page offset */
    unsigned int phys_addr = (unsigned int)frame * PAGE_SIZE + page_off;
    printf("  [Stage 5] Physical Address = %d * %d + %u = %u\n\n",
           frame, PAGE_SIZE, page_off, phys_addr);
}

int main(void) {
    print_flow_diagram();

    printf("NOTE: Simplified educational model of Intel x86-style segmented paging.\n");
    printf("      Modern x86-64 uses flat segmentation with multi-level paging.\n\n");

    const struct {
        int seg;
        unsigned int offset;
    } tests[] = {
        {0, 2100},  /* Code segment: page 2 (frame 1), offset 52 -> 1076 */
        {1,  512},  /* Data segment: page 0 (frame 9), offset 512 -> 9728 */
        {1, 1200},  /* Data segment: page 1 -> frame -1 (PAGE FAULT) */
        {2, 3500},  /* Stack segment: page 3 (frame 12), offset 428 -> 12716 */
        {0, 3500}   /* Code segment: offset 3500 >= limit 3072 (SEGMENTATION FAULT) */
    };

    int n = (int)(sizeof(tests) / sizeof(tests[0]));
    for (int i = 0; i < n; i++) {
        printf("--- Example %d ---\n", i + 1);
        translate_segmented_paging(tests[i].seg, tests[i].offset);
    }

    return 0;
}
