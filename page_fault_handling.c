#include <stdio.h>

/*
 * Program 1: Simulated Page-Fault Handling Sequence
 *
 * Demonstrates the six standard steps an operating system performs when a
 * process accesses a valid virtual page that is not currently resident in RAM.
 *
 * NOTE: This program is an EDUCATIONAL SIMULATION in user space.
 * It does NOT trigger an actual hardware MMU exception or kernel disk I/O.
 * Real page faults are handled transparently by the CPU hardware and Linux
 * kernel (measured using getrusage() in Programs 3, 4, and 6).
 */

#define NUM_PAGES  6
#define NUM_FRAMES 4
#define PAGE_SIZE  4096

typedef struct {
    int frame;      /* Physical frame number, or -1 if not in RAM */
    int valid;      /* 1 = resident in RAM, 0 = on disk (not resident) */
    int disk_block; /* Backing-store block number on SSD/HDD */
} PageTableEntry;

static void print_page_table(const PageTableEntry pt[], int n) {
    printf("  +------+-------+-------+------------+\n");
    printf("  | Page | Valid | Frame | Disk Block |\n");
    printf("  +------+-------+-------+------------+\n");
    for (int i = 0; i < n; i++) {
        printf("  |  %2d  |   %d   |  %3d  |    %4d    |\n",
               i, pt[i].valid, pt[i].frame, pt[i].disk_block);
    }
    printf("  +------+-------+-------+------------+\n");
}

static void access_virtual_address(PageTableEntry pt[], int *next_free_frame,
                                   unsigned int vaddr) {
    unsigned int page   = vaddr / PAGE_SIZE;
    unsigned int offset = vaddr % PAGE_SIZE;

    printf("\n--------------------------------------------------------------------------\n");
    printf("CPU Instruction References Virtual Address: %u (Page %u, Offset %u)\n",
           vaddr, page, offset);
    printf("--------------------------------------------------------------------------\n");

    /* Step 1: Check bounds and page table */
    printf("[Step 1] MMU checks Page Table Entry for Page %u...\n", page);
    if (page >= NUM_PAGES) {
        printf("  -> INVALID REFERENCE: Page %u is outside the process address space.\n", page);
        printf("  -> OS terminates process with SIGSEGV.\n");
        return;
    }

    if (pt[page].valid == 1) {
        unsigned int paddr = (unsigned int)pt[page].frame * PAGE_SIZE + offset;
        printf("  -> PAGE HIT: Valid bit = 1, Physical Frame = %d.\n", pt[page].frame);
        printf("  -> Translated Physical Address = %d * %d + %u = %u\n",
               pt[page].frame, PAGE_SIZE, offset, paddr);
        return;
    }

    /* Step 2: Page fault detected */
    printf("[Step 2] PAGE FAULT DETECTED: Valid bit = 0 (Page %u is not in RAM).\n", page);
    printf("         Hardware traps to OS kernel and saves CPU registers/state.\n");

    /* Step 3: OS verifies reference and locates page on backing store */
    printf("[Step 3] OS kernel checks internal memory table: reference is legal.\n");
    printf("         Page %u is located at Disk Block %d on backing store.\n",
           page, pt[page].disk_block);

    /* Step 4: Allocate a free physical frame */
    int allocated_frame = *next_free_frame;
    if (allocated_frame < NUM_FRAMES) {
        (*next_free_frame)++;
        printf("[Step 4] OS allocates Free Physical Frame %d from the free-frame list.\n",
               allocated_frame);
    } else {
        allocated_frame = 0; /* Simulated replacement if all frames are full */
        printf("[Step 4] No free frames; OS selects Victim Frame %d via replacement policy.\n",
               allocated_frame);
    }

    /* Step 5: Read page from disk into frame and update page table */
    printf("[Step 5] OS schedules disk I/O to read Disk Block %d into Frame %d.\n",
           pt[page].disk_block, allocated_frame);
    printf("         (Process blocks while I/O completes; CPU runs other processes.)\n");
    pt[page].frame = allocated_frame;
    pt[page].valid = 1;
    printf("         Disk I/O complete. OS updates Page Table: Page %u -> Frame %d (Valid = 1).\n",
           page, allocated_frame);

    /* Step 6: Restart interrupted instruction */
    unsigned int paddr = (unsigned int)pt[page].frame * PAGE_SIZE + offset;
    printf("[Step 6] OS restores CPU state and RESTARTS the interrupted instruction.\n");
    printf("         MMU re-checks Page %u -> PAGE HIT (Frame %d).\n", page, pt[page].frame);
    printf("         Final Physical Address = %d * %d + %u = %u\n",
           pt[page].frame, PAGE_SIZE, offset, paddr);
}

int main(void) {
    PageTableEntry page_table[NUM_PAGES] = {
        {0, 1, 100},  /* Page 0: resident in Frame 0 */
        {1, 1, 101},  /* Page 1: resident in Frame 1 */
        {-1, 0, 102}, /* Page 2: on disk (will cause simulated page fault) */
        {-1, 0, 103}, /* Page 3: on disk (will cause simulated page fault) */
        {-1, 0, 104}, /* Page 4: on disk */
        {-1, 0, 105}  /* Page 5: on disk */
    };
    int next_free_frame = 2; /* Frames 0 and 1 already occupied; Frames 2 and 3 free */

    printf("==========================================================================\n");
    printf("   SIMULATED PAGE-FAULT HANDLING DEMONSTRATION (USER-SPACE MODEL)\n");
    printf("==========================================================================\n");
    printf("Note: This demonstrates the logical OS steps in software. Real hardware\n");
    printf("      page faults are measured via getrusage() in Programs 3, 4, and 6.\n\n");

    printf("Initial Page Table State:\n");
    print_page_table(page_table, NUM_PAGES);

    /* 1. Page Hit (Page 1, offset 200) */
    access_virtual_address(page_table, &next_free_frame, 1 * PAGE_SIZE + 200);

    /* 2. Simulated Page Fault (Page 2, offset 512) */
    access_virtual_address(page_table, &next_free_frame, 2 * PAGE_SIZE + 512);

    /* 3. Subsequent Access to Page 2 (Now a Page Hit!) */
    access_virtual_address(page_table, &next_free_frame, 2 * PAGE_SIZE + 1024);

    /* 4. Invalid Virtual Address (Page 7 >= NUM_PAGES) */
    access_virtual_address(page_table, &next_free_frame, 7 * PAGE_SIZE + 16);

    printf("\nFinal Page Table State:\n");
    print_page_table(page_table, NUM_PAGES);
    return 0;
}
