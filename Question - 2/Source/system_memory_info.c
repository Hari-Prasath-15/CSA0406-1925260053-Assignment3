#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

/*
 * Program 3: System Page Size & Memory-Access Workload
 *
 * 1. Queries the Linux kernel page size via sysconf(_SC_PAGESIZE)
 *    (equivalent to the shell command: getconf PAGESIZE).
 * 2. Allocates a 64 MB buffer and writes one byte per page to trigger
 *    predictable minor page faults for inspection with /usr/bin/time -v.
 */

#define WORKLOAD_MB 64UL

int main(void) {
    long page_size = sysconf(_SC_PAGESIZE);
    if (page_size <= 0) {
        perror("sysconf(_SC_PAGESIZE) failed");
        return 1;
    }

    printf("============================================================\n");
    printf(" SYSTEM MEMORY INFO & PAGE-FAULT WORKLOAD\n");
    printf("============================================================\n");
    printf("Linux System Page Size (sysconf) : %ld bytes (%.2f KB)\n",
           page_size, (double)page_size / 1024.0);

    size_t total_bytes = WORKLOAD_MB * 1024UL * 1024UL;
    size_t num_pages = total_bytes / (size_t)page_size;

    unsigned char *buffer = (unsigned char *)malloc(total_bytes);
    if (buffer == NULL) {
        fprintf(stderr, "Error: Failed to allocate %lu MB.\n", WORKLOAD_MB);
        return 1;
    }

    /* Touch 1 byte in every allocated page to force demand-paged physical allocation */
    unsigned long checksum = 0;
    for (size_t i = 0; i < total_bytes; i += (size_t)page_size) {
        buffer[i] = (unsigned char)(i & 0xFF);
        checksum += buffer[i];
    }

    printf("Workload Buffer Allocated        : %lu MB (%zu pages touched)\n",
           WORKLOAD_MB, num_pages);
    printf("Verification Checksum            : %lu\n", checksum);
    printf("Run with '/usr/bin/time -v ./system_memory_info' to inspect minor/major page faults.\n");

    free(buffer);
    return 0;
}
