#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>

/*
 * Program 4: Inspecting Linux Process Memory Segments via /proc/<pid>/maps
 *
 * Allocates variables in Code, Initialized Data, BSS (Uninitialized Data),
 * Heap, and Stack, prints their virtual addresses, and displays the process's
 * actual Linux memory regions from /proc/<pid>/maps.
 *
 * Optional flag:
 *   --hold [seconds] : Keeps the process alive for external inspection
 *                      (e.g., running `cat /proc/<pid>/maps` in another terminal).
 */

static int global_init_var = 42;        /* Initialized Data segment (.data) */
static int global_uninit_var;           /* Uninitialized Data segment (.bss) */

static void sample_code_function(void) {
    /* Lives in the read-only executable Code segment (.text) */
}

static void print_proc_maps(pid_t pid) {
    char path[64];
    snprintf(path, sizeof(path), "/proc/%d/maps", (int)pid);

    FILE *fp = fopen(path, "r");
    if (fp == NULL) {
        perror("Failed to open /proc/<pid>/maps");
        return;
    }

    printf("\n--- Key Memory Regions from %s ---\n", path);
    char line[512];
    while (fgets(line, sizeof(line), fp) != NULL) {
        if (strstr(line, "memory_maps") != NULL ||
            strstr(line, "[heap]") != NULL ||
            strstr(line, "[stack]") != NULL ||
            strstr(line, "libc.") != NULL ||
            strstr(line, "libc-") != NULL ||
            strstr(line, "[vdso]") != NULL) {
            printf("  %s", line);
        }
    }
    fclose(fp);
}

int main(int argc, char *argv[]) {
    int hold_seconds = 1; /* Brief pause by default; override with --hold <sec> */
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--hold") == 0 && i + 1 < argc) {
            int parsed = atoi(argv[i + 1]);
            if (parsed >= 0) {
                hold_seconds = parsed;
            }
        }
    }

    pid_t pid = getpid();
    int stack_var = 100; /* Stack segment */

    size_t heap_size = 1024 * 1024; /* 1 MB heap allocation */
    char *heap_buf = (char *)malloc(heap_size);
    if (heap_buf == NULL) {
        fprintf(stderr, "Error: Heap allocation failed.\n");
        return 1;
    }
    memset(heap_buf, 'A', 64); /* Touch heap memory so OS maps [heap] */

    printf("==========================================================================\n");
    printf("   LINUX PROCESS MEMORY SEGMENTS INSPECTION (PID: %d)\n", (int)pid);
    printf("==========================================================================\n");
    printf("Virtual Addresses of Program Regions:\n");
    printf("  Code (.text)  - sample_code_function() : %p  (r-xp: Read/Execute)\n",
           (void *)(uintptr_t)sample_code_function);
    printf("  Data (.data)  - global_init_var        : %p  (rw-p: Read/Write)\n",
           (void *)&global_init_var);
    printf("  BSS  (.bss)   - global_uninit_var      : %p  (rw-p: Read/Write)\n",
           (void *)&global_uninit_var);
    printf("  Heap ([heap]) - malloc(1 MB) buffer    : %p  (rw-p: Grows upward)\n",
           (void *)heap_buf);
    printf("  Stack([stack])- stack_var              : %p  (rw-p: Grows downward)\n",
           (void *)&stack_var);

    print_proc_maps(pid);

    printf("\nRegion Permissions Legend:\n");
    printf("  r-xp : Read + Execute + Private (Code / Shared Libraries)\n");
    printf("  r--p : Read-Only Data (Constants / ELF headers)\n");
    printf("  rw-p : Read + Write + Private (Global Data, Heap, Stack)\n");

    if (hold_seconds > 0) {
        printf("\nKeeping PID %d alive for %d second(s) for inspection...\n",
               (int)pid, hold_seconds);
        fflush(stdout);
        sleep((unsigned int)hold_seconds);
    }

    free(heap_buf);
    printf("Heap memory freed cleanly. Exiting.\n");
    return 0;
}
