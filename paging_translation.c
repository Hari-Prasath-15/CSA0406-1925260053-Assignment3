#include <stdio.h>

/* Default PAGE_SIZE is 1024 as given in the assignment.
 * Can be overridden at compile time with -DPAGE_SIZE=2048 to verify 2048-byte pages.
 */
#ifndef PAGE_SIZE
#define PAGE_SIZE 1024
#endif

#define PAGES 8

/* Page table of a process: frame number, or -1 if the page is not in memory. */
int page_table[PAGES] = {5, 2, -1, 7, 0, -1, 3, -1};

static void translate_address(unsigned a) {
    unsigned p = a / PAGE_SIZE;
    unsigned d = a % PAGE_SIZE;

    if (p >= PAGES) {
        printf("%u -> page %u offset %u: invalid address\n", a, p, d);
    } else if (page_table[p] == -1) {
        printf("%u -> page %u offset %u: page fault\n", a, p, d);
    } else {
        unsigned physical = (unsigned)page_table[p] * PAGE_SIZE + d;
        printf("%u -> page %u offset %u: physical %u\n", a, p, d, physical);
    }
}

int main(void) {
    unsigned logical[] = {0, 1500, 3100, 6200, 9000, 2049};

    printf("=== Part 1: Assignment Complete-the-Code (PAGE_SIZE = %d bytes) ===\n", PAGE_SIZE);
    for (int i = 0; i < 6; i++) {
        unsigned a = logical[i];
        /* TODO 1: Split a into page number p = a / PAGE_SIZE and offset d = a % PAGE_SIZE.
         * If p is outside the table print "invalid address"; if the frame is -1 print
         * "page fault"; otherwise print the physical address frame * PAGE_SIZE + d.
         */
        unsigned p = a / PAGE_SIZE;
        unsigned d = a % PAGE_SIZE;

        if (p >= PAGES) {
            printf("%u -> page %u offset %u: invalid address\n", a, p, d);
        } else if (page_table[p] == -1) {
            printf("%u -> page %u offset %u: page fault\n", a, p, d);
        } else {
            unsigned physical = (unsigned)page_table[p] * PAGE_SIZE + d;
            printf("%u -> page %u offset %u: physical %u\n", a, p, d, physical);
        }
    }

    /* Part 2: Boundary address tests (start of page, end of page, and beyond last page) */
    printf("\n=== Part 2: Boundary Address Tests (PAGE_SIZE = %d bytes) ===\n", PAGE_SIZE);
    unsigned boundary_tests[] = {
        0,                          /* Start of page 0 */
        PAGE_SIZE - 1,              /* End of page 0 */
        PAGE_SIZE,                  /* Start of page 1 */
        2 * PAGE_SIZE - 1,          /* End of page 1 */
        PAGES * PAGE_SIZE - 1,      /* Last valid byte (end of last page) */
        PAGES * PAGE_SIZE,          /* First byte beyond the last page */
        PAGES * PAGE_SIZE + 512     /* Well beyond the last page */
    };
    const char *labels[] = {
        "Start of page 0",
        "End of page 0",
        "Start of page 1",
        "End of page 1",
        "End of last page (page 7)",
        "First byte beyond last page",
        "Beyond last page (+512)"
    };
    int num_tests = (int)(sizeof(boundary_tests) / sizeof(boundary_tests[0]));

    for (int i = 0; i < num_tests; i++) {
        printf("[%s] ", labels[i]);
        translate_address(boundary_tests[i]);
    }

    return 0;
}
