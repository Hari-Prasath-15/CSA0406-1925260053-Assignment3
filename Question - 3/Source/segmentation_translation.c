#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
 * Program 1: Segment-Table Address Translation
 *
 * Translates (segment, offset) logical addresses into physical addresses using
 * a segment table storing Base, Limit, Name, and Access Permissions.
 *
 * Formula:
 *   If 0 <= segment < NUM_SEGMENTS and 0 <= offset < limit:
 *       physical_address = base + offset
 *   Else:
 *       Segmentation Fault / Invalid Address Trap
 *
 * Usage:
 *   ./segmentation_translation               -> Runs the 8 built-in test cases
 *   ./segmentation_translation --interactive -> Also allows entering custom (seg, offset)
 */

#define NUM_SEGMENTS 5

typedef struct {
    int seg_id;
    const char *name;
    unsigned int base;
    unsigned int limit;
    const char *permissions;
} SegmentEntry;

static const SegmentEntry segment_table[NUM_SEGMENTS] = {
    {0, "Code (Text)",    1400, 1000, "R-X (Read/Execute, Shared)"},
    {1, "Global Data",    6300,  400, "RW- (Read/Write, Private)"},
    {2, "Heap",           4300,  500, "RW- (Read/Write, Private)"},
    {3, "Stack",          3200, 1100, "RW- (Read/Write, Private)"},
    {4, "Shared Library", 4700,  800, "R-X (Read/Execute, Shared)"}
};

static void print_segment_table(void) {
    printf("==========================================================================\n");
    printf("                           SEGMENT TABLE                                  \n");
    printf("==========================================================================\n");
    printf("%-6s %-16s %-10s %-10s %-28s\n",
           "Seg#", "Segment Name", "Base", "Limit", "Permissions");
    printf("--------------------------------------------------------------------------\n");
    for (int i = 0; i < NUM_SEGMENTS; i++) {
        printf("%-6d %-16s %-10u %-10u %-28s\n",
               segment_table[i].seg_id,
               segment_table[i].name,
               segment_table[i].base,
               segment_table[i].limit,
               segment_table[i].permissions);
    }
    printf("==========================================================================\n\n");
}

static void translate_segment_address(int seg, int offset) {
    printf("Input: (Segment = %d, Offset = %d)\n", seg, offset);

    if (seg < 0 || seg >= NUM_SEGMENTS) {
        printf("  -> Status : INVALID SEGMENT (Segment %d does not exist; valid range: 0..%d)\n",
               seg, NUM_SEGMENTS - 1);
        printf("  -> Action : Trap to OS -> Segmentation Fault (SIGSEGV)\n\n");
        return;
    }

    SegmentEntry entry = segment_table[seg];
    printf("  -> Lookup : Name = %s | Base = %u | Limit = %u\n",
           entry.name, entry.base, entry.limit);

    if (offset < 0 || (unsigned int)offset >= entry.limit) {
        printf("  -> Check  : Offset %d is outside valid range [0, %u]\n",
               offset, entry.limit - 1);
        printf("  -> Status : SEGMENTATION FAULT (Offset exceeds segment limit)\n\n");
        return;
    }

    unsigned int physical_addr = entry.base + (unsigned int)offset;
    printf("  -> Check  : 0 <= %d < %u (Valid)\n", offset, entry.limit);
    printf("  -> Result : Physical Address = %u + %d = %u\n\n",
           entry.base, offset, physical_addr);
}

int main(int argc, char *argv[]) {
    print_segment_table();

    /* 8 Test Cases (matching hand calculations in Q3_Report.docx) */
    const int test_cases[8][2] = {
        {0,  430},  /* 1. Valid: Code segment */
        {1,  120},  /* 2. Valid: Global Data segment */
        {2,  499},  /* 3. Valid: Boundary (limit - 1) of Heap segment */
        {3,  500},  /* 4. Valid: Stack segment */
        {4,  750},  /* 5. Valid: Shared Library segment */
        {1,  400},  /* 6. Invalid: Offset == Limit (off-by-one trap) */
        {2,  650},  /* 7. Invalid: Offset > Limit (Heap overflow) */
        {5,   50}   /* 8. Invalid: Segment number out of range */
    };

    printf("=== Translating 8 Logical (Segment, Offset) Addresses ===\n\n");
    for (int i = 0; i < 8; i++) {
        printf("[Test %d] ", i + 1);
        translate_segment_address(test_cases[i][0], test_cases[i][1]);
    }

    if (argc > 1 && strcmp(argv[1], "--interactive") == 0) {
        int seg, off;
        printf("=== Interactive Mode (enter -1 -1 to quit) ===\n");
        while (1) {
            printf("Enter segment number and offset: ");
            if (scanf("%d %d", &seg, &off) != 2) {
                break;
            }
            if (seg == -1 && off == -1) {
                break;
            }
            translate_segment_address(seg, off);
        }
    }

    return 0;
}
