#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include <unistd.h>
#include <sys/resource.h>

/*
 * Program 3: Sequential vs. Random Memory Access Experiment
 *
 * Allocates configurable memory buffers, touches every page sequentially and
 * in a reproducible random order (fixed seed = 42), and measures actual
 * elapsed time (clock_gettime) and actual OS minor/major page faults
 * (getrusage).
 *
 * Usage:
 *   ./memory_access_experiment              -> Tests 16 MB, 32 MB, and 64 MB
 *   ./memory_access_experiment 32 64 128    -> Tests custom sizes in MB (capped at 512 MB)
 */

#define RANDOM_SEED 42
#define MAX_SAFE_MB 512UL

static double diff_ms(struct timespec a, struct timespec b) {
    return (double)(b.tv_sec - a.tv_sec) * 1000.0 +
           (double)(b.tv_nsec - a.tv_nsec) / 1000000.0;
}

static void run_pattern(size_t mb, int is_random, long page_size) {
    size_t total_bytes = mb * 1024UL * 1024UL;
    size_t num_pages   = total_bytes / (size_t)page_size;

    /* Prepare page index order before allocating the target buffer */
    size_t *order = (size_t *)malloc(num_pages * sizeof(size_t));
    if (order == NULL) {
        fprintf(stderr, "Error: Could not allocate index array.\n");
        return;
    }
    for (size_t i = 0; i < num_pages; i++) {
        order[i] = i;
    }

    if (is_random) {
        /* Fisher-Yates shuffle with fixed seed for reproducible random page order */
        srand(RANDOM_SEED);
        for (size_t i = num_pages - 1; i > 0; i--) {
            size_t j = (size_t)rand() % (i + 1);
            size_t tmp = order[i];
            order[i] = order[j];
            order[j] = tmp;
        }
    }

    unsigned char *buf = (unsigned char *)malloc(total_bytes);
    if (buf == NULL) {
        fprintf(stderr, "Error: Could not allocate %zu MB buffer.\n", mb);
        free(order);
        return;
    }

    struct rusage ru_before, ru_after;
    struct timespec t_start, t_end;

    getrusage(RUSAGE_SELF, &ru_before);
    clock_gettime(CLOCK_MONOTONIC, &t_start);

    /* Touch 1 byte per page to measure page-faulting and TLB/cache locality */
    volatile unsigned long sink = 0;
    for (size_t i = 0; i < num_pages; i++) {
        size_t offset = order[i] * (size_t)page_size;
        buf[offset] = (unsigned char)(i & 0xFF);
        sink += buf[offset];
    }

    clock_gettime(CLOCK_MONOTONIC, &t_end);
    getrusage(RUSAGE_SELF, &ru_after);

    long minor_flt = ru_after.ru_minflt - ru_before.ru_minflt;
    long major_flt = ru_after.ru_majflt - ru_before.ru_majflt;
    double elapsed = diff_ms(t_start, t_end);

    printf("%-10zu %-12s %-14.2f %-14ld %-14ld\n",
           mb,
           is_random ? "Random" : "Sequential",
           elapsed,
           minor_flt,
           major_flt);

    (void)sink;
    free(buf);
    free(order);
}

int main(int argc, char *argv[]) {
    long page_size = sysconf(_SC_PAGESIZE);
    if (page_size <= 0) {
        page_size = 4096;
    }

    size_t default_sizes[3] = {16, 32, 64};
    size_t sizes[8];
    int count = 0;

    if (argc > 1) {
        for (int i = 1; i < argc && count < 8; i++) {
            unsigned long val = strtoul(argv[i], NULL, 10);
            if (val > 0 && val <= MAX_SAFE_MB) {
                sizes[count++] = (size_t)val;
            }
        }
    }
    if (count == 0) {
        for (int i = 0; i < 3; i++) {
            sizes[i] = default_sizes[i];
        }
        count = 3;
    }

    printf("==========================================================================\n");
    printf("   SEQUENTIAL vs. RANDOM MEMORY ACCESS (Page Size = %ld B, Seed = %d)\n",
           page_size, RANDOM_SEED);
    printf("==========================================================================\n");
    printf("%-10s %-12s %-14s %-14s %-14s\n",
           "Size (MB)", "Pattern", "Time (ms)", "Minor Faults", "Major Faults");
    printf("--------------------------------------------------------------------------\n");

    for (int i = 0; i < count; i++) {
        run_pattern(sizes[i], 0, page_size); /* Sequential */
        run_pattern(sizes[i], 1, page_size); /* Random */
    }

    printf("--------------------------------------------------------------------------\n");
    printf("Observation Note: Both patterns touch the same number of new pages, so\n");
    printf("minor page faults are similar, while random access typically takes longer\n");
    printf("due to CPU cache misses and TLB misses (poor spatial locality).\n");
    return 0;
}
