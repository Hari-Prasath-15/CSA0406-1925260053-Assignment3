#include <stdio.h>

/*
 * Program 4: page_replacement_simulator.c
 * Stage 3 - Experiment 4: Three-Task Page-Replacement Simulation (LRU)
 *
 * Simulates the three edge-device tasks competing for limited physical frames
 * (12 total frames, 20 total distinct pages across the 3 tasks):
 *   - Real-Time Control (RT) : Pages  0..3   (4 pages, periodic every cycle)
 *   - Machine Learning  (ML) : Pages 10..21  (12 pages, heavy tensor sweep)
 *   - Logging Service   (LOG): Pages 30..33  (4 pages, periodic log flush)
 *
 * Compares two configurations on the exact same reproducible 60-access trace:
 *   Mode A (No Frame Reservation): All 12 frames shared globally under LRU.
 *          -> ML memory sweeps evict RT pages, causing RT page faults!
 *   Mode B (Reserved + Locked RT): 4 frames permanently locked for RT (pages 0..3),
 *          remaining 8 frames shared by ML and LOG under LRU.
 *          -> RT incurs 0 runtime page faults after pre-loading!
 */

#define MAX_FRAMES 16
#define TRACE_LEN  60
#define TOTAL_FRAMES 12
#define RT_RESERVED_FRAMES 4

static int task_of_page(int page) {
    if (page >= 0 && page <= 3)   return 0; /* RT */
    if (page >= 10 && page <= 21) return 1; /* ML */
    return 2;                               /* LOG */
}

static int find_in(const int f[], int used, int page) {
    for (int i = 0; i < used; i++) {
        if (f[i] == page) return i;
    }
    return -1;
}

typedef struct {
    int rt_faults;
    int ml_faults;
    int log_faults;
    int total_faults;
} FaultStats;

/* Mode A: Global shared LRU pool (NO reservation for RT) */
static FaultStats simulate_unreserved_lru(const int trace[], int len, int frames) {
    int f[MAX_FRAMES], last[MAX_FRAMES];
    int used = 0;
    FaultStats st = {0, 0, 0, 0};

    for (int i = 0; i < len; i++) {
        int page = trace[i];
        int k = find_in(f, used, page);
        if (k >= 0) {
            last[k] = i;
            continue;
        }
        st.total_faults++;
        int t = task_of_page(page);
        if (t == 0) st.rt_faults++;
        else if (t == 1) st.ml_faults++;
        else st.log_faults++;

        if (used < frames) {
            f[used] = page;
            last[used] = i;
            used++;
        } else {
            int victim = 0;
            for (int j = 1; j < frames; j++) {
                if (last[j] < last[victim]) victim = j;
            }
            f[victim] = page;
            last[victim] = i;
        }
    }
    return st;
}

/* Mode B: 4 frames pre-loaded and LOCKED for RT (0..3); remaining frames LRU for ML+LOG */
static FaultStats simulate_reserved_rt_lru(const int trace[], int len, int total_frames, int rt_frames) {
    int rt_locked[MAX_FRAMES] = {0, 1, 2, 3}; /* Pre-loaded & locked before runtime */
    int shared_cap = total_frames - rt_frames;
    int sf[MAX_FRAMES], slast[MAX_FRAMES];
    int sused = 0;
    FaultStats st = {0, 0, 0, 0};

    for (int i = 0; i < len; i++) {
        int page = trace[i];
        int t = task_of_page(page);

        if (t == 0) {
            /* Check locked RT frames */
            if (find_in(rt_locked, rt_frames, page) >= 0) {
                continue; /* Guaranteed HIT */
            }
            st.rt_faults++;
            st.total_faults++;
            continue;
        }

        /* ML and LOG share the remaining unlocked frames under LRU */
        int k = find_in(sf, sused, page);
        if (k >= 0) {
            slast[k] = i;
            continue;
        }
        st.total_faults++;
        if (t == 1) st.ml_faults++;
        else st.log_faults++;

        if (sused < shared_cap) {
            sf[sused] = page;
            slast[sused] = i;
            sused++;
        } else {
            int victim = 0;
            for (int j = 1; j < shared_cap; j++) {
                if (slast[j] < slast[victim]) victim = j;
            }
            sf[victim] = page;
            slast[victim] = i;
        }
    }
    return st;
}

int main(void) {
    /*
     * Reproducible 60-access interleaved trace representing 3 control cycles:
     * Each cycle: RT runs (0,1,2,3) -> ML sweeps 12 pages (10..21) -> LOG flushes (30..33)
     */
    int trace[TRACE_LEN];
    int idx = 0;
    for (int cycle = 0; cycle < 3; cycle++) {
        for (int p = 0; p < 4; p++)   trace[idx++] = p;       /* RT: 0..3 */
        for (int p = 10; p < 22; p++) trace[idx++] = p;       /* ML: 10..21 */
        for (int p = 30; p < 34; p++) trace[idx++] = p;       /* LOG: 30..33 */
    }

    printf("==========================================================================\n");
    printf(" EXPERIMENT 4: THREE-TASK PAGE-REPLACEMENT SIMULATION (LRU, %d Frames)\n",
           TOTAL_FRAMES);
    printf("==========================================================================\n");
    printf("Workload Profile (%d total accesses across 3 cycles):\n", TRACE_LEN);
    printf("  - Real-Time Control (RT) : Pages  0..3  (4 pages, 12 accesses)\n");
    printf("  - Machine Learning  (ML) : Pages 10..21 (12 pages, 36 accesses)\n");
    printf("  - Logging Service   (LOG): Pages 30..33 (4 pages, 12 accesses)\n\n");

    FaultStats unres = simulate_unreserved_lru(trace, TRACE_LEN, TOTAL_FRAMES);
    FaultStats res   = simulate_reserved_rt_lru(trace, TRACE_LEN, TOTAL_FRAMES, RT_RESERVED_FRAMES);

    printf("%-34s %-8s %-8s %-8s %-8s\n",
           "Allocation Strategy", "RT Flt", "ML Flt", "LOG Flt", "Total");
    printf("--------------------------------------------------------------------------\n");
    printf("%-34s %-8d %-8d %-8d %-8d\n",
           "Mode A: Shared Global LRU (No Lock)",
           unres.rt_faults, unres.ml_faults, unres.log_faults, unres.total_faults);
    printf("%-34s %-8d %-8d %-8d %-8d\n",
           "Mode B: 4 Locked RT + 8 Shared LRU",
           res.rt_faults, res.ml_faults, res.log_faults, res.total_faults);
    printf("--------------------------------------------------------------------------\n\n");

    printf("Key Finding:\n");
    printf("  - In Mode A (unreserved demand paging), heavy ML tensor sweeps evict\n");
    printf("    the RT control task's pages, causing %d RT page faults (UNACCEPTABLE!).\n",
           unres.rt_faults);
    printf("  - In Mode B (4 reserved/locked RT frames), RT suffers %d runtime faults,\n",
           res.rt_faults);
    printf("    completely isolating real-time control from ML/LOG memory pressure.\n");
    return 0;
}
