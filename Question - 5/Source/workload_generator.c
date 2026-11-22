#include <stdio.h>
#include <stdlib.h>

/*
 * Program: workload_generator.c
 * Generates two reproducible page-reference workloads (fixed seed = 42):
 *   1. Strong Locality Workload (simulates a web server: 85% of requests hit
 *      a hot working set of 5 pages that shifts every 2,000 references; 15%
 *      access cold pages across a 30-page space).
 *   2. Random Access Workload (uniformly random across 30 pages).
 *
 * Counts page faults for FIFO, LRU, Clock, and Optimal across 3 to 10 frames
 * and saves results to results/locality_results.csv and results/random_results.csv.
 */

#define WORKLOAD_LEN   10000
#define NUM_PAGES      30
#define MAX_FRAMES     16
#define RANDOM_SEED    42

static int find_page(const int frames[], int used, int page) {
    for (int i = 0; i < used; i++) {
        if (frames[i] == page) {
            return i;
        }
    }
    return -1;
}

static int sim_fifo(const int ref[], int len, int capacity) {
    int f[MAX_FRAMES], used = 0, next = 0, faults = 0;
    for (int i = 0; i < len; i++) {
        if (find_page(f, used, ref[i]) >= 0) continue;
        faults++;
        if (used < capacity) {
            f[used++] = ref[i];
        } else {
            f[next] = ref[i];
            next = (next + 1) % capacity;
        }
    }
    return faults;
}

static int sim_lru(const int ref[], int len, int capacity) {
    int f[MAX_FRAMES], last[MAX_FRAMES];
    int used = 0, faults = 0;
    for (int i = 0; i < len; i++) {
        int k = find_page(f, used, ref[i]);
        if (k >= 0) {
            last[k] = i;
            continue;
        }
        faults++;
        if (used < capacity) {
            f[used] = ref[i];
            last[used] = i;
            used++;
        } else {
            int victim = 0;
            for (int j = 1; j < capacity; j++) {
                if (last[j] < last[victim]) {
                    victim = j;
                }
            }
            f[victim] = ref[i];
            last[victim] = i;
        }
    }
    return faults;
}

static int sim_clock(const int ref[], int len, int capacity) {
    int f[MAX_FRAMES], rb[MAX_FRAMES];
    int used = 0, hand = 0, faults = 0;
    for (int i = 0; i < capacity; i++) rb[i] = 0;

    for (int i = 0; i < len; i++) {
        int k = find_page(f, used, ref[i]);
        if (k >= 0) {
            rb[k] = 1;
            continue;
        }
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

static int sim_optimal(const int ref[], int len, int capacity) {
    int f[MAX_FRAMES], used = 0, faults = 0;
    for (int i = 0; i < len; i++) {
        if (find_page(f, used, ref[i]) >= 0) continue;
        faults++;
        if (used < capacity) {
            f[used++] = ref[i];
            continue;
        }
        int victim = 0, farthest = -1;
        for (int j = 0; j < capacity; j++) {
            int next_use = len;
            for (int t = i + 1; t < len; t++) {
                if (ref[t] == f[j]) {
                    next_use = t;
                    break;
                }
            }
            if (next_use > farthest) {
                farthest = next_use;
                victim = j;
            }
        }
        f[victim] = ref[i];
    }
    return faults;
}

static void generate_locality_workload(int ref[], int len) {
    srand(RANDOM_SEED);
    const int hot_size = 5;
    for (int i = 0; i < len; i++) {
        /* Shift the hot working set every 2,000 references */
        int phase = (i / 2000) % 5;
        int hot_base = phase * 5;
        int r = rand() % 100;
        if (r < 85) {
            /* 85% probability: access hot working set */
            ref[i] = hot_base + (rand() % hot_size);
        } else {
            /* 15% probability: access any page in the 30-page space */
            ref[i] = rand() % NUM_PAGES;
        }
    }
}

static void generate_random_workload(int ref[], int len) {
    srand(RANDOM_SEED + 100);
    for (int i = 0; i < len; i++) {
        ref[i] = rand() % NUM_PAGES;
    }
}

static void evaluate_and_save(const int ref[], int len, const char *title, const char *csv_path) {
    FILE *fp = fopen(csv_path, "w");
    if (fp == NULL) {
        perror("Failed to open CSV file for writing");
        return;
    }
    fprintf(fp, "Frames,FIFO,LRU,Clock,Optimal\n");

    printf("\n==========================================================================\n");
    printf(" %s (%d References, %d Unique Pages)\n", title, len, NUM_PAGES);
    printf("==========================================================================\n");
    printf("%-8s %-10s %-10s %-10s %-10s\n", "Frames", "FIFO", "LRU", "Clock", "Optimal");
    printf("--------------------------------------------------------------------------\n");

    for (int fr = 3; fr <= 10; fr++) {
        int ff = sim_fifo(ref, len, fr);
        int fl = sim_lru(ref, len, fr);
        int fc = sim_clock(ref, len, fr);
        int fo = sim_optimal(ref, len, fr);
        printf("%-8d %-10d %-10d %-10d %-10d\n", fr, ff, fl, fc, fo);
        fprintf(fp, "%d,%d,%d,%d,%d\n", fr, ff, fl, fc, fo);
    }

    fclose(fp);
    printf("Saved results to: %s\n", csv_path);
}

int main(void) {
    int *locality_ref = (int *)malloc(WORKLOAD_LEN * sizeof(int));
    int *random_ref   = (int *)malloc(WORKLOAD_LEN * sizeof(int));
    if (locality_ref == NULL || random_ref == NULL) {
        fprintf(stderr, "Error: Memory allocation failed.\n");
        free(locality_ref);
        free(random_ref);
        return 1;
    }

    generate_locality_workload(locality_ref, WORKLOAD_LEN);
    generate_random_workload(random_ref, WORKLOAD_LEN);

    evaluate_and_save(locality_ref, WORKLOAD_LEN,
                      "STRONG-LOCALITY WORKLOAD RESULTS",
                      "results/locality_results.csv");
    evaluate_and_save(random_ref, WORKLOAD_LEN,
                      "RANDOM-ACCESS WORKLOAD RESULTS",
                      "results/random_results.csv");

    free(locality_ref);
    free(random_ref);
    return 0;
}
