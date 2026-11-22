#include <stdio.h>

/*
 * Program: clock_algorithm.c
 * Implements the Clock (Second-Chance) page-replacement algorithm as a
 * practical hardware-friendly approximation of exact LRU.
 *
 * Mechanism:
 *   - Frames are arranged conceptually in a circular ring with a 'hand' pointer.
 *   - Each frame stores a single 1-bit reference flag (ref_bit: 0 or 1).
 *   - On a page hit: set ref_bit[idx] = 1 (O(1) hardware bit set).
 *   - On a page fault:
 *       * If free frame exists, load page and set ref_bit = 1.
 *       * Otherwise advance 'hand': if ref_bit[hand] == 1, give a second chance
 *         by clearing ref_bit[hand] = 0 and moving hand = (hand + 1) % frames.
 *         The first frame with ref_bit == 0 is replaced and its ref_bit set to 1.
 */

#define MAX_FRAMES 16

static int find_page(const int frames[], int used, int page) {
    for (int i = 0; i < used; i++) {
        if (frames[i] == page) {
            return i;
        }
    }
    return -1;
}

static int run_clock(const int ref[], int len, int capacity, int verbose) {
    if (capacity <= 0 || capacity > MAX_FRAMES) {
        return -1;
    }

    int frames[MAX_FRAMES];
    int ref_bit[MAX_FRAMES];
    int used = 0;
    int hand = 0;
    int faults = 0;

    for (int i = 0; i < capacity; i++) {
        frames[i] = -1;
        ref_bit[i] = 0;
    }

    if (verbose) {
        printf("\n--- Clock (Second-Chance) Trace (%d Frames) ---\n", capacity);
        printf("%-5s %-5s %-28s %-6s %-6s\n", "Step", "Page", "Frames (page:refbit)", "Hand", "Status");
    }

    for (int i = 0; i < len; i++) {
        int page = ref[i];
        int idx = find_page(frames, used, page);
        int hit = (idx >= 0);

        if (hit) {
            ref_bit[idx] = 1; /* Hardware sets reference bit to 1 */
        } else {
            faults++;
            if (used < capacity) {
                frames[used] = page;
                ref_bit[used] = 1;
                used++;
            } else {
                while (ref_bit[hand] == 1) {
                    ref_bit[hand] = 0; /* Clear bit -> give second chance */
                    hand = (hand + 1) % capacity;
                }
                frames[hand] = page;
                ref_bit[hand] = 1;
                hand = (hand + 1) % capacity;
            }
        }

        if (verbose) {
            printf("%-5d %-5d [", i + 1, page);
            for (int j = 0; j < capacity; j++) {
                if (j < used) {
                    printf("%d:%d", frames[j], ref_bit[j]);
                } else {
                    printf("-:-");
                }
                if (j < capacity - 1) {
                    printf(" ");
                }
            }
            printf("]          %-6d %s\n", hand, hit ? "HIT" : "FAULT");
        }
    }

    return faults;
}

static int run_lru_ref(const int ref[], int len, int capacity) {
    int frames[MAX_FRAMES], last[MAX_FRAMES];
    int used = 0, faults = 0;
    for (int i = 0; i < len; i++) {
        int k = find_page(frames, used, ref[i]);
        if (k >= 0) {
            last[k] = i;
            continue;
        }
        faults++;
        if (used < capacity) {
            frames[used] = ref[i];
            last[used] = i;
            used++;
        } else {
            int victim = 0;
            for (int j = 1; j < capacity; j++) {
                if (last[j] < last[victim]) {
                    victim = j;
                }
            }
            frames[victim] = ref[i];
            last[victim] = i;
        }
    }
    return faults;
}

int main(void) {
    const int ref[20] = {7, 0, 1, 2, 0, 3, 0, 4, 2, 3, 0, 3, 2, 1, 2, 0, 1, 7, 0, 1};
    const int belady[12] = {1, 2, 3, 4, 1, 2, 5, 1, 2, 3, 4, 5};

    printf("==========================================================================\n");
    printf("   CLOCK (SECOND-CHANCE) ALGORITHM vs. EXACT LRU\n");
    printf("==========================================================================\n");

    int c3 = run_clock(ref, 20, 3, 1);
    int c4 = run_clock(ref, 20, 4, 0);
    int l3 = run_lru_ref(ref, 20, 3);
    int l4 = run_lru_ref(ref, 20, 4);

    printf("\nComparison on Assignment Reference String (20 accesses):\n");
    printf("  3 Frames -> Clock: %d faults | Exact LRU: %d faults\n", c3, l3);
    printf("  4 Frames -> Clock: %d faults | Exact LRU: %d faults\n", c4, l4);

    int bc3 = run_clock(belady, 12, 3, 0);
    int bc4 = run_clock(belady, 12, 4, 0);
    printf("\nComparison on Belady String (12 accesses):\n");
    printf("  Clock -> 3 frames: %d faults | 4 frames: %d faults\n", bc3, bc4);
    return 0;
}
