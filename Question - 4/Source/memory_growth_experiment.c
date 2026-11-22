#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <sys/resource.h>

/*
 * Program 6: Controlled Memory-Growth Experiment
 *
 * Safely tests increasing memory sizes (default: 16, 32, 64, 128, 256 MB)
 * using sequential page touching, measuring execution time, actual minor/major
 * page faults, and Maximum Resident Set Size (ru_maxrss).
 *
 * SAFETY GUARD:
 *   - Maximum single allocation is capped at 512 MB by default so the program
 *     NEVER attempts to allocate 24 GB or freeze/crash your laptop.
 *   - To observe real swapping beyond free RAM safely, run this binary inside
 *     a memory-limited VM or Linux cgroup / systemd-run memory limit.
 */

#define HARD_SAFETY_CAP_MB 512UL

static double diff_ms(struct timespec a, struct timespec b) {
    return (double)(b.tv_sec - a.tv_sec) * 1000.0 +
           (double)(b.tv_nsec - a.tv_nsec) / 1000000.0;
}

static void test_memory_size(size_t mb, long page_size) {
    size_t total_bytes = mb * 1024UL * 1024UL;
    unsigned char *buf = (unsigned char *)malloc(total_bytes);
    if (buf == NULL) {
        printf("%-10zu ALLOCATION FAILED (Memory limit reached)\n", mb);
        return;
    }

    struct rusage ru_before, ru_after;
    struct timespec t_start, t_end;

    getrusage(RUSAGE_SELF, &ru_before);
    clock_gettime(CLOCK_MONOTONIC, &t_start);

    /* Sequentially write 1 byte per page to demand-allocate physical frames */
    volatile unsigned long checksum = 0;
    for (size_t i = 0; i < total_bytes; i += (size_t)page_size) {
        buf[i] = (unsigned char)((i / (size_t)page_size) & 0xFF);
        checksum += buf[i];
    }

    clock_gettime(CLOCK_MONOTONIC, &t_end);
    getrusage(RUSAGE_SELF, &ru_after);

    double elapsed_ms = diff_ms(t_start, t_end);
    long min_flt = ru_after.ru_minflt - ru_before.ru_minflt;
    long maj_flt = ru_after.ru_majflt - ru_before.ru_majflt;
    long max_rss_kb = ru_after.ru_maxrss; /* On Linux, ru_maxrss is in kilobytes */

    printf("%-10zu %-12.2f %-14ld %-14ld %-14ld\n",
           mb, elapsed_ms, min_flt, maj_flt, max_rss_kb);

    (void)checksum;
    free(buf);
}

int main(int argc, char *argv[]) {
    long page_size = sysconf(_SC_PAGESIZE);
    if (page_size <= 0) {
        page_size = 4096;
    }

    size_t default_sizes[5] = {16, 32, 64, 128, 256};
    size_t sizes[8];
    int count = 0;

    if (argc > 1) {
        for (int i = 1; i < argc && count < 8; i++) {
            unsigned long v = strtoul(argv[i], NULL, 10);
            if (v > 0 && v <= HARD_SAFETY_CAP_MB) {
                sizes[count++] = (size_t)v;
            } else if (v > HARD_SAFETY_CAP_MB) {
                fprintf(stderr, "[Safety Cap] Skipping %lu MB (exceeds %lu MB safe limit).\n",
                        v, HARD_SAFETY_CAP_MB);
            }
        }
    }
    if (count == 0) {
        for (int i = 0; i < 5; i++) {
            sizes[i] = default_sizes[i];
        }
        count = 5;
    }

    printf("==========================================================================\n");
    printf("   CONTROLLED MEMORY-GROWTH EXPERIMENT (Safe Cap = %lu MB)\n", HARD_SAFETY_CAP_MB);
    printf("==========================================================================\n");
    printf("Demand Paging Concept: Only touched pages are mapped to physical frames.\n");
    printf("This lets a 24 GB virtual program run on 16 GB RAM by keeping inactive\n");
    printf("pages on disk/swap and loading only the active working set into RAM.\n\n");

    printf("%-10s %-12s %-14s %-14s %-14s\n",
           "Size (MB)", "Time (ms)", "Minor Faults", "Major Faults", "Max RSS (KB)");
    printf("--------------------------------------------------------------------------\n");

    for (int i = 0; i < count; i++) {
        test_memory_size(sizes[i], page_size);
    }

    printf("--------------------------------------------------------------------------\n");
    printf("Note: To observe major faults (disk swapping) safely without freezing your\n");
    printf("      host laptop, run inside a memory-limited VM or cgroup (e.g., 128 MB).\n");
    return 0;
}
