#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/resource.h>

/*
 * Program 1: memory_locking_test.c
 * Stage 3 - Experiment 1: Memory Locking and Real-Time Control Loop Latency
 *
 * Compares iteration latency of a simulated real-time control loop:
 *   Phase A: Unlocked memory (freshly allocated, not pre-faulted, not mlock-ed)
 *   Phase B: Locked memory (pre-faulted/initialized + mlock() before loop)
 *
 * IMPORTANT DISCLAIMERS:
 *   - Ordinary Linux + mlock() prevents locked pages from being swapped out
 *     and avoids page faults on pre-touched pages, but it does NOT provide
 *     hard real-time guarantees and does NOT eliminate OS scheduler or
 *     hardware interrupt latency.
 *   - Measured delays depend on host CPU load, kernel version, and WSL.
 */

#define DEFAULT_BUF_KB     64UL    /* 64 KB (16 x 4 KB pages): safe within default RLIMIT_MEMLOCK */
#define MAX_SAFE_BUF_KB    16384UL /* 16 MB upper safety cap */
#define DEFAULT_ITERATIONS 2000

static double diff_us(struct timespec a, struct timespec b) {
    return (double)(b.tv_sec - a.tv_sec) * 1000000.0 +
           (double)(b.tv_nsec - a.tv_nsec) / 1000.0;
}

typedef struct {
    double min_us;
    double avg_us;
    double max_us;
    long minor_faults;
    long major_faults;
} LoopMetrics;

static LoopMetrics run_control_loop(volatile unsigned char *buf, size_t size_bytes,
                                    long page_size, int iterations) {
    LoopMetrics m = {1e12, 0.0, 0.0, 0, 0};
    double total_us = 0.0;
    struct rusage ru_start, ru_end;

    getrusage(RUSAGE_SELF, &ru_start);

    for (int iter = 0; iter < iterations; iter++) {
        struct timespec t0, t1;
        clock_gettime(CLOCK_MONOTONIC, &t0);

        /* Simulated control-loop computation touching every page of state buffer */
        unsigned int acc = 0;
        for (size_t off = 0; off < size_bytes; off += (size_t)page_size) {
            buf[off] = (unsigned char)((buf[off] + 1) & 0xFF);
            acc += buf[off];
        }
        (void)acc;

        clock_gettime(CLOCK_MONOTONIC, &t1);
        double d = diff_us(t0, t1);
        if (d < m.min_us) m.min_us = d;
        if (d > m.max_us) m.max_us = d;
        total_us += d;
    }

    getrusage(RUSAGE_SELF, &ru_end);
    m.avg_us = total_us / (double)iterations;
    m.minor_faults = ru_end.ru_minflt - ru_start.ru_minflt;
    m.major_faults = ru_end.ru_majflt - ru_start.ru_majflt;
    return m;
}

int main(int argc, char *argv[]) {
    size_t buf_kb = DEFAULT_BUF_KB;
    int iterations = DEFAULT_ITERATIONS;

    if (argc >= 2) {
        unsigned long v = strtoul(argv[1], NULL, 10);
        if (v > 0 && v <= MAX_SAFE_BUF_KB) {
            buf_kb = (size_t)v;
        } else {
            fprintf(stderr, "Invalid buffer size (allowed: 1..%lu KB). Using %lu KB.\n",
                    MAX_SAFE_BUF_KB, DEFAULT_BUF_KB);
        }
    }
    if (argc >= 3) {
        int it = atoi(argv[2]);
        if (it >= 100 && it <= 100000) {
            iterations = it;
        }
    }

    long page_size = sysconf(_SC_PAGESIZE);
    if (page_size <= 0) {
        page_size = 4096;
    }
    size_t buf_bytes = buf_kb * 1024UL;

    struct rlimit rl;
    printf("==========================================================================\n");
    printf(" EXPERIMENT 1: MEMORY LOCKING (mlock) & REAL-TIME LOOP LATENCY\n");
    printf("==========================================================================\n");
    printf("Buffer Size        : %zu KB (%zu pages of %ld bytes)\n",
           buf_kb, buf_bytes / (size_t)page_size, page_size);
    printf("Loop Iterations    : %d\n", iterations);

    if (getrlimit(RLIMIT_MEMLOCK, &rl) == 0) {
        if (rl.rlim_cur == RLIM_INFINITY) {
            printf("RLIMIT_MEMLOCK     : unlimited\n");
        } else {
            printf("RLIMIT_MEMLOCK     : %lu bytes (%lu KB soft limit)\n",
                   (unsigned long)rl.rlim_cur, (unsigned long)(rl.rlim_cur / 1024UL));
        }
    }
    printf("\n");

    /* ---------------- PHASE A: UNLOCKED (Cold Allocation) ---------------- */
    unsigned char *buf_unlocked = NULL;
    if (posix_memalign((void **)&buf_unlocked, (size_t)page_size, buf_bytes) != 0 ||
        buf_unlocked == NULL) {
        fprintf(stderr, "Error: Failed to allocate unlocked buffer.\n");
        return 1;
    }

    /* Do NOT pre-fault or lock before starting the measured loop */
    LoopMetrics m_unlocked = run_control_loop(buf_unlocked, buf_bytes, page_size, iterations);
    free(buf_unlocked);

    /* ---------------- PHASE B: LOCKED & PRE-FAULTED ---------------- */
    unsigned char *buf_locked = NULL;
    if (posix_memalign((void **)&buf_locked, (size_t)page_size, buf_bytes) != 0 ||
        buf_locked == NULL) {
        fprintf(stderr, "Error: Failed to allocate locked buffer.\n");
        return 1;
    }

    /* Step 1: Lock pages into physical RAM using mlock() */
    int mlock_ok = 0;
    if (mlock(buf_locked, buf_bytes) == 0) {
        mlock_ok = 1;
        printf("[mlock() Status]   : SUCCESS (Locked %zu KB into RAM)\n", buf_kb);
    } else {
        int err = errno;
        printf("[mlock() Status]   : FAILED (errno=%d: %s)\n", err, strerror(err));
        printf("  -> Safe Fix Hint : Check 'ulimit -l'. Increase RLIMIT_MEMLOCK or run with\n");
        printf("                     smaller buffer (e.g., 32 KB) or CAP_IPC_LOCK / sudo.\n");
    }

    /* Step 2: Pre-fault (initialize every page) BEFORE starting the timed loop */
    memset(buf_locked, 0, buf_bytes);

    /* Step 3: Run identical timed loop */
    LoopMetrics m_locked = run_control_loop(buf_locked, buf_bytes, page_size, iterations);

    if (mlock_ok) {
        munlock(buf_locked, buf_bytes);
    }
    free(buf_locked);

    printf("\n%-24s %-11s %-11s %-12s %-10s %-10s\n",
           "Test Mode", "Min (us)", "Avg (us)", "Worst (us)", "MinFlt", "MajFlt");
    printf("--------------------------------------------------------------------------\n");
    printf("%-24s %-11.2f %-11.2f %-12.2f %-10ld %-10ld\n",
           "Unlocked (Cold Start)",
           m_unlocked.min_us, m_unlocked.avg_us, m_unlocked.max_us,
           m_unlocked.minor_faults, m_unlocked.major_faults);
    printf("%-24s %-11.2f %-11.2f %-12.2f %-10ld %-10ld\n",
           mlock_ok ? "Locked + Pre-faulted" : "Pre-faulted (mlock fail)",
           m_locked.min_us, m_locked.avg_us, m_locked.max_us,
           m_locked.minor_faults, m_locked.major_faults);
    printf("--------------------------------------------------------------------------\n");

    printf("\nInterpretation Notes:\n");
    printf("  1. Page-Fault Delay : Cold unlocked buffer incurs minor faults on first touch.\n");
    printf("                        Pre-touching + mlock() yields 0 faults inside the loop.\n");
    printf("  2. Scheduler Delay  : Spikes in 'Worst (us)' can still occur due to ordinary\n");
    printf("                        Linux CFS scheduling, interrupts, or WSL host activity.\n");
    printf("  3. Real-Time Scope  : mlock() eliminates swap/page-fault delays on locked data,\n");
    printf("                        but ordinary Linux does NOT guarantee hard real-time.\n");
    return 0;
}
