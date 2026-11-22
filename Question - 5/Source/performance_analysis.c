#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/*
 * Program: performance_analysis.c
 * Benchmarks the running time of FIFO, LRU, Clock, and Optimal page-replacement
 * algorithms on a reproducible 100,000-reference strong-locality web-server
 * workload (8 frames, 30 unique pages, seed = 42).
 *
 * Writes results to results/timing_results.csv.
 */

#define BENCH_LEN    100000
#define NUM_PAGES    30
#define BENCH_FRAMES 8
#define MAX_FRAMES   16
#define RANDOM_SEED  42

static double elapsed_ms(struct timespec a, struct timespec b) {
    return (double)(b.tv_sec - a.tv_sec) * 1000.0 +
           (double)(b.tv_nsec - a.tv_nsec) / 1000000.0;
}

static int find_page(const int f[], int used, int page) {
    for (int i = 0; i < used; i++) {
        if (f[i] == page) return i;
    }
    return -1;
}

static int bench_fifo(const int ref[], int len, int capacity) {
    int f[MAX_FRAMES], used = 0, next = 0, faults = 0;
    for (int i = 0; i < len; i++) {
        if (find_page(f, used, ref[i]) >= 0) continue;
        faults++;
        if (used < capacity) f[used++] = ref[i];
        else { f[next] = ref[i]; next = (next + 1) % capacity; }
    }
    return faults;
}

static int bench_lru(const int ref[], int len, int capacity) {
    int f[MAX_FRAMES], last[MAX_FRAMES];
    int used = 0, faults = 0;
    for (int i = 0; i < len; i++) {
        int k = find_page(f, used, ref[i]);
        if (k >= 0) { last[k] = i; continue; }
        faults++;
        if (used < capacity) {
            f[used] = ref[i];
            last[used] = i;
            used++;
        } else {
            int victim = 0;
            for (int j = 1; j < capacity; j++) {
                if (last[j] < last[victim]) victim = j;
            }
            f[victim] = ref[i];
            last[victim] = i;
        }
    }
    return faults;
}

static int bench_clock(const int ref[], int len, int capacity) {
    int f[MAX_FRAMES], rb[MAX_FRAMES];
    int used = 0, hand = 0, faults = 0;
    for (int i = 0; i < capacity; i++) rb[i] = 0;
    for (int i = 0; i < len; i++) {
        int k = find_page(f, used, ref[i]);
        if (k >= 0) { rb[k] = 1; continue; }
        faults++;
        if (used < capacity) {
            f[used] = ref[i];
            rb[used] = 1;
            used++;
        } else {
            while (rb[hand] == 1) {
                rb[hand] = 0;
                hand = (hand + 1) % capacity;
            }
            f[hand] = ref[i];
            rb[hand] = 1;
            hand = (hand + 1) % capacity;
        }
    }
    return faults;
}

static int bench_optimal(const int ref[], int len, int capacity) {
    int f[MAX_FRAMES], used = 0, faults = 0;
    for (int i = 0; i < len; i++) {
        if (find_page(f, used, ref[i]) >= 0) continue;
        faults++;
        if (used < capacity) { f[used++] = ref[i]; continue; }
        int victim = 0, farthest = -1;
        for (int j = 0; j < capacity; j++) {
            int next_use = len;
            for (int t = i + 1; t < len; t++) {
                if (ref[t] == f[j]) { next_use = t; break; }
            }
            if (next_use > farthest) { farthest = next_use; victim = j; }
        }
        f[victim] = ref[i];
    }
    return faults;
}

int main(void) {
    int *ref = (int *)malloc(BENCH_LEN * sizeof(int));
    if (ref == NULL) {
        fprintf(stderr, "Error: Memory allocation failed.\n");
        return 1;
    }

    /* Generate reproducible 100,000-reference strong-locality workload */
    srand(RANDOM_SEED);
    for (int i = 0; i < BENCH_LEN; i++) {
        int phase = (i / 10000) % 5;
        int hot_base = phase * 5;
        if ((rand() % 100) < 85) {
            ref[i] = hot_base + (rand() % 5);
        } else {
            ref[i] = rand() % NUM_PAGES;
        }
    }

    const char *names[4] = {"FIFO", "LRU", "Clock", "Optimal"};
    int (*funcs[4])(const int *, int, int) = {
        bench_fifo, bench_lru, bench_clock, bench_optimal
    };

    FILE *fp = fopen("results/timing_results.csv", "w");
    if (fp == NULL) {
        perror("Failed to open results/timing_results.csv");
        free(ref);
        return 1;
    }
    fprintf(fp, "Algorithm,References,Frames,Page_Faults,Runtime_ms,Avg_ns_per_Ref\n");

    printf("==========================================================================\n");
    printf("   RUNTIME BENCHMARK (%d References, %d Frames, Seed = %d)\n",
           BENCH_LEN, BENCH_FRAMES, RANDOM_SEED);
    printf("==========================================================================\n");
    printf("%-12s %-12s %-14s %-16s\n",
           "Algorithm", "Page Faults", "Runtime (ms)", "Avg Time (ns/ref)");
    printf("--------------------------------------------------------------------------\n");

    for (int a = 0; a < 4; a++) {
        struct timespec t0, t1;
        clock_gettime(CLOCK_MONOTONIC, &t0);
        int faults = funcs[a](ref, BENCH_LEN, BENCH_FRAMES);
        clock_gettime(CLOCK_MONOTONIC, &t1);

        double ms = elapsed_ms(t0, t1);
        double ns_per_ref = (ms * 1000000.0) / (double)BENCH_LEN;

        printf("%-12s %-12d %-14.3f %-16.2f\n", names[a], faults, ms, ns_per_ref);
        fprintf(fp, "%s,%d,%d,%d,%.3f,%.2f\n",
                names[a], BENCH_LEN, BENCH_FRAMES, faults, ms, ns_per_ref);
    }

    fclose(fp);
    printf("--------------------------------------------------------------------------\n");
    printf("Saved timing benchmark to: results/timing_results.csv\n");

    free(ref);
    return 0;
}
