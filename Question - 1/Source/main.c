#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include "encryption.h"
#include "maps_helper.h"

/*
 * Banking Branch Application (Static & Dynamic Linked Version)
 *
 * The exact same source code is compiled in two ways to guarantee a fair comparison:
 *   1. Statically linked against libencryption.a  -> bank_static
 *   2. Dynamically linked against libencryption.so -> bank_dynamic
 *
 * Optional flag:
 *   --hold [seconds] : Keeps the process alive so you can inspect pmap and
 *                      /proc/<PID>/smaps (used in Experiment 3).
 */

int main(int argc, char *argv[]) {
    int hold_seconds = 0;
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--hold") == 0) {
            hold_seconds = 20;
            if (i + 1 < argc) {
                int parsed = atoi(argv[i + 1]);
                if (parsed > 0) {
                    hold_seconds = parsed;
                }
            }
        }
    }

    const char *transaction = "ACC:102938|AMT:INR_50000|TO:998877";
    size_t len = strlen(transaction);
    unsigned char key = 0x5A;

    char *encrypted = (char *)malloc(len + 1);
    char *decrypted = (char *)malloc(len + 1);
    if (encrypted == NULL || decrypted == NULL) {
        fprintf(stderr, "Error: Memory allocation failed.\n");
        free(encrypted);
        free(decrypted);
        return 1;
    }

    printf("============================================================\n");
    printf("   BANK BRANCH APPLICATION (PID: %d)\n", getpid());
    printf("============================================================\n");
    printf("Encryption Library Version : %s\n", get_library_version());
    printf("Encryption Algorithm       : %s\n", get_algorithm_name());

    /* Perform encryption and decryption */
    encrypt_transaction(transaction, encrypted, len, key);
    decrypt_transaction(encrypted, decrypted, len, key);

    printf("Original Transaction       : %s\n", transaction);
    print_hex("Encrypted (Hex)           ", encrypted, len);
    printf("Decrypted Transaction      : %s\n", decrypted);
    printf("Integrity Verification     : %s\n",
           (strcmp(transaction, decrypted) == 0) ? "PASS" : "FAIL");

    /* Show runtime virtual addresses of functions and variables */
    printf("\n--- Virtual Addresses in Process Memory ---\n");
    printf("  Address of main()                : %p\n", (void *)(uintptr_t)main);
    printf("  Address of encrypt_transaction() : %p\n", (void *)(uintptr_t)encrypt_transaction);
    printf("  Address of heap buffer           : %p\n", (void *)encrypted);
    printf("  Address of stack variable (key)  : %p\n", (void *)&key);

    /* Display memory layout from /proc/self/maps */
    print_relevant_memory_maps("Process Memory Mappings");

    if (hold_seconds > 0) {
        printf("\n[Hold Mode] Process PID %d sleeping for %d seconds for pmap/smaps inspection...\n",
               getpid(), hold_seconds);
        fflush(stdout);
        sleep((unsigned int)hold_seconds);
    }

    free(encrypted);
    free(decrypted);
    return 0;
}
