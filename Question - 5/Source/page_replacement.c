#include <stdio.h>

/*
 * Program: page_replacement.c
 * Implements FIFO, LRU, and Optimal page-replacement algorithms with
 * step-by-step frame state tracing for hand-calculation verification and
 * Belady's Anomaly demonstration.
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

static void print_frame_state(const int frames[], int used, int capacity) {
    printf("[");
    for (int i = 0; i < capacity; i++) {
        if (i < used) {
            printf("%2d", frames[i]);
        } else {
            printf(" -");
        }
        if (i < capacity - 1) {
            printf(" ");
        }
    }
    printf("]");
}

static int run_fifo_trace(const int ref[], int len, int capacity, int verbose) {
    if (capacity <= 0 || capacity > MAX_FRAMES) {
        return -1;
    }
    int frames[MAX_FRAMES];
    int used = 0, next = 0, faults = 0;

    if (verbose) {
        printf("\n--- FIFO Trace (%d Frames) ---\n", capacity);
        printf("%-6s %-6s %-18s %-8s\n", "Step", "Page", "Frames After", "Status");
    }

    for (int i = 0; i < len; i++) {
        int hit = (find_page(frames, used, ref[i]) >= 0);
        if (!hit) {
            faults++;
            if (used < capacity) {
                frames[used++] = ref[i];
            } else {
                frames[next] = ref[i];
                next = (next + 1) % capacity;
            }
        }
        if (verbose) {
            printf("%-6d %-6d ", i + 1, ref[i]);
            print_frame_state(frames, used, capacity);
            printf("   %s\n", hit ? "HIT" : "FAULT");
        }
    }
    return faults;
}

static int run_lru_trace(const int ref[], int len, int capacity, int verbose) {
    if (capacity <= 0 || capacity > MAX_FRAMES) {
        return -1;
    }
    int frames[MAX_FRAMES];
    int last_used[MAX_FRAMES];
    int used = 0, faults = 0;

    if (verbose) {
        printf("\n--- LRU Trace (%d Frames) ---\n", capacity);
        printf("%-6s %-6s %-18s %-8s\n", "Step", "Page", "Frames After", "Status");
    }

    for (int i = 0; i < len; i++) {
        int idx = find_page(frames, used, ref[i]);
        int hit = (idx >= 0);
        if (hit) {
            last_used[idx] = i;
        } else {
            faults++;
            if (used < capacity) {
                frames[used] = ref[i];
                last_used[used] = i;
                used++;
            } else {
                int victim = 0;
                for (int j = 1; j < capacity; j++) {
                    if (last_used[j] < last_used[victim]) {
                        victim = j;
                    }
                }
                frames[victim] = ref[i];
                last_used[victim] = i;
            }
        }
        if (verbose) {
            printf("%-6d %-6d ", i + 1, ref[i]);
            print_frame_state(frames, used, capacity);
            printf("   %s\n", hit ? "HIT" : "FAULT");
        }
    }
    return faults;
}

static int run_optimal_trace(const int ref[], int len, int capacity, int verbose) {
    if (capacity <= 0 || capacity > MAX_FRAMES) {
        return -1;
    }
    int frames[MAX_FRAMES];
    int used = 0, faults = 0;

    if (verbose) {
        printf("\n--- Optimal Trace (%d Frames) ---\n", capacity);
        printf("%-6s %-6s %-18s %-8s\n", "Step", "Page", "Frames After", "Status");
    }

    for (int i = 0; i < len; i++) {
        int hit = (find_page(frames, used, ref[i]) >= 0);
        if (!hit) {
            faults++;
            if (used < capacity) {
                frames[used++] = ref[i];
            } else {
                int victim = 0, farthest = -1;
                for (int j = 0; j < capacity; j++) {
                    int next_use = len;
                    for (int t = i + 1; t < len; t++) {
                        if (ref[t] == frames[j]) {
                            next_use = t;
                            break;
                        }
                    }
                    if (next_use > farthest) {
                        farthest = next_use;
                        victim = j;
                    }
                }
                frames[victim] = ref[i];
            }
        }
        if (verbose) {
            printf("%-6d %-6d ", i + 1, ref[i]);
            print_frame_state(frames, used, capacity);
            printf("   %s\n", hit ? "HIT" : "FAULT");
        }
    }
    return faults;
}

int main(void) {
    const int ref[20] = {7, 0, 1, 2, 0, 3, 0, 4, 2, 3, 0, 3, 2, 1, 2, 0, 1, 7, 0, 1};
    const int belady[12] = {1, 2, 3, 4, 1, 2, 5, 1, 2, 3, 4, 5};

    printf("==========================================================================\n");
    printf("   PAGE-REPLACEMENT ALGORITHMS: FIFO, LRU, AND OPTIMAL\n");
    printf("==========================================================================\n");

    int f3 = run_fifo_trace(ref, 20, 3, 1);
    int l3 = run_lru_trace(ref, 20, 3, 1);
    int o3 = run_optimal_trace(ref, 20, 3, 1);

    int f4 = run_fifo_trace(ref, 20, 4, 0);
    int l4 = run_lru_trace(ref, 20, 4, 0);
    int o4 = run_optimal_trace(ref, 20, 4, 0);

    printf("\n=== Summary on Standard 20-Page Reference String ===\n");
    printf("  3 Frames -> FIFO: %2d faults | LRU: %2d faults | Optimal: %2d faults\n", f3, l3, o3);
    printf("  4 Frames -> FIFO: %2d faults | LRU: %2d faults | Optimal: %2d faults\n", f4, l4, o4);

    printf("\n==========================================================================\n");
    printf("   BELADY'S ANOMALY DEMONSTRATION (String: 1 2 3 4 1 2 5 1 2 3 4 5)\n");
    printf("==========================================================================\n");
    int bf3 = run_fifo_trace(belady, 12, 3, 1);
    int bf4 = run_fifo_trace(belady, 12, 4, 1);
    int bl3 = run_lru_trace(belady, 12, 3, 0);
    int bl4 = run_lru_trace(belady, 12, 4, 0);

    printf("\nBelady's Anomaly Summary:\n");
    printf("  FIFO -> 3 frames: %d faults | 4 frames: %d faults  <-- ANOMALY (+%d fault!)\n",
           bf3, bf4, bf4 - bf3);
    printf("  LRU  -> 3 frames: %d faults | 4 frames: %d faults  <-- NO ANOMALY (Stack Property)\n",
           bl3, bl4);
    return 0;
}
