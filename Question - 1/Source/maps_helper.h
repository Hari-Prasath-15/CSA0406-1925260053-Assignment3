#ifndef MAPS_HELPER_H
#define MAPS_HELPER_H

/*
 * Reads /proc/self/maps and prints the virtual memory mappings relevant
 * to the executable, shared libraries (libencryption, libc), heap, and stack.
 */
void print_relevant_memory_maps(const char *heading);

/*
 * Prints a byte buffer in readable hexadecimal format.
 */
void print_hex(const char *label, const char *data, size_t len);

#endif /* MAPS_HELPER_H */
