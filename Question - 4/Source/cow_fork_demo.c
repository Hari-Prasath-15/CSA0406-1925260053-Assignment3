#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/resource.h>

/*
 * Program 4: Copy-on-Write (CoW) Demonstration with fork()
 *
 * 1. Parent allocates and initializes a 32 MB buffer (all pages become resident).
 * 2. Parent forks a child process. Immediately after fork(), parent and child
 *    share the same physical frames (marked read-only for Copy-on-Write).
 * 3. Child reads the buffer (no page copies triggered).
 * 4. Child modifies every page of the buffer -> triggers CoW minor page faults
 *    as the OS duplicates each written page into a new private physical frame.
 *
 * NOTE: Exact VmRSS and Private_Dirty readings can vary slightly across Linux
 * kernels and Ubuntu WSL memory-accounting configurations.
 */

#define BUFFER_MB 32UL

static long read_vm_rss_kb(void) {
    FILE *fp = fopen("/proc/self/status", "r");
    if (fp == NULL) {
        return -1;
    }
    char line[256];
    long rss = -1;
    while (fgets(line, sizeof(line), fp) != NULL) {
        if (strncmp(line, "VmRSS:", 6) == 0) {
            sscanf(line + 6, "%ld", &rss);
            break;
        }
    }
    fclose(fp);
    return rss;
}

static void print_stats(const char *label) {
    struct rusage ru;
    getrusage(RUSAGE_SELF, &ru);
    long rss = read_vm_rss_kb();
    printf("  %-44s | VmRSS: %7ld KB | MinFlt: %6ld | MajFlt: %ld\n",
           label, rss, ru.ru_minflt, ru.ru_majflt);
}

int main(void) {
    long page_size = sysconf(_SC_PAGESIZE);
    if (page_size <= 0) {
        page_size = 4096;
    }

    size_t total_bytes = BUFFER_MB * 1024UL * 1024UL;
    unsigned char *buffer = (unsigned char *)malloc(total_bytes);
    if (buffer == NULL) {
        fprintf(stderr, "Error: Failed to allocate %lu MB.\n", BUFFER_MB);
        return 1;
    }

    printf("==========================================================================\n");
    printf("   COPY-ON-WRITE (CoW) WITH fork() DEMONSTRATION (%lu MB Buffer)\n", BUFFER_MB);
    printf("==========================================================================\n");

    print_stats("[Parent] Before initializing 32 MB buffer");

    /* Initialize every page so the buffer is physically resident in RAM */
    memset(buffer, 0x55, total_bytes);
    print_stats("[Parent] After initializing 32 MB buffer");
    fflush(stdout);

    pid_t pid = fork();
    if (pid < 0) {
        perror("fork() failed");
        free(buffer);
        return 1;
    }

    if (pid == 0) {
        /* Child process */
        print_stats("[Child ] Immediately after fork() (Shared)");

        /* Read-only pass: should NOT trigger Copy-on-Write page faults */
        volatile unsigned long checksum = 0;
        for (size_t i = 0; i < total_bytes; i += (size_t)page_size) {
            checksum += buffer[i];
        }
        print_stats("[Child ] After READ-ONLY pass (Still Shared)");

        /* Write pass: modifying 1 byte per page triggers CoW copies */
        struct rusage before_w, after_w;
        getrusage(RUSAGE_SELF, &before_w);

        for (size_t i = 0; i < total_bytes; i += (size_t)page_size) {
            buffer[i] = 0xAA;
        }

        getrusage(RUSAGE_SELF, &after_w);
        print_stats("[Child ] After WRITING all pages (CoW Done)");

        long cow_faults = after_w.ru_minflt - before_w.ru_minflt;
        printf("\n  -> Child triggered %ld minor page faults during the write pass\n",
               cow_faults);
        printf("     (Expected ~%zu faults for %lu MB with %ld-byte pages).\n",
               total_bytes / (size_t)page_size, BUFFER_MB, page_size);

        (void)checksum;
        free(buffer);
        fflush(stdout);
        _exit(0);
    }

    /* Parent waits for child to finish */
    int status = 0;
    waitpid(pid, &status, 0);

    print_stats("[Parent] After child exited");
    printf("\n  -> Parent buffer[0] is still 0x%02X (unchanged by child's write).\n",
           buffer[0]);
    printf("  -> Note: Exact VmRSS and fault counts may vary slightly across Linux/WSL.\n");

    free(buffer);
    return 0;
}
