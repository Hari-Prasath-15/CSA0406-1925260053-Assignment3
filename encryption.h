#ifndef ENCRYPTION_H
#define ENCRYPTION_H

#include <stddef.h>

/*
 * Encryption Library Interface for the Banking Application.
 * Used by static linking (libencryption.a), dynamic linking (libencryption.so),
 * and runtime loading (dlopen / dlsym).
 */

/* Returns the version string of the encryption library. */
const char *get_library_version(void);

/* Returns the name of the encryption algorithm currently implemented. */
const char *get_algorithm_name(void);

/*
 * Encrypts 'input' into 'output' using 'key'.
 * 'output' must have space for at least len + 1 bytes (null-terminated).
 */
void encrypt_transaction(const char *input, char *output, size_t len, unsigned char key);

/*
 * Decrypts 'input' back into 'output' using the same 'key'.
 * 'output' must have space for at least len + 1 bytes (null-terminated).
 */
void decrypt_transaction(const char *input, char *output, size_t len, unsigned char key);

#endif /* ENCRYPTION_H */
