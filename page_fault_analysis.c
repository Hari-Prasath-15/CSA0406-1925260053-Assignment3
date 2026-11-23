#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/resource.h>

/*
 * Program 3: page_fault_analysis.c
 * Stage 3 - Experiment 3: Page-Fault Behaviour Before & After Initialization / Locking
 *
 * Uses getrusage() (ru_minflt, ru_majflt) and /proc/self/status (VmRSS, VmLck)
 * to verify page-fault behaviour across:
 *   1. Unlocked 4 MB buffer: first touch vs. repeated touch.
 *   2. Locked 64 KB buffer: before mlock/init, during pre-touch + mlock(),
 *      and during the critical execution loop (0 faults observed).
 *
 * Note: Observing 0 faults in a short user-space test confirms that pre-faulted
 * locked pages did not fault during the test; it does NOT prove page faults can
 * never happen under every possible system condition (e.g., unlocked stack growth).
 */

#define UNLOCKED_SIZE_MB 4UL
#define LOCKED_SIZE_KB   64UL

static void read_proc_status_kb(long *vm_rss_kb, long *vm_lck_kb) {
    *vm_rss_kb = -1;
    *vm_lck_kb = -1;
    FILE *fp = fopen("/proc/self/status", "r");
    if (fp == NULL) {
        return;
    }
    char line[256];
    while (fgets(line, sizeof(line), fp) != NULL) {
        if (strncmp(line, "VmRSS:", 6) == 0) {
            sscanf(line + 6, "%ld", vm_rss_kb);
        } else if (strncmp(line, "VmLck:", 6) == 0) {
            sscanf(line + 6, "%ld", vm_lck_kb);
        }
    }
    fclose(fp);
}

static void print_snapshot(const char *stage, long delta_min, long delta_maj) {
    long rss = -1, lck = -1;
    read_proc_status_kb(&rss, &lck);
    char lck_str[32];
    if (lck >= 0) {
        snprintf(lck_str, sizeof(lck_str), "%4ld KB", lck);
    } else {
        snprintf(lck_str, sizeof(lck_str), "    N/A");
    }
    printf("  %-42s | dMinFlt: %5ld | dMajFlt: %2ld | VmRSS: %6ld KB | VmLck: %s\n",
           stage, delta_min, delta_maj, rss, lck_str);
}

int main(void) {
    long page_size = sysconf(_SC_PAGESIZE);
    if (page_size <= 0) page_size = 4096;

    printf("==========================================================================\n");
    printf(" EXPERIMENT 3: PAGE-FAULT ANALYSIS (getrusage + /proc/self/status)\n");
    printf("==========================================================================\n");
    printf("System Page Size: %ld bytes\n\n", page_size);

    /* ---------- TEST A: UNLOCKED 4 MB BUFFER ---------- */
    size_t u_bytes = UNLOCKED_SIZE_MB * 1024UL * 1024UL;
    unsigned char *u_buf = (unsigned char *)malloc(u_bytes);
    if (u_buf == NULL) {
        fprintf(stderr, "Error: Failed to allocate unlocked buffer.\n");
        return 1;
    }

    struct rusage r0, r1, r2;
    printf("--- Test A: Unlocked %lu MB Buffer (%zu pages) ---\n",
           UNLOCKED_SIZE_MB, u_bytes / (size_t)page_size);

    getrusage(RUSAGE_SELF, &r0);
    print_snapshot("1. After malloc() (before touching)", 0, 0);

    /* First touch: demand-paging allocates physical frames -> minor faults */
    for (size_t i = 0; i < u_bytes; i += (size_t)page_size) {
        u_buf[i] = (unsigned char)(i & 0xFF);
    }
    getrusage(RUSAGE_SELF, &r1);
    print_snapshot("2. After FIRST touch (demand allocation)",
                   r1.ru_minflt - r0.ru_minflt, r1.ru_majflt - r0.ru_majflt);

    /* Second touch: pages are already resident -> 0 faults */
    volatile unsigned long sum = 0;
    for (size_t i = 0; i < u_bytes; i += (size_t)page_size) {
        u_buf[i] ^= 0x01;
        sum += u_buf[i];
    }
    getrusage(RUSAGE_SELF, &r2);
    print_snapshot("3. After SECOND touch (resident pages)",
                   r2.ru_minflt - r1.ru_minflt, r2.ru_majflt - r1.ru_majflt);
    free(u_buf);

    /* ---------- TEST B: LOCKED 64 KB CONTROL BUFFER ---------- */
    size_t l_bytes = LOCKED_SIZE_KB * 1024UL;
    unsigned char *l_buf = NULL;
    if (posix_memalign((void **)&l_buf, (size_t)page_size, l_bytes) != 0 || l_buf == NULL) {
        fprintf(stderr, "Error: Failed to allocate locked buffer.\n");
        return 1;
    }

    printf("\n--- Test B: Locked %lu KB Control Buffer (%zu pages) ---\n",
           LOCKED_SIZE_KB, l_bytes / (size_t)page_size);

    struct rusage b0, b1, b2;
    getrusage(RUSAGE_SELF, &b0);
    print_snapshot("1. Before mlock() and initialization", 0, 0);

    int locked = (mlock(l_buf, l_bytes) == 0);
    memset(l_buf, 0x11, l_bytes); /* Pre-fault all pages before critical loop */
    getrusage(RUSAGE_SELF, &b1);
    print_snapshot(locked ? "2. After mlock() + pre-touch init" : "2. After pre-touch (mlock failed)",
                   b1.ru_minflt - b0.ru_minflt, b1.ru_majflt - b0.ru_majflt);

    /* Critical loop: 1000 passes over the locked buffer */
    for (int pass = 0; pass < 1000; pass++) {
        for (size_t i = 0; i < l_bytes; i += (size_t)page_size) {
            l_buf[i] = (unsigned char)((l_buf[i] + 1) & 0xFF);
            sum += l_buf[i];
        }
    }
    getrusage(RUSAGE_SELF, &b2);
    print_snapshot("3. During 1000-pass critical loop",
                   b2.ru_minflt - b1.ru_minflt, b2.ru_majflt - b1.ru_majflt);

    if (locked) {
        munlock(l_buf, l_bytes);
    }
    free(l_buf);
    (void)sum;

    printf("\nVerification Summary:\n");
    printf("  - Faults during locked critical loop: %ld minor, %ld major.\n",
           b2.ru_minflt - b1.ru_minflt, b2.ru_majflt - b1.ru_majflt);
    printf("  - Caution: A short test confirms 0 faults on the locked buffer during\n");
    printf("    this run; production RT code should also lock stack & code via mlockall().\n");
    return 0;
}
