#include <stdio.h>

/*
 * Program 5: Working-Set Model, Thrashing & Frame Allocation Simulator
 *
 * Part A:
 *   - Simulates LRU paging on a realistic 24-reference string with locality phases
 *     (working-set window delta = 4).
 *   - Measures page faults as allocated frames vary from 1 to 8.
 *   - Clearly identifies the thrashing zone where frames < working-set size.
 *
 * Part B:
 *   - Demonstrates Equal Frame Allocation vs. Proportional Frame Allocation
 *     across 3 processes sharing 100 physical frames.
 */

#define REF_LEN    24
#define MAX_FRAMES 8
#define DELTA      4

static int simulate_lru(const int ref[], int len, int frames) {
    int mem[MAX_FRAMES];
    int last_used[MAX_FRAMES];
    int used = 0;
    int faults = 0;

    for (int i = 0; i < len; i++) {
        int page = ref[i];
        int hit = -1;
        for (int j = 0; j < used; j++) {
            if (mem[j] == page) {
                hit = j;
                break;
            }
        }
        if (hit >= 0) {
            last_used[hit] = i;
            continue;
        }

        faults++;
        if (used < frames) {
            mem[used] = page;
            last_used[used] = i;
            used++;
        } else {
            int victim = 0;
            for (int j = 1; j < frames; j++) {
                if (last_used[j] < last_used[victim]) {
                    victim = j;
                }
            }
            mem[victim] = page;
            last_used[victim] = i;
        }
    }
    return faults;
}

static void show_working_set_sizes(const int ref[], int len, int delta) {
    int max_wss = 0;
    printf("--- Working-Set Size (WSS) with Window Delta = %d ---\n", delta);
    for (int t = 0; t < len; t++) {
        int start = (t - delta + 1 > 0) ? (t - delta + 1) : 0;
        int distinct = 0;
        for (int i = start; i <= t; i++) {
            int seen = 0;
            for (int j = start; j < i; j++) {
                if (ref[j] == ref[i]) {
                    seen = 1;
                    break;
                }
            }
            if (!seen) {
                distinct++;
            }
        }
        if (distinct > max_wss) {
            max_wss = distinct;
        }
    }
    printf("  Peak Working-Set Size (WSS_max) = %d distinct pages in any %d-step window.\n",
           max_wss, delta);
    printf("  When allocated frames < %d, thrashing (high fault rate) occurs.\n\n", max_wss);
}

static void demonstrate_frame_allocation(void) {
    const int total_frames = 100;
    const char *names[3] = {"P1 (Small Utility)", "P2 (Medium Editor)", "P3 (Medical Imaging)"};
    const int sizes_mb[3] = {10, 30, 160}; /* Total = 200 MB */
    int total_size = 0;
    for (int i = 0; i < 3; i++) {
        total_size += sizes_mb[i];
    }

    printf("==========================================================================\n");
    printf(" PART B: EQUAL vs. PROPORTIONAL FRAME ALLOCATION (Total Frames = %d)\n",
           total_frames);
    printf("==========================================================================\n");
    printf("%-22s %-12s %-16s %-20s\n",
           "Process", "Size (MB)", "Equal Frames", "Proportional Frames");
    printf("--------------------------------------------------------------------------\n");

    int equal_share = total_frames / 3;
    for (int i = 0; i < 3; i++) {
        int prop_share = (sizes_mb[i] * total_frames) / total_size;
        printf("%-22s %-12d %-16d %-20d\n",
               names[i], sizes_mb[i], equal_share, prop_share);
    }
    printf("--------------------------------------------------------------------------\n");
    printf("Note: Equal allocation gives P1 excess frames while starving P3.\n");
    printf("      Proportional allocation assigns frames according to process size.\n");
}

int main(void) {
    /* Reference string with two locality phases (pages {1,2,3,4} then {4,5,6,7}) */
    const int ref[REF_LEN] = {
        1, 2, 3, 4, 1, 2, 3, 4, 1, 2, 3, 4,
        4, 5, 6, 7, 4, 5, 6, 7, 5, 6, 7, 4
    };

    printf("==========================================================================\n");
    printf(" PART A: WORKING-SET MODEL & PAGE FAULTS vs. NUMBER OF FRAMES (LRU)\n");
    printf("==========================================================================\n");
    printf("Reference String (%d accesses): ", REF_LEN);
    for (int i = 0; i < REF_LEN; i++) {
        printf("%d ", ref[i]);
    }
    printf("\n\n");

    show_working_set_sizes(ref, REF_LEN, DELTA);

    printf("%-10s %-14s %-16s %-26s\n",
           "Frames", "Page Faults", "Fault Rate (%)", "System Status");
    printf("--------------------------------------------------------------------------\n");
    for (int f = 1; f <= MAX_FRAMES; f++) {
        int faults = simulate_lru(ref, REF_LEN, f);
        double rate = ((double)faults / (double)REF_LEN) * 100.0;
        const char *status;
        if (f < DELTA) {
            status = "THRASHING (Frames < WSS)";
        } else if (f == DELTA) {
            status = "STABLE (Knee of Curve)";
        } else {
            status = "OPTIMAL / MIN FAULTS";
        }
        printf("%-10d %-14d %-15.2f%% %-26s\n", f, faults, rate, status);
    }
    printf("\n");

    demonstrate_frame_allocation();
    return 0;
}
