#include "../source/encryption.h"

/*
 * Version 2.0 of the Banking Encryption Library (Security Patch).
 * Demonstrates upgrading a shared library (libencryption.so) without
 * recompiling the dynamically linked main executable.
 *
 * Uses a position-dependent rotating XOR + modular shift cipher.
 */

const char *get_library_version(void) {
    return "v2.0 (Security Patched Release)";
}

const char *get_algorithm_name(void) {
    return "Patched Rotating-Key XOR + Byte-Shift Cipher";
}

void encrypt_transaction(const char *input, char *output, size_t len, unsigned char key) {
    for (size_t i = 0; i < len; i++) {
        unsigned char rotated_key = (unsigned char)((key + (i * 7)) & 0xFF);
        unsigned char shifted = (unsigned char)(((unsigned char)input[i] + 3) & 0xFF);
        output[i] = (char)(shifted ^ rotated_key);
    }
    output[len] = '\0';
}

void decrypt_transaction(const char *input, char *output, size_t len, unsigned char key) {
    for (size_t i = 0; i < len; i++) {
        unsigned char rotated_key = (unsigned char)((key + (i * 7)) & 0xFF);
        unsigned char unxored = (unsigned char)((unsigned char)input[i] ^ rotated_key);
        output[i] = (char)((unxored - 3) & 0xFF);
    }
    output[len] = '\0';
}
