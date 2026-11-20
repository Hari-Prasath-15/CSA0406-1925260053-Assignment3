#include <stdio.h>

/*
 * Program 2: Effective Access Time (EAT) & Max Acceptable Page-Fault Rate
 *
 * Assignment Given Values:
 *   - Memory access time (ma)          = 100 ns
 *   - Page-fault service time (pf_time)= 8 ms = 8,000,000 ns
 *   - Page-fault rates (p)             = 1/1,000 ; 1/100,000 ; 1/1,000,000
 *   - Slowdown limit                   = 10% (max EAT = 110 ns)
 */

int main(void) {
    const double ma_ns = 100.0;                 /* 100 nanoseconds */
    const double pf_ms = 8.0;                   /* 8 milliseconds */
    const double pf_ns = pf_ms * 1000000.0;     /* 1 ms = 1,000,000 ns -> 8,000,000 ns */

    const double rates[3] = {
        1.0 / 1000.0,
        1.0 / 100000.0,
        1.0 / 1000000.0
    };
    const char *rate_labels[3] = {
        "1 in 1,000 (0.001)",
        "1 in 100,000 (0.00001)",
        "1 in 1,000,000 (0.000001)"
    };

    printf("==========================================================================\n");
    printf("   EFFECTIVE ACCESS TIME (EAT) & 10%% SLOWDOWN THRESHOLD ANALYSIS\n");
    printf("==========================================================================\n");
    printf("Given Parameters:\n");
    printf("  - Normal memory access time (ma)     = %.0f ns\n", ma_ns);
    printf("  - Page-fault service time (pf_time)  = %.0f ms\n", pf_ms);
    printf("  - Unit conversion                    = %.0f ms * 1,000,000 ns/ms = %.0f ns\n",
           pf_ms, pf_ns);
    printf("  - Formula: EAT = (1 - p) * ma + p * pf_time\n\n");

    for (int i = 0; i < 3; i++) {
        double p = rates[i];
        double eat = (1.0 - p) * ma_ns + p * pf_ns;
        double slowdown_factor = eat / ma_ns;
        double pct_increase = ((eat - ma_ns) / ma_ns) * 100.0;

        printf("Case %d: Page-Fault Rate p = %s\n", i + 1, rate_labels[i]);
        printf("  Substitution : EAT = (1 - %.6f) * %.0f + %.6f * %.0f\n",
               p, ma_ns, p, pf_ns);
        printf("  Result       : EAT = %.2f ns (%.4f microseconds)\n", eat, eat / 1000.0);
        printf("  Comparison   : %.2fx slower than normal 100 ns access (+%.2f%% slowdown)\n\n",
               slowdown_factor, pct_increase);
    }

    /* Maximum acceptable page-fault rate for at most 10% slowdown */
    double max_eat = ma_ns * 1.10; /* 110 ns */
    double p_max = (max_eat - ma_ns) / (pf_ns - ma_ns);
    double inv_p_max = 1.0 / p_max;

    printf("--------------------------------------------------------------------------\n");
    printf("Maximum Acceptable Page-Fault Rate for <= 10%% Slowdown:\n");
    printf("--------------------------------------------------------------------------\n");
    printf("  Requirement  : EAT <= 1.10 * 100 ns = %.2f ns\n", max_eat);
    printf("  Equation     : (1 - p) * 100 + p * 8,000,000 <= 110\n");
    printf("                 100 + 7,999,900 * p <= 110\n");
    printf("                 p <= 10 / 7,999,900\n");
    printf("  Result       : p_max = %.9f (approx. 1.25 x 10^-6)\n", p_max);
    printf("  Equivalent   : At most 1 page fault per %.0f memory accesses!\n", inv_p_max);
    return 0;
}
