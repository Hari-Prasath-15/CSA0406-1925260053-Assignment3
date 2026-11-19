#include <stdio.h>
#include <string.h>
#include "maps_helper.h"

void print_relevant_memory_maps(const char *heading) {
    FILE *fp = fopen("/proc/self/maps", "r");
    if (fp == NULL) {
        perror("Failed to open /proc/self/maps");
        return;
    }

    printf("\n--- %s (/proc/self/maps) ---\n", heading);
    char line[512];
    int count = 0;

    while (fgets(line, sizeof(line), fp) != NULL) {
        /* Show mappings for the bank executable, our encryption library, libc, heap, and stack */
        if (strstr(line, "bank_") != NULL ||
            strstr(line, "libencryption") != NULL ||
            strstr(line, "libc.") != NULL ||
            strstr(line, "libc-") != NULL ||
            strstr(line, "[heap]") != NULL ||
            strstr(line, "[stack]") != NULL) {
            printf("  %s", line);
            count++;
        }
    }

    if (count == 0) {
        printf("  (No matching entries found in /proc/self/maps)\n");
    }
    fclose(fp);
}

void print_hex(const char *label, const char *data, size_t len) {
    printf("%s: ", label);
    for (size_t i = 0; i < len; i++) {
        printf("%02X ", (unsigned char)data[i]);
    }
    printf("\n");
}
