#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define NUM_PROCESSES 100
#define RANDOM_SEED   42

/*
 * Experiment 1: Internal Fragmentation for 100 processes of reproducible random sizes.
 * Student program sizes are generated between 4 KB and 4096 KB (4 MB).
 */
static void run_fragmentation_experiment(void) {
    const unsigned long long page_sizes[3] = {
        1024ULL,                /* 1 KB */
        4096ULL,                /* 4 KB */
        2ULL * 1024ULL * 1024ULL /* 2 MB */
    };
    const char *size_names[3] = {"1 KB", "4 KB", "2 MB"};

    unsigned long long proc_sizes[NUM_PROCESSES];
    srand(RANDOM_SEED);

    unsigned long long total_requested = 0;
    for (int i = 0; i < NUM_PROCESSES; i++) {
        /* Random size between 4 KB (4,096 B) and 4 MB (4,194,304 B) */
        unsigned long long kb = 4ULL + (unsigned long long)(rand() % 4093);
        unsigned long long extra_bytes = (unsigned long long)(rand() % 1024);
        proc_sizes[i] = kb * 1024ULL + extra_bytes;
        total_requested += proc_sizes[i];
    }

    printf("========================================================================\n");
    printf(" EXPERIMENT 1: INTERNAL FRAGMENTATION (%d RANDOM PROCESSES, SEED=%d)\n",
           NUM_PROCESSES, RANDOM_SEED);
    printf("========================================================================\n");
    printf("Total Requested Memory (100 processes): %.2f MB (%llu bytes)\n\n",
           (double)total_requested / (1024.0 * 1024.0), total_requested);

    printf("%-10s %-16s %-16s %-16s %-10s\n",
           "Page Size", "Total Allocated", "Total Waste", "Avg Waste/Proc", "Waste (%)");
    printf("------------------------------------------------------------------------\n");

    for (int s = 0; s < 3; s++) {
        unsigned long long psize = page_sizes[s];
        unsigned long long total_allocated = 0;
        unsigned long long total_waste = 0;

        for (int i = 0; i < NUM_PROCESSES; i++) {
            unsigned long long pages_needed = (proc_sizes[i] + psize - 1) / psize;
            unsigned long long allocated = pages_needed * psize;
            total_allocated += allocated;
            total_waste += (allocated - proc_sizes[i]);
        }

        double avg_waste_kb = ((double)total_waste / NUM_PROCESSES) / 1024.0;
        double waste_pct = ((double)total_waste / (double)total_allocated) * 100.0;

        printf("%-10s %-12.2f MB  %-12.2f MB  %-11.2f KB   %-8.2f%%\n",
               size_names[s],
               (double)total_allocated / (1024.0 * 1024.0),
               (double)total_waste / (1024.0 * 1024.0),
               avg_waste_kb,
               waste_pct);
    }
    printf("\nNote: Theoretical average internal fragmentation per process is ~0.5 * page_size.\n\n");
}

/*
 * Experiment 2: Effective Memory-Access Time (EMAT) with and without TLB.
 * Assumptions:
 *   - Main memory access time (T_mem) = 100 ns
 *   - TLB lookup time (T_tlb)         = 10 ns
 *   - Single-level page table (1 extra memory access on TLB miss)
 */
static void run_tlb_experiment(void) {
    const double t_mem = 100.0; /* ns */
    const double t_tlb = 10.0;  /* ns */
    const double emat_no_tlb = 2.0 * t_mem;
    const double hit_ratios[3] = {0.80, 0.95, 0.99};

    printf("========================================================================\n");
    printf(" EXPERIMENT 2: EFFECTIVE MEMORY-ACCESS TIME (WITH AND WITHOUT TLB)\n");
    printf("========================================================================\n");
    printf("Assumptions: Memory access time (T_m) = %.0f ns, TLB lookup (T_tlb) = %.0f ns\n",
           t_mem, t_tlb);
    printf("Without TLB (single-level page table): EMAT = 2 * T_m = %.2f ns\n\n", emat_no_tlb);

    printf("%-16s %-18s %-18s %-16s\n",
           "TLB Hit Ratio", "EMAT (ns)", "Overhead vs 100ns", "Speedup vs No-TLB");
    printf("------------------------------------------------------------------------\n");

    for (int i = 0; i < 3; i++) {
        double alpha = hit_ratios[i];
        double emat = alpha * (t_tlb + t_mem) + (1.0 - alpha) * (t_tlb + 2.0 * t_mem);
        double overhead_pct = ((emat - t_mem) / t_mem) * 100.0;
        double speedup = emat_no_tlb / emat;

        printf("%-14.0f%%  %-16.2f   +%-15.2f%%  %.2fx\n",
               alpha * 100.0, emat, overhead_pct, speedup);
    }
    printf("\n");
}

/*
 * Experiment 3: Page-Table Size Calculations for 32-bit and 48-bit address spaces
 * with 4 KB (2^12 bytes) pages.
 */
static void run_page_table_size_experiment(void) {
    const unsigned long long page_size = 4096ULL; /* 4 KB = 2^12 bytes */

    /* 32-bit virtual address space, 4-byte PTE */
    unsigned long long entries_32 = (1ULL << (32 - 12)); /* 2^20 = 1,048,576 */
    unsigned long long pte_32 = 4ULL;
    unsigned long long flat_size_32_bytes = entries_32 * pte_32; /* 4 MB */

    /* 48-bit virtual address space, 8-byte PTE */
    unsigned long long entries_48 = (1ULL << (48 - 12)); /* 2^36 = 68,719,476,736 */
    unsigned long long pte_48 = 8ULL;
    double flat_size_48_gb = ((double)entries_48 * (double)pte_48) / (1024.0 * 1024.0 * 1024.0);

    printf("========================================================================\n");
    printf(" EXPERIMENT 3: FLAT PAGE-TABLE SIZE vs. MULTI-LEVEL PAGING (4 KB PAGES)\n");
    printf("========================================================================\n");
    printf("Page size = %llu bytes (12 offset bits)\n\n", page_size);

    printf("1. 32-bit Virtual Address Space (PTE = %llu bytes):\n", pte_32);
    printf("   - Number of pages (entries) : 2^20 = %llu\n", entries_32);
    printf("   - Flat page-table size      : %.2f MB per process\n",
           (double)flat_size_32_bytes / (1024.0 * 1024.0));
    printf("   - 2-level paging (10+10+12) : Only active 4 KB leaf tables allocated\n");
    printf("     (e.g., 12 KB for a small process: 1 outer dir + 2 leaf tables)\n\n");

    printf("2. 48-bit Virtual Address Space (PTE = %llu bytes):\n", pte_48);
    printf("   - Number of pages (entries) : 2^36 = %llu\n", entries_48);
    printf("   - Flat page-table size      : %.2f GB per process (IMPRACTICAL!)\n", flat_size_48_gb);
    printf("   - 4-level paging (9+9+9+9+12): Each table holds 512 entries (4 KB).\n");
    printf("     A small process mapping 1 region uses only 4 tables = 16 KB.\n\n");
}

int main(void) {
    run_fragmentation_experiment();
    run_tlb_experiment();
    run_page_table_size_experiment();
    return 0;
}
