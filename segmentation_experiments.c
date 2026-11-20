#include <stdio.h>
#include <stdlib.h>

/*
 * Program 3: Segmentation External Fragmentation & Table-Size Comparison
 *
 * Part A:
 *   - Generates 20 reproducible random segment sizes (seed = 42).
 *   - Loads all 20 segments into a contiguous 4096 KB physical memory pool.
 *   - Removes selected non-adjacent segments to create scattered holes.
 *   - Computes allocated memory, free memory, number of holes, largest hole,
 *     and external fragmentation percentage.
 *
 * Part B:
 *   - Compares memory-table sizes for Pure Paging, Pure Segmentation, and
 *     Segmented Paging using configurable parameters.
 */

#define NUM_SEGMENTS      20
#define RANDOM_SEED       42
#define TOTAL_MEMORY_KB   4096

/* Configurable parameters for Part B (Table Size Comparison) */
#define ADDR_SPACE_BITS   32       /* 32-bit virtual address space (4 GB) */
#define PAGE_SIZE_BYTES   4096ULL  /* 4 KB page size */
#define PTE_SIZE_BYTES    4ULL     /* 4-byte Page Table Entry */
#define STE_SIZE_BYTES    8ULL     /* 8-byte Segment Table Entry (base + limit + flags) */
#define PROC_SEGMENTS     8ULL     /* Typical segments in a student process */
#define AVG_SEG_SIZE_KB   256ULL   /* Average segment size = 256 KB (64 pages/segment) */

typedef struct {
    int id;
    int start_kb;
    int size_kb;
    int active; /* 1 = allocated in memory, 0 = removed (free hole) */
} MemSegment;

static void print_memory_layout(const MemSegment segs[], int n, int used_end_kb, const char *title) {
    printf("\n--- %s ---\n", title);
    printf("%-8s %-12s %-12s %-10s %-12s\n", "SegID", "Start (KB)", "End (KB)", "Size (KB)", "State");
    printf("------------------------------------------------------------\n");
    for (int i = 0; i < n; i++) {
        printf("S%-7d %-12d %-12d %-10d %-12s\n",
               segs[i].id,
               segs[i].start_kb,
               segs[i].start_kb + segs[i].size_kb - 1,
               segs[i].size_kb,
               segs[i].active ? "ALLOCATED" : "FREE HOLE");
    }
    if (used_end_kb < TOTAL_MEMORY_KB) {
        printf("%-8s %-12d %-12d %-10d %-12s\n",
               "TAIL", used_end_kb, TOTAL_MEMORY_KB - 1,
               TOTAL_MEMORY_KB - used_end_kb, "FREE HOLE");
    }
}

static void run_fragmentation_experiment(void) {
    MemSegment segs[NUM_SEGMENTS];
    srand(RANDOM_SEED);

    int cursor = 0;
    for (int i = 0; i < NUM_SEGMENTS; i++) {
        /* Random segment size between 40 KB and 199 KB */
        int sz = 40 + (rand() % 160);
        segs[i].id = i;
        segs[i].start_kb = cursor;
        segs[i].size_kb = sz;
        segs[i].active = 1;
        cursor += sz;
    }

    printf("==========================================================================\n");
    printf(" EXPERIMENT A: EXTERNAL FRAGMENTATION WITH %d RANDOM SEGMENTS (SEED=%d)\n",
           NUM_SEGMENTS, RANDOM_SEED);
    printf("==========================================================================\n");
    printf("Total Physical Memory Pool : %d KB\n", TOTAL_MEMORY_KB);

    print_memory_layout(segs, NUM_SEGMENTS, cursor, "Memory Layout BEFORE Removal (All 20 Loaded)");

    int alloc_before = cursor;
    int free_before = TOTAL_MEMORY_KB - cursor;
    printf("\nSummary Before Removal: Allocated = %d KB | Free = %d KB (1 contiguous block)\n",
           alloc_before, free_before);

    /* Remove 8 non-adjacent segments to simulate student programs terminating */
    const int to_remove[] = {1, 3, 6, 8, 11, 13, 16, 18};
    int num_remove = (int)(sizeof(to_remove) / sizeof(to_remove[0]));
    for (int i = 0; i < num_remove; i++) {
        segs[to_remove[i]].active = 0;
    }

    print_memory_layout(segs, NUM_SEGMENTS, cursor, "Memory Layout AFTER Removing 8 Non-Adjacent Segments");

    /* Compute fragmentation metrics by scanning contiguous free holes */
    int total_allocated = 0;
    int total_free = 0;
    int hole_count = 0;
    int largest_hole = 0;
    int current_hole = 0;

    for (int i = 0; i < NUM_SEGMENTS; i++) {
        if (segs[i].active) {
            total_allocated += segs[i].size_kb;
            if (current_hole > 0) {
                hole_count++;
                if (current_hole > largest_hole) {
                    largest_hole = current_hole;
                }
                current_hole = 0;
            }
        } else {
            total_free += segs[i].size_kb;
            current_hole += segs[i].size_kb;
        }
    }

    /* Include trailing free block at the end of the memory pool */
    int tail_free = TOTAL_MEMORY_KB - cursor;
    if (tail_free > 0) {
        total_free += tail_free;
        current_hole += tail_free;
    }
    if (current_hole > 0) {
        hole_count++;
        if (current_hole > largest_hole) {
            largest_hole = current_hole;
        }
    }

    /* External fragmentation metric: fraction of free memory unusable by a single block */
    double ext_frag_pct = 0.0;
    if (total_free > 0) {
        ext_frag_pct = (1.0 - ((double)largest_hole / (double)total_free)) * 100.0;
    }

    printf("\n--- Fragmentation Metrics AFTER Removal ---\n");
    printf("  Total Physical Memory       : %d KB\n", TOTAL_MEMORY_KB);
    printf("  Total Allocated Memory      : %d KB\n", total_allocated);
    printf("  Total Free Memory           : %d KB\n", total_free);
    printf("  Number of Free Holes        : %d\n", hole_count);
    printf("  Largest Contiguous Hole     : %d KB\n", largest_hole);
    printf("  External Fragmentation      : %.2f%%  [Formula: (1 - LargestHole / TotalFree) * 100]\n",
           ext_frag_pct);

    int incoming_req = largest_hole + 100;
    if (incoming_req <= total_free) {
        printf("  Allocation Failure Example  : A new segment requesting %d KB FAILS to load\n",
               incoming_req);
        printf("                                even though %d KB total memory is free!\n",
               total_free);
    }
}

static void run_table_size_comparison(void) {
    printf("\n==========================================================================\n");
    printf(" EXPERIMENT B: MEMORY TABLE SIZE COMPARISON (PER PROCESS)\n");
    printf("==========================================================================\n");
    printf("Configured Assumptions:\n");
    printf("  - Virtual Address Space : %d-bit (4 GB)\n", ADDR_SPACE_BITS);
    printf("  - Page Size             : %llu bytes (4 KB)\n", PAGE_SIZE_BYTES);
    printf("  - Page Table Entry (PTE): %llu bytes\n", PTE_SIZE_BYTES);
    printf("  - Segment Entry (STE)   : %llu bytes\n", STE_SIZE_BYTES);
    printf("  - Segments per Process  : %llu segments (avg %llu KB each -> %llu MB active)\n\n",
           PROC_SEGMENTS, AVG_SEG_SIZE_KB, (PROC_SEGMENTS * AVG_SEG_SIZE_KB) / 1024ULL);

    /* 1. Pure Paging (Flat single-level table for full 32-bit space) */
    unsigned long long paging_entries = (1ULL << (ADDR_SPACE_BITS - 12)); /* 2^20 */
    unsigned long long paging_bytes   = paging_entries * PTE_SIZE_BYTES;  /* 4 MB */

    /* 2. Pure Segmentation */
    unsigned long long seg_entries = PROC_SEGMENTS;
    unsigned long long seg_bytes   = seg_entries * STE_SIZE_BYTES;

    /* 3. Segmented Paging (Segment table + per-segment page tables for active segments) */
    unsigned long long pages_per_seg   = (AVG_SEG_SIZE_KB * 1024ULL) / PAGE_SIZE_BYTES;
    unsigned long long total_seg_ptes  = PROC_SEGMENTS * pages_per_seg;
    unsigned long long seg_paging_bytes = seg_bytes + (total_seg_ptes * PTE_SIZE_BYTES);

    printf("%-22s %-18s %-20s\n", "Scheme", "Table Entries", "Table Memory Required");
    printf("--------------------------------------------------------------------------\n");
    printf("%-22s %-18llu %llu bytes (%.2f MB)\n",
           "Pure Paging (Flat)", paging_entries, paging_bytes,
           (double)paging_bytes / (1024.0 * 1024.0));
    printf("%-22s %-18llu %llu bytes\n",
           "Pure Segmentation", seg_entries, seg_bytes);
    printf("%-22s %-18llu %llu bytes (%.2f KB)\n",
           "Segmented Paging", seg_entries + total_seg_ptes, seg_paging_bytes,
           (double)seg_paging_bytes / 1024.0);
    printf("--------------------------------------------------------------------------\n");
    printf("Note: Segmented paging stores %llu STEs (%llu B) + %llu PTEs (%llu B).\n",
           seg_entries, seg_bytes, total_seg_ptes, total_seg_ptes * PTE_SIZE_BYTES);
}

int main(void) {
    run_fragmentation_experiment();
    run_table_size_comparison();
    return 0;
}
