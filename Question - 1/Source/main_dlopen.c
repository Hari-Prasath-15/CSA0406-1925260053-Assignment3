#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <dlfcn.h>
#include "maps_helper.h"

/*
 * Banking Branch Application (Runtime Loading Version using dlopen/dlsym)
 *
 * Demonstrates loading libencryption.so ONLY when an encrypted transaction
 * is actually requested, instead of linking it at startup.
 *
 * Usage:
 *   ./bank_dlopen              -> Runs an encrypted transfer (loads libencryption.so)
 *   ./bank_dlopen --no-encrypt -> Runs a local balance inquiry (never loads libencryption.so)
 *   ./bank_dlopen --hold [sec] -> Keeps process alive while library is loaded
 */

/* Function pointer typedefs matching encryption.h */
typedef const char *(*version_fn_t)(void);
typedef const char *(*algo_fn_t)(void);
typedef void (*crypto_fn_t)(const char *, char *, size_t, unsigned char);

int main(int argc, char *argv[]) {
    int need_encryption = 1;
    int hold_seconds = 0;
    const char *lib_path = "./libencryption.so";

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--no-encrypt") == 0) {
            need_encryption = 0;
        } else if (strcmp(argv[i], "--hold") == 0) {
            hold_seconds = 20;
            if (i + 1 < argc) {
                int parsed = atoi(argv[i + 1]);
                if (parsed > 0) {
                    hold_seconds = parsed;
                }
            }
        }
    }

    printf("============================================================\n");
    printf("   BANK BRANCH APPLICATION - RUNTIME dlopen() (PID: %d)\n", getpid());
    printf("============================================================\n");
    printf("Address of main() : %p\n", (void *)(uintptr_t)main);

    if (!need_encryption) {
        printf("Operation         : Local Balance Inquiry (Encryption NOT required)\n");
        printf("Account Balance   : INR 2,45,000.00\n");
        printf("Library Status    : libencryption.so was NEVER loaded into memory.\n");
        print_relevant_memory_maps("Memory Mappings (No Encryption Needed)");
        return 0;
    }

    /* Show memory mappings BEFORE dlopen() */
    print_relevant_memory_maps("BEFORE dlopen()");

    /* Step 1: Load the shared library at runtime */
    printf("\n[Runtime Loader] Loading %s via dlopen()...\n", lib_path);
    void *handle = dlopen(lib_path, RTLD_LAZY);
    if (handle == NULL) {
        /* Graceful error handling if the library is missing or corrupted */
        fprintf(stderr, "[ERROR] Could not load encryption library: %s\n", dlerror());
        fprintf(stderr, "[FALLBACK] Encrypted fund transfers are temporarily disabled.\n");
        fprintf(stderr, "[FALLBACK] Non-encrypted branch services remain operational.\n");
        return 2;
    }

    /* Clear any existing error before calling dlsym() */
    dlerror();

    /* Step 2: Resolve function symbols dynamically using dlsym() */
    version_fn_t get_version = (version_fn_t)(uintptr_t)dlsym(handle, "get_library_version");
    algo_fn_t get_algo       = (algo_fn_t)(uintptr_t)dlsym(handle, "get_algorithm_name");
    crypto_fn_t encrypt_fn   = (crypto_fn_t)(uintptr_t)dlsym(handle, "encrypt_transaction");
    crypto_fn_t decrypt_fn   = (crypto_fn_t)(uintptr_t)dlsym(handle, "decrypt_transaction");

    const char *dlsym_err = dlerror();
    if (dlsym_err != NULL || !get_version || !get_algo || !encrypt_fn || !decrypt_fn) {
        fprintf(stderr, "[ERROR] Failed to resolve library symbols: %s\n",
                dlsym_err ? dlsym_err : "Unknown symbol error");
        dlclose(handle);
        return 3;
    }

    /* Step 3: Execute the banking transaction using resolved function pointers */
    const char *transaction = "ACC:102938|AMT:INR_50000|TO:998877";
    size_t len = strlen(transaction);
    unsigned char key = 0x5A;

    char *encrypted = (char *)malloc(len + 1);
    char *decrypted = (char *)malloc(len + 1);
    if (encrypted == NULL || decrypted == NULL) {
        fprintf(stderr, "[ERROR] Memory allocation failed.\n");
        free(encrypted);
        free(decrypted);
        dlclose(handle);
        return 1;
    }

    printf("\nEncryption Library Version : %s\n", get_version());
    printf("Encryption Algorithm       : %s\n", get_algo());

    encrypt_fn(transaction, encrypted, len, key);
    decrypt_fn(encrypted, decrypted, len, key);

    printf("Original Transaction       : %s\n", transaction);
    print_hex("Encrypted (Hex)           ", encrypted, len);
    printf("Decrypted Transaction      : %s\n", decrypted);
    printf("Integrity Verification     : %s\n",
           (strcmp(transaction, decrypted) == 0) ? "PASS" : "FAIL");

    printf("\n--- Virtual Addresses (After dlopen) ---\n");
    printf("  Address of main()                : %p\n", (void *)(uintptr_t)main);
    printf("  Address of encrypt_transaction() : %p\n", (void *)(uintptr_t)encrypt_fn);
    printf("  Address of heap buffer           : %p\n", (void *)encrypted);

    /* Show memory mappings AFTER dlopen() */
    print_relevant_memory_maps("AFTER dlopen()");

    if (hold_seconds > 0) {
        printf("\n[Hold Mode] Process PID %d sleeping for %d seconds...\n",
               getpid(), hold_seconds);
        fflush(stdout);
        sleep((unsigned int)hold_seconds);
    }

    /* Step 4: Unload the shared library when no longer needed */
    free(encrypted);
    free(decrypted);
    dlclose(handle);
    printf("\n[Runtime Loader] Unloaded %s via dlclose().\n", lib_path);

    /* Show memory mappings AFTER dlclose() */
    print_relevant_memory_maps("AFTER dlclose()");

    return 0;
}
