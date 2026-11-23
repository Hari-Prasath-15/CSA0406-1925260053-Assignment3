#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/mman.h>

/*
 * Program 2: task_memory_simulator.c
 * Stage 3 - Experiment 2: Smart-Factory Edge Device Memory Budget Simulator
 *
 * Models the 2 GB (2048 MB) RAM, no-swap edge device logically without
 * actually allocating 2 GB on the student's laptop.
 *
 * Performs a safe, small (32 KB) physical allocation + mlock() to demonstrate
 * locking the real-time control task's state buffer while tracking the full
 * 2048 MB logical memory budget across:
 *   - Operating System & Kernel Reserve :  256 MB
 *   - Real-Time Control Task (Locked)   :   64 MB
 *   - Machine-Learning Task (Bounded)   : 1536 MB (1.5 GB)
 *   - Logging Task (Ring Buffer)        :  128 MB
 *   - Free Safety Headroom              :   64 MB
 *
 * Then evaluates adding a 4th Task requiring 500 MB and demonstrates
 * budget overflow detection and mitigation options.
 */

#define TOTAL_RAM_MB       2048
#define SWAP_DISK_MB       0
#define SAFE_RT_DEMO_BYTES (32UL * 1024UL) /* Safe 32 KB physical allocation */

typedef struct {
    const char *name;
    int budget_mb;
    const char *policy;
    const char *fault_tolerance;
} TaskBudget;

static void print_budget_table(const TaskBudget tasks[], int count, const char *scenario_title) {
    int total_used = 0;
    printf("\n==========================================================================\n");
    printf(" %s\n", scenario_title);
    printf("==========================================================================\n");
    printf("%-28s %-10s %-18s %-16s\n", "Component / Task", "RAM (MB)", "Memory Policy", "Page Faults?");
    printf("--------------------------------------------------------------------------\n");

    for (int i = 0; i < count; i++) {
        printf("%-28s %-10d %-18s %-16s\n",
               tasks[i].name, tasks[i].budget_mb, tasks[i].policy, tasks[i].fault_tolerance);
        total_used += tasks[i].budget_mb;
    }

    int remaining = TOTAL_RAM_MB - total_used;
    printf("--------------------------------------------------------------------------\n");
    printf("Total Physical RAM : %d MB (2.00 GB) | Swap Disk : %d MB\n",
           TOTAL_RAM_MB, SWAP_DISK_MB);
    printf("Total Requested    : %d MB (%.2f%% of physical RAM)\n",
           total_used, ((double)total_used / (double)TOTAL_RAM_MB) * 100.0);

    if (remaining >= 0) {
        printf("Remaining Headroom : %d MB  --> STATUS: FITS SAFELY WITHIN 2 GB RAM\n",
               remaining);
    } else {
        printf("Memory Deficit     : %d MB  --> STATUS: OVERFLOW! EXCEEDS 2 GB RAM!\n",
               -remaining);
        printf("  -> Impact (No Swap): Linux OOM Killer will terminate a process (likely\n");
        printf("     the high-RSS ML task) unless budgets are reduced or RAM is upgraded.\n");
    }
}

int main(void) {
    printf("==========================================================================\n");
    printf(" EXPERIMENT 2: EDGE DEVICE 2 GB MEMORY BUDGET & TASK SIMULATOR\n");
    printf("==========================================================================\n");
    printf("Safety Note: Logical 2048 MB budget simulation; only 32 KB is physically\n");
    printf("allocated and locked here to avoid stressing your laptop's RAM.\n");

    /* Safe small physical mlock() check for the Real-Time Control Task */
    void *rt_state = NULL;
    long page_size = sysconf(_SC_PAGESIZE);
    if (page_size <= 0) page_size = 4096;

    if (posix_memalign(&rt_state, (size_t)page_size, SAFE_RT_DEMO_BYTES) == 0 && rt_state != NULL) {
        memset(rt_state, 0x5A, SAFE_RT_DEMO_BYTES);
        if (mlock(rt_state, SAFE_RT_DEMO_BYTES) == 0) {
            printf("RT Control State Buffer : Allocated & mlock()-ed 32 KB safely (SUCCESS).\n");
            munlock(rt_state, SAFE_RT_DEMO_BYTES);
        } else {
            printf("RT Control State Buffer : Allocated 32 KB; mlock() returned: %s.\n",
                   strerror(errno));
        }
        free(rt_state);
    }

    /* Scenario 1: Baseline 3-Task Plan */
    TaskBudget baseline[4] = {
        {"Linux OS & Kernel Reserve", 256,  "Kernel Paging",    "No (Kernel Resident)"},
        {"Task 1: RT Control Loop",    64,  "Paged + mlock()",  "NEVER (Zero Faults)"},
        {"Task 2: Machine Learning", 1536,  "Paged + cgroup",   "Tolerable (Soft)"},
        {"Task 3: Logging Service",   128,  "Paged + RingBuf",  "Tolerable (Best-Effort)"}
    };
    print_budget_table(baseline, 4, "SCENARIO 1: BASELINE 3-TASK EDGE DEVICE PLAN");

    /* Scenario 2: Adding a 4th Task requiring 500 MB (Unmitigated) */
    TaskBudget with_fourth[5] = {
        {"Linux OS & Kernel Reserve", 256,  "Kernel Paging",    "No (Kernel Resident)"},
        {"Task 1: RT Control Loop",    64,  "Paged + mlock()",  "NEVER (Zero Faults)"},
        {"Task 2: Machine Learning", 1536,  "Paged + cgroup",   "Tolerable (Soft)"},
        {"Task 3: Logging Service",   128,  "Paged + RingBuf",  "Tolerable (Best-Effort)"},
        {"Task 4: New Vendor Task",   500,  "Paged + cgroup",   "Tolerable"}
    };
    print_budget_table(with_fourth, 5, "SCENARIO 2: ADDING 4TH TASK (500 MB) - UNMITIGATED");

    /* Scenario 3: Mitigated Plan (ML Quantized FP32 -> INT8: 1536 MB -> 900 MB) */
    TaskBudget mitigated[5] = {
        {"Linux OS & Kernel Reserve", 256,  "Kernel Paging",    "No (Kernel Resident)"},
        {"Task 1: RT Control Loop",    64,  "Paged + mlock()",  "NEVER (Zero Faults)"},
        {"Task 2: ML (INT8 Quant.)",  900,  "Paged + cgroup",   "Tolerable (Soft)"},
        {"Task 3: Logging Service",   128,  "Paged + RingBuf",  "Tolerable (Best-Effort)"},
        {"Task 4: New Vendor Task",   500,  "Paged + cgroup",   "Tolerable"}
    };
    print_budget_table(mitigated, 5, "SCENARIO 3: MITIGATED PLAN (INT8 ML QUANTIZATION -> 900 MB)");

    return 0;
}
