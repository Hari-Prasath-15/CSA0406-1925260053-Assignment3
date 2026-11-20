#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

/*
 * Program 5: Safe Segmentation Fault Demonstration using fork()
 *
 * Isolates an illegal memory access (dereferencing a NULL pointer) inside a
 * child process so the parent process and experiment script continue cleanly.
 * The parent inspects the child's termination status using waitpid().
 */

int main(void) {
    printf("==========================================================================\n");
    printf("   SEGMENTATION FAULT DEMONSTRATION (ISOLATED VIA fork())\n");
    printf("==========================================================================\n");
    printf("[Parent PID %d] Forking a child process to perform an invalid memory write...\n",
           (int)getpid());
    fflush(stdout);

    pid_t pid = fork();
    if (pid < 0) {
        perror("fork() failed");
        return 1;
    }

    if (pid == 0) {
        /* Child process: deliberately dereference a NULL pointer (address 0x0) */
        printf("[Child  PID %d] Attempting to write to NULL pointer (address 0x0)...\n",
               (int)getpid());
        fflush(stdout);

        volatile int *invalid_ptr = NULL;
        *invalid_ptr = 999; /* Triggers hardware fault -> OS sends SIGSEGV (11) */

        /* This line is never reached */
        printf("[Child  PID %d] Unexpectedly survived invalid write!\n", (int)getpid());
        _exit(0);
    }

    /* Parent process: wait for child and report how the OS terminated it */
    int status = 0;
    if (waitpid(pid, &status, 0) < 0) {
        perror("waitpid() failed");
        return 1;
    }

    printf("\n[Parent PID %d] Child process (PID %d) has terminated.\n",
           (int)getpid(), (int)pid);

    if (WIFSIGNALED(status)) {
        int sig = WTERMSIG(status);
        printf("  -> Termination Cause : Killed by signal %d (%s)\n", sig, strsignal(sig));
        printf("  -> What the OS did   :\n");
        printf("       1. CPU/MMU tried to translate address 0x0 and found no valid mapping.\n");
        printf("       2. Hardware raised a page-fault / protection exception to the kernel.\n");
        printf("       3. Linux kernel verified the access was outside all valid VMAs.\n");
        printf("       4. Kernel delivered SIGSEGV (signal 11) and terminated the child.\n");
        printf("       5. Parent process remained completely protected and unaffected.\n");
    } else if (WIFEXITED(status)) {
        printf("  -> Child exited normally with code %d (no signal received).\n",
               WEXITSTATUS(status));
    }

    return 0;
}
