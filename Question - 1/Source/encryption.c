#include "encryption.h"

/*
 * Version 1.0 of the Banking Encryption Library.
 * Implements a simple symmetric XOR cipher for demonstration purposes.
 * In Experiment 2, this shared library is replaced with Version 2.0
 * (library/encryption_v2.c) without recompiling the main program.
 */

const char *get_library_version(void) {
    return "v1.0 (Original Release)";
}

const char *get_algorithm_name(void) {
    return "Basic Single-Key XOR Cipher";
}

void encrypt_transaction(const char *input, char *output, size_t len, unsigned char key) {
    for (size_t i = 0; i < len; i++) {
        output[i] = (char)((unsigned char)input[i] ^ key);
    }
    output[len] = '\0';
}

void decrypt_transaction(const char *input, char *output, size_t len, unsigned char key) {
    /* XOR is its own inverse */
    for (size_t i = 0; i < len; i++) {
        output[i] = (char)((unsigned char)input[i] ^ key);
    }
    output[len] = '\0';
}
